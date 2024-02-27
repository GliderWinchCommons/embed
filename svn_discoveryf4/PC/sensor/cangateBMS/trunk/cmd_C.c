/******************************************************************************
* File Name          : cmd_C.h
* Date First Issued  : 02/24/2024
* Board              : PC
* Description        : Subcommands for bmsmot/emcmmc Cooling function
*******************************************************************************/
/*
*/

#include "cmd_C.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"
/* CAN msg commands for Cooling sub-function. */
#include "../../../../../../../GliderWinchItems/bmsmot/emcmmc/F446/Ourtasks/EMCLTaskCmd.h"

extern int fdp;	/* port file descriptor */

static void sendcanmsg(struct CANRCVBUF* pcan);
static void cmd_C_timerthread(void);
static int starttimer(void);

/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds; */
#define CANID_RX_DEFAULT CANID_UNIT_EMCMMC1   //','A0000000','bmsmot 1 EMC/MMC Local1');
#define CANID_TX_DEFAULT CANID_CMD_EMCMMC1_PC //

#define HEADERMAX 16 // Number of print groups between placing a header


union UF
{
	uint8_t uc[4];
	float f;
};

static struct CANRCVBUF cantx;
static uint32_t canid_rx;
static uint32_t canid_tx; // POLLer CAN ID (is one who "polls")

/* Start with default. User can change. */
static uint32_t canid_whoresp = CANID_RX_DEFAULT; // Response 
static uint32_t candid_poller = CANID_TX_DEFAULT; // Default pollster

#define DEFAULT_POLLDUR 0 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Duration between polls

static char subchar = 't'; // Default is temperature from heartbeat
static char subsubchar = 'c'; // Default C verus F deg reporting

static uint8_t reqtype; // Request type (miscq code)
static uint8_t canseqnumber;


uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static uint32_t timerctr;
static uint32_t timernext; // Next timer count

static uint8_t ncell_prev;
static uint8_t headerctr;


/******************************************************************************
 * static printcanmsg(struct CANRCVBUF* p);
 * @brief 	: CAN msg 
 * @param	: p = pointer 
*******************************************************************************/
#if 1
static void printcanmsg(struct CANRCVBUF* p)
{
	int i;
	printf(" %08X %01d:",p->id,p->dlc);
	for (i = 0; i < p->dlc; i++)
		printf (" %02X",p->cd.uc[i]);
	return;
}
#endif
/******************************************************************************
 * static void printsettings
 * @brief 	: Print static variable settings that can be set
*******************************************************************************/
static void printfsettings(void)
{
	printf("Poll duration: %d (ms)\n",polldur);
	printf("Poller CAN ID: %08X\n",candid_poller); // Pollster
	printf("Responder  ID: %08X\n",canid_whoresp); // Who responds
	return;
}

static void printmenu(char* p)
{
	printf("C - Cooling function\n\t"
				"Cc Temperature (C) \n\t"
				"Cf Temperature (F) \n\t"
				"Cp <pppppppp> set Poller CAN ID (EMC1-PC:A1600000(default),(EMC2-PC:A1E00000\n\t"
				"Cw <pppppppp> set Responder CAN ID (EMC1:A0000000(default),(EMC2:A0200000\n\t"
				"Cd <pppppppp> set duration (ms) between polls (default: 0 = no polling)\n\t"
				);	
	printfsettings();
	return;
}

/******************************************************************************
 * int cmd_e_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
/*
 */

int cmd_C_init(char* p)
{
	uint32_t len = strlen(p);

//	printf("Input replay: %c%c %i\n",*p,*(p+1),len);

	cantx.cd.ull   = 0; // Clear all payload bytes
	cantx.id       = candid_poller; // Pollster ID (Default CANID_TX_DEFAULT)
	cantx.dlc      = 8;

	if (len < 3)
	{
		printmenu(p);
		printf("Too few chars: %d\n",len);
		return -1;
	}
	else
	{	
		switch ( *(p+1) )
		{ 
		case 'w': // 'Cw' Set "Who" responds
			if (len < 3)
			{
				printf("Need more info--Enter 0 or a CAN ID\n");
				return -1;
			}
			if (len < 12)
			{ // Not enough chars for a CAN ID
				canid_whoresp = 0;
				printf("New: All nodes respond: %d\n",canid_whoresp );
				return 0;
			}
			cantx.cd.ui[1] = canid_whoresp; // CAN ID for responder
			printfsettings();
			return 0;
	
		case 'p': // 'Cp' Set POLL'er CAN ID
			if (len < 12)
			{
				printf ("Set POLLer CAN ID: input too short: %d\n", len);
				printf ("Example: Cp A1E00000\n");
				return -1;
			}
			sscanf( (p+2), "%x",&cantx.id);
			printf("New POLLer CAN ID set to: 0x%08X\n",cantx.id);
			candid_poller = cantx.id;
			printfsettings();
			break;

		case 'd': // 'Cd' Set "Duration" between poll msgs (ms)
			if (len < 5)			
			{
				printf("\nNot enough input chars to set poll duration\n");
				return -1;
			}
			sscanf( (p+3), "%d",&polldur);
			printf("\nPoll duration: %d\n", polldur);
			if (polldur == 0)
			{
				printf("NOTE: Zero = no polling");
			}
			printfsettings();
			break;

		case 'f': // Report temperature in deg F
			subchar    = 't';
			subsubchar = 'f';
			return 0;			

		case 'c': // Report temperature in deg C
			subchar    = 't';
			subsubchar = 'c';
			return 0;		

		case ' ': // Null
			printmenu(p);
			return 0;					

		case 'h': // 'Ch' Help
			printmenu(p);
			return 0;					

		default:
			printf("2rd char not recognized: %c\n", *(p+1));
			return -1;
		}
	}
	ncell_prev =   0;
	headerctr  = HEADERMAX;
	kaON       =   1;
	timerctr   = 0;
	timernext  = polldur/10;

	if (polldur != 0)
	{
		sendcanmsg(&cantx);	// Send first request
		starttimer(); // Start timer for repetitive requests
	}
	return 0;
}
/******************************************************************************
 * static void print_t(struct CANRCVBUF* pcan);
 * @brief 	: Display temperature
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void print_t(struct CANRCVBUF* pcan)
{
	int i;
	float ftmp;
	for (i = 2; i < 8; i++)
	{
		if (pcan->cd.uc[i] == 0xA5)
		{ // Here, not installed
			printf(" 00");
		}
		else
		{
			if (subsubchar == 'f')
			{ // Convert to F
				ftmp = pcan->cd.uc[i];
				ftmp = (ftmp * (9.0/5.0)) + 32;
				printf("%3.0f",ftmp);
			}
			else
			{ // Default: C
				printf("%3i",pcan->cd.uc[i]);
			}
		}
	}
}
/******************************************************************************
 * void cmd_C_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs from Cooling function
*******************************************************************************/
static uint32_t lctr;
void cmd_C_do_msg(struct CANRCVBUF* p)
{
	if ((p->id & 0xfffffffc) != canid_whoresp)
		return; // Not responder we want

	printf("%5d ",lctr++);
//	printcanmsg(p);

	switch(subchar)
	{
	case 't':
		print_t(p);
		break;
	default:
		printf("cmd_C_do_msg switch error: %c\n",subchar);
		break;
	}

	printf("\n");

	return;
}
/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
uint32_t www;
static void sendcanmsg(struct CANRCVBUF* pcan)
{
//int i;printf("%08X %d: ", pcan->id,pcan->dlc);for (i=0; i<pcan->dlc; i++) printf(" %02X",pcan->cd.uc[i]);printf("\n");

	struct PCTOGATEWAY pctogateway; 
//	pcan->id = canid_tx;
//	pcan->dlc = 1;

	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
//printcanmsg(pcan);	printf(" %4d\n",www++);
	return;
}
/******************************************************************************
 * static void cmd_C_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_C_timerthread(void)
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
	int ret = timer_thread_init(&cmd_C_timerthread, 10000); // 10 ms
	if (ret != 0)
	{
		printf("\ncmd_p: timer thread startup unsucessful\n");
		return -1;
	}
	printf("START TIMER\n");
	return 0;
}
