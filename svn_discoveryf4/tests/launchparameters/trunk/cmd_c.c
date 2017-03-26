/******************************************************************************
* File Name          : cmd_c.c
* Date First Issued  : 12/03/2016
* Board              : PC
* Description        : Launch parameter transfer test
*******************************************************************************/
/*
This routine is a hack of the cmd_c.c for the PC, in
  ~/GliderWinchCommons/embed/svn_discoveryf4/sensor/gateway_ftdi_2can/trunk
Minor modifications:
  Timeout checking uses DWTTIME
  Pusbutton starts, rather than command c being entered.

Purpose: debugging the PC/HC transfer of launchparameters.

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

#include <unistd.h>

#include "libopencm3/stm32/f4/gpio.h"
#include "cmd_c.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "../../../../../svn_common/trunk/common_can.h"
#include "../../../sw_discoveryf4/trunk/lib/libusartstm32f4/bsp_uart.h"
//#include "clockspecifysetup.h"
#include "DTW_counter.h"
#include "DISCpinconfig.h"
#include "common_can.h"
#include "xprintf.h"

#define UXPRT	6	// Uart number for 'xprintf' messages

/* Debug xprintf */
//#define DEBUGXPRINTF	// define to include

/* Number of parameters we expect */
#define NUMBER_PARAMETERS 28	// Number of parameters in list

/* From parameter database: CMD_CODES_INSERT.sql */
#define CMD_LAUNCH_PARM_HDSHK 38  // Send msg to handshake transferring launch parameters
#define CMD_SEND_LAUNCH_PARM  39  // Send msg to send burst of parameters

#define LAUNCH_PARAM_BURST_SIZE	1	// Maximum number of CAN msgs in a burst when sending launch parameters');
#define LAUNCH_PARAM_RETRY_CT	3	// Number of error retries when sending launch parameters');
#define LAUNCH_PARAM_RETRY_TIMEOUT 500	// Number of milliseconds to wait for a response when sending launch parameters');
#define VER_MCL	1	// Version number

// Default CAN ids for PC/HC (I) and MC (R) 
#define CANID_CMD_MCLI	0xFFE00000	// MCL: Master Controller Launch parameters I (HC)
#define CANID_CMD_MCLR	0XFFE00004	// mcl: Master Controller Launch parameters R (MC)

#define TIMEOUT_USEC    50000	// Timeout increment (microseconds) 
//#define TIMEOUT_USEC 500000	// Timeout increment (microseconds) 0.5 secs
//#define TIMEOUT_USEC 2000000	// Timeout increment (microseconds) 2 secs LONG

extern unsigned int sysclk_freq;

/* Function calls in lptest.c (main) */
void canbuf_add(struct CANRCVBUF* p);


static void requestburst(struct CANRCVBUF* p);

static u32 lp_burst_size;	// Working burst size
static u32 lp_retry_ctr;	// Retry counter

static u32 state;	// Receiving state

//static struct PCTOGATEWAY pctogateway; 

// Default command CAN ids
static u32 keybrd_id_i;	// PC/HC sends with this CAN ID
static u32 keybrd_id_r; // MC sends with this CAN ID

//static u32 canseqnumber;	// Serial ascii/hex msg has a seq number
static struct CANRCVBUF can_last;	// Last CAN msg sent

static u32 idx_burst;	// Index within burst
static u32 idx_list;	// Index within list, 

static u32 burst_bits;	// Bits for msg within burst OK

static float flist[NUMBER_PARAMETERS];

static u32 t1;		// Next time of a timeout
static int flag_to;	// Timeout flag: 0 = inactive; not zero = active

// Overall time duration from request to end
static u32 tim_req;	
static u32 tim_end;

extern int fdp;	/* port file descriptor */

//  Input pin configuration */
static const struct PINCONFIG	input = { \
	GPIO_MODE_INPUT, 	// mode: Output
	0,	 		// 
	0,		 	// speed: fastest 
	GPIO_PUPD_PULLDOWN, 	// pull up/down: pulldown
	0 };			// 

static u32 pbsw; // Pushbutton switch previous
/******************************************************************************
 * static int checkpushbutton(void);
 * @brief	: Check pushbutton w logic for one polled return per press
 * @return	: zero = not pressed; not zero = pressed
*******************************************************************************/
static int checkpushbutton(void)
{
	u32 gpioA1 = GPIO_IDR(GPIOA) & 0X1;
	if (gpioA1 != 0)
	{ // Here, button is pressed
		if (pbsw == 0)
		{
			pbsw = gpioA1;
			return 1;	
		}
	}
	else
	{
		pbsw = gpioA1;
	}
	// Here button not pressed
	// Or pressed, but previously pressed
	return 0;
}

/******************************************************************************
 * void pushbutton_pinsetup(void);
 * @brief 	: Setup PA0 pin setup
*******************************************************************************/
void pushbutton_pinsetup(void)
{
	/*  Setup User pushbutton on Discovery F4 board */
	f4gpiopins_Config ((volatile u32*)GPIOA, 0, (struct PINCONFIG*)&input);
	return;
}
/******************************************************************************
 * static u32 timeval setnexttimeout(int usec);
 * @brief 	: Add an increment to the current time
 * @param	: usec = increment usec (0 to 2147483647)
 * @return	: new time (as processor sysclk ticks)
*******************************************************************************/
static u32 setnexttimeout(u32 usec)
{
	u32 tics = DTWTIME;		// Get current time
	/* Convert usec to processor ticks. */
	tics += (usec * (sysclk_freq/1000000));
	return tics;
}
/******************************************************************************
 * static int timeout(void);
 * @brief 	: Check if current time exceeds the timeout set previously
 * @return	: 0 = no timeout; not zero = timed out
*******************************************************************************/
static int checktimeout(void)
{
	int temp = 0;
	if (flag_to != 0) // Timing active?
	{ // Here yes.
		if ( (int)(DTWTIME - t1) > 0)
		{ // Here we have a timeout
			temp = 1;	// Return the timeout code
			lp_retry_ctr += 1;
			xprintf(UXPRT, "Timeout: retry ctr: %d: retry_size %d\t",lp_retry_ctr, LAUNCH_PARAM_RETRY_CT);
			if (lp_retry_ctr >= LAUNCH_PARAM_RETRY_CT)
			{ // Too many retries.  Give up, miserable fool.
				xprintf(UXPRT, "END OF RETRIES.  Re-execute command\n\r");
				flag_to = 0;	// Stop timeout checks
				state = 2;	// Go to idle state if response msgs come in
				return temp;
			}
			// Try this burst again.  Success may be yours!
			requestburst(&can_last); // Send last msg and set new timeout

			switch (state) // Add msg to show where we are at
			{
			case 0: // Handshake response
				xprintf(UXPRT, "Handshake request\n\r");
				break;
				
			case 1: // Handle request/receive bursts of msgs
				xprintf(UXPRT, "Burst @ list index: %d\tburst bits %08X\n\r",(idx_list + idx_burst),burst_bits);
				break;		
			default:
				xprintf(UXPRT, "Timeout: should not come here!\n\r");
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
	xprintf(UXPRT,"%s ",pc);	// Heading string
	xprintf(UXPRT,"%08X ",p->id);	// CAN id
	xprintf(UXPRT,":%d: ",p->dlc);	// Payload length
	for (i = 0; i < p->dlc; i++) // List payload in hex
		xprintf(UXPRT,"%02X ",p->cd.uc[i]);
	xprintf(UXPRT,"\n\r");	// A mess without this dear ones
	return;
}
/******************************************************************************
 * static void sendmsg_c (struct CANRCVBUF* p);
 * @brief 	: Send CAN msg; save msg; set a timeout; set timer flag
 * @param	: p = pointer to CAN msg buffer
*******************************************************************************/
static void sendmsg_c (struct CANRCVBUF* p)
{
	canbuf_add(p);		// Place in buffer of msgs to go to PC
	can_last = *p;		// Save msg in case of timeout retry.
	t1 = setnexttimeout(TIMEOUT_USEC); // Set next timeout time
	flag_to = 1;		// Set timer flag ON
#ifdef DEBUGXPRINTF
int i;
xprintf(UXPRT,"%08X ",p->id);
for (i = 0; i < p->dlc; i++)
{
  xprintf(UXPRT,"%02X ",p->cd.uc[i]);
}
xprintf(UXPRT," %08x\n\r",burst_bits);
#endif
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
	return;
}
/******************************************************************************
 * static void cmd_c_init(void);
 * @brief 	: Init for transfer request
*******************************************************************************/
static void cmd_c_init(void)
{
	struct CANRCVBUF can;
	struct CANRCVBUF* pcan = &can;

	keybrd_id_i = CANID_CMD_MCLI; // HC Sends with this CAN ID
	keybrd_id_r = CANID_CMD_MCLR; // MC Sends with this CAN ID	

	xprintf(UXPRT, "\n\r\n\r###############################################################\n\r");
	xprintf(UXPRT, "CANIDs being used: PC/HC sends: %08X  MC sends: %08X\n\r", keybrd_id_i, keybrd_id_r);

	state = 0;	// Wait for response to handshake request
	lp_retry_ctr = 0; // Retry counter reset

	/* Setup CAN msg for handshake request. */
	pcan->id       = keybrd_id_r;	// MC sends CAN id
	pcan->dlc      = 4;		// Size of payload
	pcan->cd.uc[0] = CMD_LAUNCH_PARM_HDSHK; // Handshake code
	pcan->cd.uc[1] = LAUNCH_PARAM_BURST_SIZE;	
	pcan->cd.uc[2] = NUMBER_PARAMETERS;
	pcan->cd.uc[3] = VER_MCL;	// Version number

tim_req = DTWTIME;

	/* Send request for launch parameters.
		Send to port number
		Set next timer timeout time
		Save can msg in case timeout requires a retry
		Set timer flag ON (active) */
	sendmsg_c(pcan); // Do all of the above wonderful things

	return;
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
	int ret;
	double dbl;

	/* Union for float to bytes */
	union {
		float f;
		unsigned int i;
		unsigned char uc[8];
	}u;

	/* Check pushbutton */
	ret = checkpushbutton();
	if (ret != 0)
	{ // Here, signal to initiate transfer
		cmd_c_init();
		return;
	}

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

/* Display msgs to this CAN id for debugging & test. */
#ifdef DEBUGXPRINTF
xprintf(UXPRT, "%2d %08X ",debug1++,p->id);
for (i = 0; i < p->dlc; i++)
{
  xprintf(UXPRT, "%02X ",p->cd.uc[i]);
}
xprintf(UXPRT, " %08x\n\r",burst_bits);
#endif

	/* Is payload too short to be of any use? */
	if (p->dlc < 2) return;

	/* Assume a good enough msg to warrant a new timeout. */
	t1 = setnexttimeout(TIMEOUT_USEC); // Set next timeout time

	/* Handshake v Burst msgs */
	switch(state)
	{ 
	case 0: // Handshake response: msg just received expected to be handshake response
		if (p->dlc != 4)
		{ // Handshake payload length is incorrect for a handshake
			printerror("Handshake response error: dlc not 4: ", p);
//$			state = 2; // Failed so go to idle state
			return;
		}
		/* The dlc is sufficient to proceed. */
		if (p->cd.uc[0] != CMD_LAUNCH_PARM_HDSHK)
		{ // Expected msg was not a handshake (assumes no other CANID_CMD_MCL activity)
			printerror("Unexpected handshake code: ",p);
			xprintf(UXPRT, "	[0] should be %02X\n\r",CMD_LAUNCH_PARM_HDSHK);
			xprintf(UXPRT, "	      and not %02X\n\r",p->cd.uc[0]);
			state = 2; // Go to idle state upon next entry.
			return;
		}
		if (p->cd.uc[2] != NUMBER_PARAMETERS)
		{ 	if (p->cd.uc[2] == 0)
			{ // Here, PC telling us parameters are not ready/available
				xprintf(UXPRT, "Zero parameters: PC does not have parameters ready\n\r");
				state = 2; // Failed so go to idle state
				return;
			}
			// PC parameter count does not match ours--something is wrong!
			printerror("Handshake response error: number parameters ", p);
			xprintf(UXPRT, "        our expected count  = %d\n\r", NUMBER_PARAMETERS);
			xprintf(UXPRT, "          PC response count = %d\n\r", p->cd.uc[2]);
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
		xprintf(UXPRT, "Working burst size = %d\n\r", lp_burst_size);

		/* Handshake success! Request first burst */
		idx_list = 0;	 	// Whole list reset
		requestburst(p); 	// Send request for first burst
		burst_bits_reset();	// Set bits for checking off msgs received
		state = 1;		// Next state is handling the burst
		break;

	case 1: // Handle request/receive bursts of msgs
		
		/* Validity and gross error checks. */
		if (p->dlc != 6)
		{ // Burst msg payload length is incorrect.  
			printerror("Burst msg response error: dlc not 6: ", p);
			return;
		}
		if (p->cd.uc[0] != CMD_SEND_LAUNCH_PARM)
		{ // Expected msg was not a burst msg (assumes no other CANID_CMD_MCL activity)
			printerror("Unexpected burst request code: ",p);
			xprintf(UXPRT, "	[0] should be %02X\n\r",CMD_SEND_LAUNCH_PARM);
			xprintf(UXPRT, "	      and not %02X\n\r",p->cd.uc[0]);
			state = 2;
			return;
		}
		if (p->cd.uc[1] > NUMBER_PARAMETERS)
		{ // Index with this parameter is out-of-range
			printerror("Index of parameter is out-of-range: ",p);
			xprintf(UXPRT, "	[1] should be less than %d\n\r",NUMBER_PARAMETERS);
			xprintf(UXPRT, "			and not %d\n\r",p->cd.uc[1]);
			return;	// Retry logic will pick this up
		} 
		// Compute burst index for the parameter index just received
		idx_burst = p->cd.uc[1] - idx_list;
		if (idx_burst >= lp_burst_size)
		{ // Here, msg index is out of bounds to the burst being received
			xprintf(UXPRT, "Index in msg greater than start index plus burst size (or negative): "
			" idx_burst: %d   p->cd.us[1]: %d  idx_list: %d\n\r",idx_burst,p->cd.uc[1],idx_list);
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
					xprintf(UXPRT, "Burst failed at list index: %d\tburst bits %08X\n\r",(idx_list + idx_burst),temp);
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
tim_end = DTWTIME;
			xprintf(UXPRT, "TRANSFER COMPLETE!\n\r");
			for (i = 0; i < (NUMBER_PARAMETERS-1); i++)
			{
				dbl = flist[i];
				xprintf(UXPRT, "%2d %f\n\r", (i+1), dbl);
			}
			u.i = flist[i];
			xprintf(UXPRT, "%2d 0x%08X\n\r",i, u.i); // Last one will be the CRC
double dx1 = (int)(tim_end - tim_req);
xprintf(UXPRT,"Total time (ms): %d %10.3f\n\r",((int)tim_end-(int)tim_req),dx1/sysclk_freq);

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
					xprintf(UXPRT, "Burst failed at list index: %d\tburst bits %08X\n\r",(idx_list + idx_burst),burst_bits);
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

