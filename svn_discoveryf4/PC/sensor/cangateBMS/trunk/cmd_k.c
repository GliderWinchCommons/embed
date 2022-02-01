/******************************************************************************
* File Name          : cmd_k.h
* Date First Issued  : 08/04/2019
* Board              : PC
* Description        : Keep-alive testing of contactor project
*******************************************************************************/
/*
The following is from the perspective of the contactor node--

RECEIVED msgs directed into contactor function:
 (1) contactor command (also keep-alive) "cid_keepalive_i"
     payload[0]
       bit 7 - connect request
       bit 6 - reset critical error
 (2) poll (time sync) "cid_gps_sync"
 (3) function command (diagnostic poll) "cid_cmd_i"
    
SENT by contactor function:
 (1) contactor command "cid_keepalive_r" (response to "cid_keepalive_i")
     payload[0]
       bit 7 - faulted (code in payload[2])
       bit 6 - warning: minimum pre-chg immediate connect.
              (warning bit only resets with power cycle)
		 bit[0]-[3]: Current main state code

     payload[1] = critical error state error code
         0 = No fault
         1 = battery string voltage (hv1) too low
         2 = contactor 1 de-energized, aux1 closed
         3 = contactor 2 de-energized, aux2 closed
         4 = contactor #1 energized, after closure delay aux1 open
         5 = contactor #2 energized, after closure delay aux2 open
         6 = contactor #1 does not appear closed
         7 = Timeout before pre-charge voltage reached cutoff
         8 = Contactor #1 closed but voltage across it too big
         9 = Contactor #2 closed but voltage across it too big

		payload[2]
         bit[0]-[3] - current substate CONNECTING code
         bit[4]-[7] - current substate (spare) code


*/

#include "cmd_k.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs

static void cmd_k_timerthread(void);

#define DEFAULTRX  0xE3C00000 // CANID_CMD_CNTCTRKAR: U8_U8 : Contactor1: R KeepAlive response
#define DEFAULTTX  0xE3800000 // CANID_CMD_CNTCTRKAI:U8 : Contactor1: I KeepAlive and connect command

#define KEEPALIVEDURATION 300 // Duration between keep-alive msgs (ms)

enum CONTACTOR_STATE
{
	DISCONNECTED,   /*  0 */
	CONNECTING,     /*  1 */
	CONNECTED,      /*  2 */
	FAULTING,       /*  3 */
	FAULTED,        /*  4 */
	RESETTING,      /*  5 */
	DISCONNECTING,  /*  6 */
	OTOSETTLING,    /*  7 */
};

enum CONTACTOR_FAULTCODE
{
	NOFAULT,
	BATTERYLOW,
	CONTACTOR1_OFF_AUX1_ON,
	CONTACTOR2_OFF_AUX2_ON,
	CONTACTOR1_ON_AUX1_OFF,
	CONTACTOR2_ON_AUX2_OFF,
	CONTACTOR1_DOES_NOT_APPEAR_CLOSED,
   PRECHGVOLT_NOTREACHED,
	CONTACTOR1_CLOSED_VOLTSTOOBIG,
	CONTACTOR2_CLOSED_VOLTSTOOBIG,
	KEEP_ALIVE_TIMER_TIMEOUT,
	NO_UART3_HV_READINGS,
	HE_AUTO_ZERO_TOLERANCE_ERR,
};


extern int fdp;	/* port file descriptor */

static u8 canseqnumber = 0;
static struct CANRCVBUF cantx;

static u32 idrx; // CAN id that CAN node sends (to "us)
static u32 idtx; // CAN id that "we" send (to CAN node)

static u32 state;
static u32 kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.

#define KEEPALIVETICKCT 70		// 10 ms count for timing keepalive
static uint32_t tickctr;
static int flagnow;

/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void sendcanmsg(struct CANRCVBUF* pcan)
{
	struct PCTOGATEWAY pctogateway; 
	pcan->id = idtx;
	pcan->dlc = 1;

	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
printf("TX: %08x %d %02X\n",pcan->id, pcan->dlc, pcan->cd.u8[0]);
	return;
}

/******************************************************************************
 * static void starttimer(void);
 * @brief 	: Setup timing
*******************************************************************************/
static int starttimer(void)
{
	/* Start timer thread for sending CAN msgs. */
	int ret = timer_thread_init(&cmd_k_timerthread, 10000);
	if (ret != 0)
	{
		printf("\ncommand K: timer thread startup unsucessful\n");
		return -1;
	}
	return 0;
}
/******************************************************************************
 * int cmd_k_init(char* p);
 * @brief 	: Setup timing
 * @param	: p = pointer to keyboard input
*******************************************************************************/
int cmd_k_init(char* p)
{
	u32 len = strlen(p);

	idrx = DEFAULTRX;
	idtx = DEFAULTTX;

	tickctr = 0;
	flagnow = 0;

	if (len == 2)
	{ // Here, 'k<enter>' entered
		printf("Using DEFAULT CAN IDs: Send %08X  Rcv %08X\n",idtx,idrx);		
		printf("\t to use different CAN IDs, enter exactly in the following format\n\t"
			"k aaaaaaaa bbbbbbbb<enter>\n\t"
			"where: a = ID PC sends, b = ID PC receives\n");
		state = 0;
		kaON = 0;
		return starttimer();
	}
	if (len >= 20)
	{ // Here, Op entered the two CAN IDs
		sscanf((p+1),"%x %x",&idrx,&idrx);
		printf("Using entered CAN IDs: Send %08X  Rcv %08X\n",idtx,idrx);
		state = 0;
		kaON = 0;
		return starttimer();
	}
	if (len == 3)
	{ // Here one of the following commands entered
		switch(*(p+1))
		{
		case 'a':
		case 'A': // 'ka' = Start keep-alive msgs (disconnect)
//			gettimeofday(&te, NULL); // get current time
//   		millisec_prev = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
//			millisec_prev += KEEPALIVEDURATION; // Next time for a keep-alive msg
			flagnow = 1;	 // Send msg upon next timer event
			kaON = 1;  // Show we are in keep-alive mode
			state = 1;			
			break;

		case 'x': // 'kx' = Stop keep-alive msgs (disconnect)
		case 'X':
			flagnow = 1;	 // Send msg upon next timer event
			kaON = 0;  // Show we are not in keep-alive mode
			state = 2;			
			break;

		case '0': // 'k0' = Disconnect (de-energize contactors)
			cantx.cd.uc[0]  = 0x00; //  Request: disconnect
			flagnow = 1;	 // Send msg upon next timer event
			state = 3;
			break;

		case '1': // 'k1' = Connect (energize contactors)
			cantx.cd.uc[0] = 0x80; // Request: connect
			flagnow = 1;	 // Send msg upon next timer event
			state = 4;
			break;

		case '4':
		case '2': // 'k2' = Send fault reset command
			cantx.cd.uc[0] = 0x40; // Request: Reset
			flagnow = 1;	 // Send msg upon next timer event
			state = 5;
			break;

		default:
			printf("%c is not in command list\n",*(p+1));
			return -1;
		}
	}
	return starttimer();
}
/******************************************************************************
 * void cmd_k_do_msg(struct CANRCVBUF* p);
 * @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
extern u32 msg_sw;	// Command in effect

void cmd_k_do_msg(struct CANRCVBUF* p)
{

	if (msg_sw != 'k') return;	// Command in effect

	if ((p->id & 0xfffffffc) != idrx) return;

printf("KA response (status): %08X %i ",p->id,p->dlc);
int i;
for (i = 0; i < p->dlc; i++) printf(" %02X",p->cd.uc[i]);
//printf("\n");

	if (p->dlc != 3)
	{
		printf(" dlc is not 3.  It is %i\n",p->dlc);
		return;
	}

	/* If rcv'd status is RESET, then revert to sending disconnect. */
	if ((p->cd.uc[0] & 0x40) != 0) cantx.cd.uc[0] = 0x00; // Disconnect cmd

	// Primary state code, Substate code
	printf(" : %2i %2i ",(p->cd.uc[0] & 0xf),(p->cd.uc[2] & 0xf));

	switch (p->cd.uc[0] & 0xf)
	{
	case	DISCONNECTED:   /*  0 */
		printf("DISCONNECTED"); 
		break;
	case	CONNECTING:     /*  1 */
		printf("CONNECTING");
		break;	
	case	CONNECTED:      /*  2 */
		printf("CONNECTED ");
		break;	
	case	FAULTING:       /*  3 */
		printf("FAULTING  ");
		break;	
	case	FAULTED:        /*  4 */
		printf("FAULTED   ");
		break;	
	case	RESETTING:      /*  5 */
		printf("RESETTING ");
		break;	
	case	DISCONNECTING:  /*  6 */
		printf("DISCONNECTING");
		break;	
	case	OTOSETTLING:    /*  7 */
		printf("OTOSETTLNG"); 
		break;
	default:
		printf("ARGH: PRIMARY CODE IS NOT RECOGNIZED!");
	}
	printf(" : ");

	switch (p->cd.uc[1])
	{
	case NOFAULT: 
		printf("NO_FAULT");
		break;
	case BATTERYLOW: 
		printf("BATTERYLOW");
		break;
	case CONTACTOR1_OFF_AUX1_ON: 
		printf("CONTACTOR1_OFF_AUX1_ON");
		break;
	case CONTACTOR2_OFF_AUX2_ON: 
		printf("CONTACTOR2_OFF_AUX2_ON");
		break;
	case CONTACTOR1_ON_AUX1_OFF: 
		printf("CONTACTOR1_ON_AUX1_OFF");
		break;
	case CONTACTOR2_ON_AUX2_OFF: 
		printf("CONTACTOR2_ON_AUX2_OFF");
		break;
	case CONTACTOR1_DOES_NOT_APPEAR_CLOSED: 
		printf("CONTACTOR1_DOES_NOT_APPEAR_CLOSED");
		break;
	case PRECHGVOLT_NOTREACHED: 
		printf("PRECHGVOLT_NOTREACHED");
		break;
	case CONTACTOR1_CLOSED_VOLTSTOOBIG: 
		printf("CONTACTOR1_CLOSED_VOLTSTOOBIG");
		break;
	case CONTACTOR2_CLOSED_VOLTSTOOBIG: 
		printf("CONTACTOR2_CLOSED_VOLTSTOOBIG");
		break;
	case KEEP_ALIVE_TIMER_TIMEOUT: 
		printf("KEEP_ALIVE_TIMER_TIMEOUT");
		break;
	case NO_UART3_HV_READINGS:
		printf("UART3_HV_READINGS: timer timed out");
		break;
	case 	HE_AUTO_ZERO_TOLERANCE_ERR:			flagnow = 1;	 // Send msg upon next timer event

		printf("HE_AUTO_ZERO_TOLERANCE_ERR");
		break;
	default:
		printf("ARGH: FAULT CODE IS NOT RECOGNIZED!");
	}
	printf("\n");

	return;
}

/******************************************************************************
 * static void cmd_k_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_k_timerthread(void)
{
	if (kaON == 0) return; // No keep-alive msgs

	tickctr += 1;
	if (tickctr >= KEEPALIVETICKCT)
	{
		tickctr = 0;
		
	/* Send keep-alive msg with the current command code in payload [0] */
		sendcanmsg(&cantx);		
	}
	if (flagnow != 0)
	{
		flagnow = 0;
		sendcanmsg(&cantx);		
	}

	return;
}

