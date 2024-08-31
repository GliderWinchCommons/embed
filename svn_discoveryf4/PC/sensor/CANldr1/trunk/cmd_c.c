/******************************************************************************
* File Name          : cmd_c.c
* Date First Issued  : 12/03/2016
* Board              : PC
* Description        : Details for handling the 'c' command (list msgs for the id entered)
*******************************************************************************/
/*
CAN msg format--
  Handshake request to PC
    CAN ID = CANID_CMD_MCLR (Send)
    dlc = 4 (payload length)
    cd.uc[0] = CMD_LAUNCH_PARM_HDSHK (handshake code)
    cd.uc[1] = LAUNCH_PARAM_BURST_SIZE (size of burst we can handle)
    cd.uc[2] = Number of parameters in our list
    cd.uc[3] = VER_MCL (parameter list version number)

  Handshake response from PC
    CAN ID = CANID_CMD_MCLI (Listen)
    dlc = 4 (payload length)
    cd.uc[0] = CMD_LAUNCH_PARM_HDSHK (handshake code)
    cd.uc[1] = LAUNCH_PARAM_BURST_SIZE (size of burst we can handle)
    cd.uc[2] = Number of parameters in our list
    cd.uc[3] = VER_MCL (parameter list version number)

  Request to PC to send burst 
    CAN ID = CANID_CMD_MCLR (Send)
    dlc = 2 (payload length)
    cd.uc[0] = CMD_SEND_LAUNCH_PARM (request burst)
    cd.cd[1] = Index of first parameter in the burst

  Response from PC to our send burst request
    CAN ID = CANID_CMD_MCLI (Listen)
    dlc = 6 (payload length)
    cd.uc[0] = CMD_SEND_LAUNCH_PARM (burst response)
    cd.cd[1] = Index of this parameter
    cd.cd[2]-[5] = Floating point number

*/

#include <sys/time.h>
#include <unistd.h>

#include "cmd_c.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

/* Number of parameters we expect */
#define NUMBER_PARAMETERS 28	// Number of parameters in list

/* From parameter database: CMD_CODES_INSERT.sql */
#define CMD_LAUNCH_PARM_HDSHK 38  // Send msg to handshake transferring launch parameters
#define CMD_SEND_LAUNCH_PARM  39  // Send msg to send burst of parameters

#define LAUNCH_PARAM_BURST_SIZE	16	// Maximum number of CAN msgs in a burst when sending launch parameters');
#define LAUNCH_PARAM_RETRY_CT	3	// Number of error retries when sending launch parameters');
#define LAUNCH_PARAM_RETRY_TIMEOUT 500	// Number of milliseconds to wait for a response when sending launch parameters');
#define VER_MCL	1	// Version number

// Default CAN ids for PC/HC (I) and MC (R) 
#define CANID_CMD_MCLI	0xFFE00000	// MCL: Master Controller Launch parameters I (HC)
#define CANID_CMD_MCLR	0XFFE00004	// mcl: Master Controller Launch parameters R (MC)

#define TIMEOUT_USEC   200000	// Timeout increment (microseconds) 0.2 secs
//#define TIMEOUT_USEC 500000	// Timeout increment (microseconds) 0.5 secs
//#define TIMEOUT_USEC 2000000	// Timeout increment (microseconds) 2 secs LONG

static void requestburst(struct CANRCVBUF* p);

static u32 lp_burst_size;	// Working burst size
static u32 lp_retry_ctr;	// Retry counter

static u32 state;	// Receiving state

static struct PCTOGATEWAY pctogateway; 

// Default command CAN ids
static u32 keybrd_id_i;	// PC/HC sends with this CAN ID
static u32 keybrd_id_r; // MC sends with this CAN ID

static u32 canseqnumber;	// Serial ascii/hex msg has a seq number
static struct CANRCVBUF can_last;	// Last CAN msg sent

static u32 idx_burst;	// Index within burst
static u32 idx_list;	// Index within list, 

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
 * static void burst_bits_reset(void);
 * @brief 	: Set bits for checking off received msgs in a burst
*******************************************************************************/
static void burst_bits_reset(void)
{
	/* Clear burst msg received index array by setting bits ON
	   for the burst size, and the higher order bits OFF.
	   Turn off bits as burst msg comes in to show msg was
	   received and passed checks. */
	burst_bits = ~0;	// All bits ON
	burst_bits = burst_bits << (32 - lp_burst_size);
	burst_bits = burst_bits >> (32 - lp_burst_size);
	return;
}

/******************************************************************************
 * static void requestburst (struct CANRCVBUF* p);
 * @brief 	: 
 * @param	: p = pointer to CAN msg buffer
*******************************************************************************/
static void requestburst(struct CANRCVBUF* p)
{
	p->id = keybrd_id_r;
	p->dlc = 2;	// Size of payload for burst request
	p->cd.uc[0] = CMD_SEND_LAUNCH_PARM; // Code for burst request
	p->cd.uc[1] = idx_list; // Index for first parameter in burst
	sendmsg_c(p);

	idx_burst = 0;	// Reset index within the burst

	t1 = setnexttimeout(0, TIMEOUT_USEC); // Set next timeout time
int i;
printf("%08X ",p->id);
for (i = 0; i < p->dlc; i++)
{
  printf("%02X ",p->cd.uc[i]);
}
printf(" %08x\n",burst_bits);

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

	/* Check keyboard entry and setup up the two CAN IDs */
	if (strlen(p) < 1)
	{ // Here too few chars
		printf("%d is too few chars for the 'c' command, example\nc FFE00000 FFE00004 "\
		"[CAN ID that PC-sends MC-sends]\nDefault CAN IDs is simply 'c'\n",(int)strlen(p));
		return -1;
	}
	// Check if default IDs are to bel used
	if (strlen(p) < 4)
	{ // Use default CAN ID assignments
		keybrd_id_i = CANID_CMD_MCLI; // HC Sends with this CAN ID
		keybrd_id_r = CANID_CMD_MCLR; // MC Sends with this CAN ID	
	}
	else
	{
		if (strlen(p) < 20)
		{ // Not enough chars for two CAN IDs
			printf("%d is too few chars for the 'c' command with both CAN ids entered, example\n"\
			"c FFE00000 FFE00004 [CAN ID that PC-sends MC-sends]\nDefault CAN IDs is simply 'c'\n",(int)strlen(p));
			return -1;
		}
		sscanf( (p+1), "%x %x",&keybrd_id_i,&keybrd_id_r); // Get CAN IDs from keyboard entry
	}

	printf ("CANIDs being used: PC/HC sends: %08X  MC sends: %08X\n", keybrd_id_i, keybrd_id_r);

	state = 0;	// Wait for response to handshake request
	lp_retry_ctr = 0; // Retry counter reset

	/* Setup CAN msg for handshake request. */
	pcan->id       = keybrd_id_r;	// MC sends CAN id
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
int i;
printf("%08X ",pcan->id);
for (i = 0; i < pcan->dlc; i++)
{
  printf("%02X ",pcan->cd.uc[i]);
}
printf("\n");

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
static int debug1;
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
	if (!((p->id & 0xfffffffc) == (keybrd_id_i & 0xfffffffc))) return;

printf("%2d %08X ",debug1++,p->id);
for (i = 0; i < p->dlc; i++)
{
  printf("%02X ",p->cd.uc[i]);
}
printf(" %08x\n",burst_bits);

	/* Is payload too short to be of any use? */
	if (p->dlc < 2) return;

	/* Handshake v Burst msgs */
	switch(state)
	{ 
	case 0: // Handshake response: msg just received expected to be handshake response
		if (p->dlc != 4)
		{ // Handshake payload length is incorrect for a handshake
			printerror("Handshake response error: dlc not 4: ", p);
			state = 2; // Failed so go to idle state
			return;
		}
		/* The dlc is sufficient to proceed. */
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
				state = 2; // Failed so go to idle state
				return;
			}
			// PC parameter count does not match ours--something is wrong!
			printerror("Handshake response error: number parameters ", p);
			printf("        our expected count  = %d\n", NUMBER_PARAMETERS);
			printf("          PC response count = %d\n", p->cd.uc[2]);
			state = 2; // Failed so go to idle state
			return;			
		}
		if (p->cd.uc[1] < LAUNCH_PARAM_BURST_SIZE)
		{ // Burst count to use: smaller of PC or ours
			lp_burst_size = p->cd.uc[1];
		}
		else
		{
			lp_burst_size = LAUNCH_PARAM_BURST_SIZE;
		}
               
		printf("Working burst size = %d\n", lp_burst_size);
		printf("    our size limit = %d\n"
		       "     they will use = %d\n", LAUNCH_PARAM_BURST_SIZE, p->cd.uc[1]);

		/* Handshake success! Request first burst */
		idx_list = 0;	 	// Whole list reset
		requestburst(p); 	// Send request
		burst_bits_reset();	// Set bits for checking off msgs received
		state = 1;		// Next state is handling the burst
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
			printerror("Unexpected burst request code: ",p);
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
		// Compute burst index for the parameter index just received
		idx_burst = p->cd.uc[1] - idx_list;
		if (idx_burst >= lp_burst_size)
		{ // Here, msg index is out of bounds to the burst being received
			printf("Index in msg greater than start index plus burst size (or negative): "
			" idx_burst: %d   p->cd.us[1]: %d  idx_list: %d\n",idx_burst,p->cd.uc[1],idx_list);
			return;
		}

		/* Store payload bytes as a float at index */
		flist[p->cd.uc[1]] = payloadtofloat(p);

		/* Zero bit corresponding the the burst index, to show msg was stored. */
		burst_bits = burst_bits & ~(1 << idx_burst);

		/* Was the parameter just stored the last in the list? */
		if ((idx_list + idx_burst + 1) >= NUMBER_PARAMETERS)
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
			flag_to = 0;	// Disable timer
			state = 2;
			return;
		}

		/* Was the parameter just stored the last in the burst? */
		if ((idx_burst + 1) >= lp_burst_size)
		{ // End of burst
			if (burst_bits != 0)
			{ // Here, one or more msgs in burst failed
				lp_retry_ctr += 1;
				if (lp_retry_ctr > LAUNCH_PARAM_RETRY_CT)
				{ // Too many retries.  You tried, but alas, no joy.
					printf("Burst failed at list index: %d\tburst bits %08X\n",(idx_list + idx_burst),burst_bits);
					state = 2; // Idle state
					return;
				}
				// Try this burst again.  Victory is possible!
				requestburst(p);
				return;
			}
			/* Here, all msgs in the burst were OK and stored. */
			idx_list += lp_burst_size;// Step to next burst start index in list
			lp_retry_ctr = 0;	// Success was ours, so reset if there were retries
			requestburst(p);	// Send new burst request
			burst_bits_reset();	// Set bits for checking off msgs received
		}		
		break;

	case 2: // Idle, but display unexpected msgs.
		printerror("Transfer terminated, but CAN id msgs still came in(!): ",p);
		break;
	}
	return;
}

