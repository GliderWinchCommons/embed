/******************************************************************************
* File Name          : cmd_p.c
* Date First Issued  : 01/27/2023
* Board              : PC
* Description        : ELCON Charger 
*******************************************************************************/
#include "cmd_p.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"

extern int fdp;	/* port file descriptor */

static void sendcanmsg(struct CANRCVBUF* pcan);
static void printcanmsg(struct CANRCVBUF* p);

static uint8_t canseqnumber;
static uint32_t lctr; // Output line counter

static void cmd_p_timerthread(void);
static int starttimer(void);
/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
INSERT INTO CANID VALUES ('CANID_ELCON_TX','C7FA872E','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger transmit: ');
INSERT INTO CANID VALUES ('CANID_ELCON_RX','C0372FA4','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger receive: '); */
#define CANID_RX_DEFAULT CANID_ELCON_TX  // C7FA872C' This cmd RECEIVES; ELCON transmits
#define CANID_TX_DEFAULT CANID_ELCON_RX  // C0372FA4' This cmd SENDS; ELCON receives
union UF
{
	uint8_t uc[4];
	float f;
};

struct CELLMSG
{
	double     d; // u32 converted to double

	uint32_t u32; // reading
	uint16_t u16; // reading (100uv)
	uint8_t flag; // 0 = no reading; 1 = new reading; 2 = new bit reading
	uint8_t  max; // reading index: max encountered
};

static struct CANRCVBUF cantx;
static uint32_t canid_rx;

static float fvolts;
static float famps;
static uint32_t ivolts;
static uint32_t iamps;

static float fmsgvolts;
static float fmsgamps;


uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static uint32_t timerctr;
static uint32_t timernext; // Next timer count


#define DEFAULT_POLLDUR 900 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Number of polls per sec

/******************************************************************************
 * static printcanmsg(struct CANRCVBUF* p);
 * @brief 	: CAN msg 
 * @param	: p = pointer 
*******************************************************************************/
static void printcanmsg(struct CANRCVBUF* p)
{
	int i;
	printf(" %08X %01d:",p->id,p->dlc);
	for (i = 0; i < p->dlc; i++)
		printf (" %02X",p->cd.uc[i]);
	return;
}

/******************************************************************************
 * int cmd_p_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_p_init(char* p)
{
	lctr = 0;
	uint32_t len = strlen(p);

	printf("%c%c %i\n",*p,*(p+1),len);
/*

"p - Set voltage and current to zero "
"pd display ELCON msg: CANID: C7FA872E)\n\t"
"ps <vvv.v> <iii.i> Set voltage and current (CANID: C0372FA4)\n\t"
"px Set charger off bit to 1 \n\t"
"po Set charger off bit to 0\n");	
*/

	canid_rx = CANID_RX_DEFAULT; // Default BMS

	/* Set default command to ELCON */
	cantx.id       = CANID_TX_DEFAULT; // 
	cantx.dlc      = 8;
	cantx.cd.ull   = 0; // Clear all payload bytes
	cantx.cd.uc[4] = 1; // Battery protection, charger close output


	/* POLLER requests BMS node, string, or all. */
	switch ( *(p+1) )
	{ 
	case ' ': // 'This is a p' with a space following
		sendcanmsg(&cantx); // Send (default) msg to turn ELCON charging off.
		return 0;

	case 'd': // Display msg =>from<= ELCON, do not send to ELCON
		{
			printf("\nDisplay msg ELCON sends\n");
		}
		return 0;

	case 'm': // Display msg =>to<= ELCON that someone else sends
		return 0; // Skip starting polling	

	case 's': // Enter voltage and current limits 
		if (len > 10)
		{
			sscanf( (p+3), "%f %f",&fvolts, &famps);
			printf(" Volts %6.1f  Amps %6.1f\n",fvolts,famps);
			ivolts = fvolts * 10.0f;
			iamps  = famps * 10.0f;
			if (ivolts > 4000)
			{
				printf("Volts limit is 400.0\n");
				return -1;
			}
			if (iamps > 100)
			{
				printf("Amps limit is 10.0\n");
				return -1;
			}
			cantx.cd.uc[0] = (ivolts >> 8);
			cantx.cd.uc[1] = (ivolts & 0xFF);
			cantx.cd.uc[2] = (iamps >> 8);
			cantx.cd.uc[3] = (iamps & 0xFF);
			cantx.cd.uc[4] = 0; // Charging on
			sendcanmsg(&cantx);
			printcanmsg(&cantx);
			break;
		}
			printf("Not enough chars for ps command: %d",len);
		break;

	case 'j': // Set charger off bit to 1
		cantx.cd.uc[4] = 1;
		sendcanmsg(&cantx);
		break;

	case 'o': // Set charger off bit to 0	
		cantx.cd.uc[4] = 0;
		sendcanmsg(&cantx);
		break;

	default:
		printf("2nd char not recognized: %c\n", *(p+1));
		return -1;
	}

	sendcanmsg(&cantx);
	timerctr   = 0;
	timernext  = polldur/10;
	starttimer();
	return 0;
}

/******************************************************************************
 * void cmd_p_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
static char* tsw[5] = {
	"HW FAIL:",
	"OVR TEMP",
	"INPUT",
	"BATT DISC",
	"COMM TO"};

void cmd_p_do_msg(struct CANRCVBUF* p)
{
		int i;
		int ts;
	/* Expect the BMS node CAN msg format TYPE2, etc
	     and skip other CAN IDs.
	   These #defines originate from the processing of the .sql file
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CMD_CODES_INSERT.sql
	   which generates the file
	     ../../../../../svn_common/trunk/db/gen_db.h */


	if ((p->id & 0xfffffffc) == canid_rx)
	{ // ELCON sent this msg
		printf("%5d ",lctr++);
		printcanmsg(p);

		uint32_t itmpvolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
		uint32_t itmpamps = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
		fmsgvolts = itmpvolts;
		fmsgamps  = itmpamps;	
		printf ("%6.1f V %6.1f I ", fmsgvolts * 0.1, fmsgamps * 0.1);

		for (i = 0; i < 5; i++)
		{
			ts = ((p->cd.uc[4] >> i) & 1);
			printf(" :%s %d",tsw[i],ts);
		}
		printf("\n");
	}
	else if ((p->id & 0xfffffffc) == cantx.id)
	{ // Someone else sent this msg =>to<= the ELCON
		printf("%5d ",lctr++);
		printcanmsg(p);

		uint32_t itmpvolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
		uint32_t itmpamps = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
		fmsgvolts = itmpvolts;
		fmsgamps  = itmpamps;	
		printf ("%6.1f V %6.1f I  CHG ON: %d\n",fmsgvolts*0.1,fmsgamps*0.1,(p->cd.uc[4] & 1));

	}
	return; // CAN ID is not ELCON Charger
}
/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void sendcanmsg(struct CANRCVBUF* pcan)
{
	struct PCTOGATEWAY pctogateway; 

	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
//printf("TX: %08x %d %02X\n",pcan->id, pcan->dlc, pcan->cd.u8[0]);
	return;
}
/******************************************************************************
 * static void cmd_p_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_p_timerthread(void)
{
	timerctr += 1; // 10 ms tick counter
	if ((int)(timerctr - timernext) > 0)
	{ // Time to output accumulated readings
		timernext += polldur/10;
		sendcanmsg(&cantx);	// Send next request
	}
	return;
}
/******************************************************************************
 * static void starttimer(void);
 * @brief 	: Setup timing
*******************************************************************************/
static int starttimer(void)
{
	/* Start timer thread for sending CAN msgs. */
	int ret = timer_thread_init(&cmd_p_timerthread, 10000); // 10 ms
	if (ret != 0)
	{
		printf("\ncmd_p: timer thread startup unsucessful\n");
		return -1;
	}
	printf("START TIMER\n");
	return 0;
}
