/******************************************************************************
* File Name          : cmd_c.c
* Date First Issued  : 12/03/2016
* Board              : PC
* Description        : Details for handling the 'c' command (list msgs for the id entered)
*******************************************************************************/
/*
This is text for git updating....
*/

#include <sys/time.h>
#include <unistd.h>

#include "cmd_c.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

/* From parameter database: CMD_CODES_INSERT.sql */
#define CMD_LAUNCH_PARM_HDSHK 38  // Send msg to handshake transferring launch parameters
#define CMD_SEND_LAUNCH_PARM  39  // Send msg to send burst of parameters

#define LAUNCH_PARAM_BURST_SIZE	16	// Maximum number of CAN msgs in a burst when sending launch parameters');
#define LAUNCH_PARAM_RETRY_CT	3	// Number of error retries when sending launch parameters');
#define LAUNCH_PARAM_RETRY_TIMEOUT 500	// Number of milliseconds to wait for a response when sending launch parameters');
#define VER_MCL	1	// Version number

#define NUMBER_PARAMETERS 24	// Number of parameters in list

#define TIMEOUT_USEC 100000	// Timeout increment (microseconds) 0.1 secs
//#define TIMEOUT_USEC 500000	// Timeout increment (microseconds) 0.5 secs
//#define TIMEOUT_USEC 2000000	// Timeout increment (microseconds) 2 secs LONG

static void requestburst(struct CANRCVBUF* p);

static u32 lp_burst_size;	// Working burst size
static u32 lp_retry_ctr;	// Retry counter

static u32 state;	// Receiving state

static struct PCTOGATEWAY pctogateway; 
static u32 keybrd_id_c;
static u32 canseqnumber;
static struct CANRCVBUF can_last;	// Last CAN msg sent

static u32 idx_burst;	// Index within burst
static u32 idx_list;	// Index within list

static u32 burst_bits;	// Bits for msg within burst OK

static float flist[NUMBER_PARAMETERS];

static struct timeval t1;	// Next time of a timeout
static int flag_to;	// Timeout flag: 0 = inactive; not zero = active

extern int fdp;	/* port file descriptor */

/******************************************************************************
 * static struct timeval setnexttimeout(int sec, int usec);
 * @brief 	: Add an increment to the current time
 * @param	: sec = increment secs
 * @param	: usec = increment usec (0 to 2147483647)
 * @return	: new time
*******************************************************************************/
static struct timeval setnexttimeout(int sec, int usec)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);	// Get current time
	tv.tv_usec += usec;		// Add in microseconds
	while (tv.tv_usec >= 1000000)	// Check turnover to secs
	{ // Here, turnover of microseconds
		tv.tv_usec -= 1000000;
		tv.tv_sec += 1;	// Advance seconds
	}
	tv.tv_sec += sec;
	return tv;
}
/******************************************************************************
 * static int timeout(void);
 * @brief 	: Check if current time exceeds the timeout set previously
 * @return	: 0 = no timeout; not zero = timed out
*******************************************************************************/
static int checktimeout(void)
{
	int temp = 0;
	struct timeval tnow;
	if (flag_to != 0) // Timing active?
	{ // Here yes.
		temp = 1;
		gettimeofday(&tnow, NULL);
		if (timercmp(&tnow,&t1, >))
		{ // Here we have a timeout
			lp_retry_ctr += 1;
			printf("Timeout: retry ctr: %d: retry_size %d\t",lp_retry_ctr, LAUNCH_PARAM_RETRY_CT);
			if (lp_retry_ctr >= LAUNCH_PARAM_RETRY_CT)
			{ // Too many retries.  Give up, miserable fool.
				printf("END OF RETRIES.  Re-execute command\n");
				flag_to = 0;	// Stop timeout checks
				state = 2;	// Go to idle state if response msgs come in
				return temp;
			}
			// Try this burst again.  Success may be yours!
			requestburst(&can_last); // Send last msg and set new timeout

			switch (state) // Add msg to show where we are at
			{
			case 0: // Handshake response
				printf("Handshake request\n");
				break;
				
			case 1: // Handle request/receive bursts of msgs
				printf("Burst @ list index: %d\tburst bits %08X\n",(idx_list + idx_burst),burst_bits);
				break;		
			default:
				printf("Timeout: should not come here!\n");
				break;
			}
		}
	}
	return temp;
}
/******************************************************************************
 * static void printerror(char *pc, struct CANRCVBUF* p);
 * @brief 	: print header string and CAN msg
 * @param	: pc = pointer to string 
 * @param	: p = pointer to header string
*******************************************************************************/
static void printerror(char *pc,struct CANRCVBUF* p)
{
	int i;
	printf("%s ",pc);	// Heading string
	printf("%08X ",p->id);	// CAN id
	printf(":%d: ",p->dlc);	// Payload length
	for (i = 0; i < p->dlc; i++) // List payload in hex
		printf("%02X ",p->cd.uc[i]);
	printf("\n");	// A mess without this dear ones
	return;
}
/******************************************************************************
 * static void sendmsg_c (struct CANRCVBUF* p);
 * @brief 	: Send CAN msg; save msg; set a timeout; set timer flag
 * @param	: p = pointer to CAN msg buffer
*******************************************************************************/
static void sendmsg_c (struct CANRCVBUF* p)
{
	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, p);  // Send to file descriptor (e.g. serial port)
	can_last = *p;		// Save in case of timeout retry.
	t1 = setnexttimeout(0, TIMEOUT_USEC); // Set next timeout time
	flag_to = 1;		// Set timer flag ON
	return;
}
/******************************************************************************
 * static void requestburst (struct CANRCVBUF* p);
 * @brief 	: 
 * @param	: p = pointer to CAN msg buffer
*******************************************************************************/
static void requestburst(struct CANRCVBUF* p)
{
	p->dlc = 2;	// Size of payload for burst request
	p->cd.uc[0] = CMD_SEND_LAUNCH_PARM; // Code for burst request
	p->cd.uc[1] = idx_list; // Index for first parameter in burst
	sendmsg_c(p);

	idx_burst = 0;	// Reset index within the burst

	/* Clear burst msg received index array by setting bits ON
	   for the burst size, and the higher order bits OFF.
	   Turn off bits as burst msg comes in to show msg was
	   received and passed checks. */
	burst_bits = ~0;	// All bits ON
	burst_bits = burst_bits << (32 - lp_burst_size);
	burst_bits = burst_bits >> (32 - lp_burst_size);

	t1 = setnexttimeout(0, TIMEOUT_USEC); // Set next timeout time
	return;
}
/******************************************************************************
 * int cmd_c_init(char* p);
 * @brief 	: Check keyboard entry; send handshake CAN msg
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_c_init(char* p)
{
	struct CANRCVBUF can;
	struct CANRCVBUF* pcan = &can;

	if (strlen(p) < 6)
	{ // Here too few chars
		printf("Too few chars for the 'c' command (needs 8), example\nc 30e00000 [for c command and unit id]\n");
		return -1;
	}
	
	sscanf( (p+1), "%x",&keybrd_id_c);
	printf ("ID: %x\n", keybrd_id_c);

	state = 0;	// Wait for response to handshake request
	lp_retry_ctr = 0; // Retry counter reset

	/* Setup CAN msg for handshake request. */
	pcan->id       = keybrd_id_c;	// CAN id
	pcan->dlc      = 4;		// Size of payload
	pcan->cd.uc[0] = CMD_LAUNCH_PARM_HDSHK; // Handshake code
	pcan->cd.uc[1] = LAUNCH_PARAM_BURST_SIZE;	
	pcan->cd.uc[2] = NUMBER_PARAMETERS;
	pcan->cd.uc[3] = VER_MCL;	// Version number

	/* Send request for launch parameters.
		Send to port number
		Set next timer timeout time
		Save can msg in case timeout requires a retry
		Set timer flag ON (active) */
	sendmsg_c(pcan); // Do all of the above wonderful things

	return 0;
}
/******************************************************************************
 * static float payloadtofloat(struct CANRCVBUF* p);
 * @brief 	: Convert burst msg payload bytes into float
 * @param	: p = pointer to CAN msg buffer
*******************************************************************************/
static float payloadtofloat(struct CANRCVBUF* p)
{
	/* Union for turning payload bytes into float */
	union {
		float f;
		unsigned char uc[8];
	}u;
	u.uc[0] = p->cd.uc[2];
	u.uc[1] = p->cd.uc[3];
	u.uc[2] = p->cd.uc[4];
	u.uc[3] = p->cd.uc[5];
	return u.f;
}
/******************************************************************************
 * void cmd_c_do_msg(struct CANRCVBUF* p);
 * @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg buffer
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, OR, if the cangate.c 'select' 
statement exited with a timeout (1 ms), provided, of course, 
if command 'c' was selected by the hapless Op.
*/
void cmd_c_do_msg(struct CANRCVBUF* p)
{
	int i;
	u32 temp_msk;
	u32 temp;
	/* Union for float to bytes */
	union {
		float f;
		unsigned int i;
		unsigned char uc[8];
	}u;

	/* Were we called by the timer? */
	if (p == NULL)
	{ // Here yes.
		checktimeout();
		return;
	}

	/* Here we were called with a CAN msg? */
	// Check for timeout
	checktimeout();

	/* Check for command CAN id we want. */
	if (!((p->id & 0xfffffffc) == (keybrd_id_c & 0xfffffffc))) return;

	/* Is payload too short to be of any use? */
	if (p->dlc < 2) return;

	/* Handshake v Burst msgs */
	switch(state)
	{ 
	case 0: // Handshake response: msg just received expected to be handshake response
		if (p->dlc != 4)
		{ // Handshake payload length is incorrect for a handshake
			printerror("Handshake response error: dlc not 4: ", p);
			state = 2;
			return;
		}
		/* The dlc is sufficient to proceede. */
		if (p->cd.uc[0] != CMD_LAUNCH_PARM_HDSHK)
		{ // Expected msg was not a handshake (assumes no other CANID_CMD_MCL activity)
			printerror("Unexpected handshake code: ",p);
			printf("	[0] should be %02X\n",CMD_LAUNCH_PARM_HDSHK);
			printf("	      and not %02X\n",p->cd.uc[0]);
			state = 2; // Go to idle state upon next entry.
			return;
		}
		if (p->cd.uc[2] != NUMBER_PARAMETERS)
		{ 	if (p->cd.uc[2] == 0)
			{ // Here, PC telling us parameters are not ready/available
				printf("Zero parameters: PC does not have parameters ready\n");
				state = 2;
				return;
			}
			// PC parameter count does not match ours--something is wrong!
			printerror("Handshake response error: number parameters ", p);
			printf("        our expected count  = %d\n", NUMBER_PARAMETERS);
			printf("          PC response count = %d\n", p->cd.uc[2]);
			state = 2;
			return;			
		}
		if (p->cd.uc[1] < LAUNCH_PARAM_BURST_SIZE)
		{ // Burst count to use: smaller of PC or ours
			lp_burst_size = p->cd.uc[1];
			printf("Working burst size = %d\n", lp_burst_size);
		}
		else
		{
			lp_burst_size = LAUNCH_PARAM_BURST_SIZE;
		}

		/* Handshake success! Request first burst */
		idx_list = 0;	 // Whole list reset
		requestburst(p); // Send request & reset burst index
		state = 1;
		break;

	case 1: // Handle request/receive bursts of msgs
		
		/* Validity and gross error checks. */
		if (p->dlc != 6)
		{ // Burst msg payload length is incorrect
			printerror("Burst msg response error: dlc not 6: ", p);
			return;
		}
		if (p->cd.uc[0] != CMD_SEND_LAUNCH_PARM)
		{ // Expected msg was not a burst msg (assumes no other CANID_CMD_MCL activity)
			printerror("Unexpected handshake code: ",p);
			printf("	[0] should be %02X\n",CMD_SEND_LAUNCH_PARM);
			printf("	      and not %02X\n",p->cd.uc[0]);
			state = 2;
			return;
		}
		if (p->cd.uc[1] > NUMBER_PARAMETERS)
		{ // Index with this parameter is out-of-range
			printerror("Index of parameter is out-of-range: ",p);
			printf("	[1] should be less than %d\n",NUMBER_PARAMETERS);
			printf("			and not %d\n",p->cd.uc[1]);
			return;	// Retry logic will pick this up
		}
		if (p->cd.uc[1] != (idx_list + idx_burst))
		{ // Here, the index in this msg is not what we expect!
			printerror("Index received does not match expected index: ",p);
			printf("	[1] was %d\n",NUMBER_PARAMETERS);
			printf("	came in %d\n",p->cd.uc[1]);			
			return;	// Retry logic will pick this up
		}

		/* Store payload bytes as a float at index */
		flist[idx_list + idx_burst] = payloadtofloat(p);

		/* Mark this burst msg complete by setting burst bit to zero */
		burst_bits = burst_bits & ~(1 << idx_burst);

		idx_burst += 1;	// Advance burst index

		/* Was the parameter just stored the last in the list? */
		if ((idx_list + idx_burst) >= NUMBER_PARAMETERS)
		{ // Here, yes, but was this last, and maybe partial, burst all OK?
			temp_msk = ~0;	// Setup mask for bits for msgs in this burst
			temp_msk = temp_msk << (32 - 1 - idx_burst);
			temp_msk = temp_msk >> (32 - 1 - idx_burst);
			temp = burst_bits & temp_msk;
			if (temp != 0)
			{ // Here, one or more msgs in burst failed
				lp_retry_ctr += 1;
				if (lp_retry_ctr > LAUNCH_PARAM_RETRY_CT)
				{ // Too many retries.  Give up you miserable fool!
					printf("Burst failed at list index: %d\tburst bits %08X\n",(idx_list + idx_burst),temp);
					state = 2;
					return;
				}
				else
				{ // Try this burst again.  Perservere and success may be yours!
					requestburst(p);
				}
				return;
			}
			/*  Huzzah!  All done (except CRC checking which we will implement later. */
			printf("TRANSFER COMPLETE!\n");
			for (i = 0; i < (NUMBER_PARAMETERS-1); i++)
				printf("%2d %f\n", (i+1), flist[i]);
			u.i = flist[i];
			printf("%2d 0x%08X\n",i, u.i); // Last one will be the CRC
			state = 2;
			return;
		}

		/* Was the parameter just stored the last in the burst? */
		if (idx_burst >= lp_burst_size)
		{ // End of burst
			if (burst_bits != 0)
			{ // Here, one or more msgs in burst failed
				lp_retry_ctr += 1;
				if (lp_retry_ctr > LAUNCH_PARAM_RETRY_CT)
				{ // Too many retries.  You tried, but alas, no joy.
					printf("Burst failed at list index: %d\tburst bits %08X\n",(idx_list + idx_burst),burst_bits);
					state = 2;
					return;
				}
				// Try this burst again.  Victory is possible!
				requestburst(p);
				return;
			}
			idx_list += idx_burst;	// Step to next index in list
			lp_retry_ctr = 0;	// Success, so reset if there were retries
			requestburst(p);	// Send burst request and reset idx_burst
		}		
		break;

	case 2: // Idle, but display unexpected msgs.
		printerror("Transfer failed, but CAN id msgs came in: ",p);
		break;
	}
	return;
}

