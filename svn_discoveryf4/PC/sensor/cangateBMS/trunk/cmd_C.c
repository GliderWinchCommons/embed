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
static void printcanmsg(struct CANRCVBUF* p);

//#define PRINTCANMSG

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
static uint8_t cmd_code = EMCL_COOLING_STATUS1; // Default command code

/* Start with default. User can change. */
static uint32_t canid_whoresp = CANID_RX_DEFAULT; // Response 
static uint32_t candid_poller = CANID_TX_DEFAULT; // Default pollster

#define DEFAULT_POLLDUR 0 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Duration between polls

static char subchar = 't'; // Default is temperature from heartbeat
static char subsubchar = 'c'; // Default C verus F deg reporting

static int sw_print_col_hdr = 0; // 0 = no print; print column headers evert 'n'
static uint32_t lctr;

static uint8_t canseqnumber;

static uint32_t timerctr;
static uint32_t timernext; // Next timer count

/* Save & Prep payloads in a CAN msg struct. */
static struct CANRCVBUF can_motry_rx; // Current settings in emcmmc unit
static struct CANRCVBUF can_motry_tx; // Current settings in emcmmc unit
static uint8_t flag_can_motry_rx; // 0 = no current settings

/******************************************************************************
 * static void print_motor_settings(struct CANRCVBUF* pcan);
 * @brief 	: Title says it
 ******************************************************************************/
static void print_motor_settings(struct CANRCVBUF* pcan, uint8_t change)
{	int i;
	printf(" COOLING SYSTEM 4 MOTORS\n"
	       "OC:     1   2   3   4\n"
	       "Index:  8   9  10  11\n Pct:");
	for (i = 0; i < 4; i++)
	{
		if ((change & (1<<i)) != 0) 
			printf("%4i",pcan->cd.uc[i+4]);
		else
			printf(" ...");
	}
	printf("\n");
}
/******************************************************************************
 * static void print_rekat_settings(struct CANRCVBUF* p);
 * @brief 	: Title says it
 ******************************************************************************/
static void print_subry(int i, uint8_t change, struct CANRCVBUF* pcan)
{
	if ((change & (1<<i)) != 0)
	{ // Here, change bit is ON
		if ((pcan->cd.uc[3] & (1<<i)) == 0)
			printf(" OFF"); // Set relay off
		else
			printf("  ON"); // Set on
	}
	else
		printf(" ..."); // No change
	return;
}
static void print_relay_settings_hdr(void)
{
	printf("  COOLING SYSTEM 8 RELAYS\n"
	       "   OA: 1   2   3   4    OB:  1   2   3   4\n"
	       "Index: 0   1   2   3 Index:  4   5   6   7\n    ");
	return;
}
static void print_relay_settings(struct CANRCVBUF* pcan, uint8_t change)
{	int i;
	print_relay_settings_hdr();
	for (i = 0; i < 4; i++)
		print_subry(i, change, pcan);
	printf("      ");
	for (i = 4; i < 8; i++)
		print_subry(i, change, pcan);
	printf("\n");
	return;
}
static void print_relay_settings_chg(struct CANRCVBUF* pcan)
{ // Show ON/OFF for selected settings to be changed
	print_relay_settings(pcan,pcan->cd.uc[2]);
}
static void print_relay_settings_rcv(struct CANRCVBUF* pcan)
{ // Show ON/OFF for all relay settings
	print_relay_settings(pcan,0xFF);	
}
/******************************************************************************
 * static int8_t inputYN(void);
 * @brief 	: Keyboard input 
 ******************************************************************************/
static int8_t inputYN(void)
{
	char buf[32];	
	char* reta = fgets(buf,32,stdin);
	if (reta == NULL) return 0;

	char* p = &buf[0];
	while ((p != &buf[30]) && !((*p == 'Y')||(*p == 'y')))
		p += 1;
	if (p == &buf[30])
		return 0;
	return 1;    
}
/******************************************************************************
 * static int8_t input_relay(struct CANRCVBUF* pcan);
 * @brief 	: Keyboard input 
 * @return  : 0 = no changes; not zero = changes; -1 = failed
 ******************************************************************************/
static int8_t input_relay(struct CANRCVBUF* pcan)
{
	int8_t retx = 0;
	int8_t rety = 0;
	int8_t retz = 0;
	int xx,z;
//	char ctmp;
	// Clear bits for relay update
	pcan->cd.uc[2] = 0; // Change bits
	pcan->cd.uc[3] = 0; // ON/OFF bits
	do
	{
		do
		{
	//		print_relay_settings_rcv(&can_motry_rx);
			printf("Enter relay =>INDEX<= (0-7; abort: 99 1) and OFF/ON (0-1) xx z\n");
			scanf("%i %i",&xx, &z);
			if (xx == 99)
			{
				retx = 1;
				continue;
			}
			if ((xx < 0) || (xx > 7))
			{
				printf("ERROR: index needs to be 0 - 7. It was %i\n",xx);
				retx = 0;
				continue;
			}
			if ((z < 0) || (z > 1))
			{
				printf("ERROR: ON/OFF needs to be 0 or 1. It was %i\n",z);
				retx = 0;
				continue;
			}
			// Set relay ON/OFF bits
			if (z == 0)
				pcan->cd.uc[3] &= ~(1<<xx);
			if (z == 1)
				pcan->cd.uc[3] |=  (1<<xx);

			// Mark relay to be updated
			pcan->cd.uc[2] |= (1<<xx);

			printf("CHANGE: ");
			print_relay_settings_chg(&can_motry_tx);
			retx  = 1;
			retz |= 1; // Return one-or-more relays set
		} while (retx == 0);

		printf("Do another relay? (y/N) ");
		rety = inputYN();
		rety = inputYN(); // Why is 2nd call needed???
printf("input relay: rety %i\n",rety);
	} while (rety == 1);
	return retz;
}
/******************************************************************************
 * static int8_t input_motor(struct CANRCVBUF* pcan);
 * @brief 	: Keyboard input 
 * @return  : 0 = no changes; not zero = changes; -1 = failed
 ******************************************************************************/
static int8_t input_motor(struct CANRCVBUF* pcan)
{
	int8_t retx = 0;
	int8_t retz = 0;
	int8_t rety = 0;
	int xx,z;

	// Clear bits for motor update
	pcan->cd.uc[1] = 0;
	print_motor_settings(&can_motry_rx,0xFF); // Status of all motors
	do
	{
		do
		{
			printf("Enter motor =>INDEX<= (8-11) and percent (0-100) (255 255 to abort), e.g. xx ppp\n");
			scanf("%i %i",&xx, &z);
			if (xx == 255)
			{
				retx = 1;
				continue;
			}
			if ((xx > 11) || (xx < 8))
			{
				printf("motor index should be 8-11, but was %i\n",xx);
				retx = 0;
				continue;
			}
			if ((z > 100) || (z < 0))
			{
				printf("motor percent should be 0 - 100, but was %i\n",z);
				retx = 0;
				continue;
			}
			// Set payload 
			pcan->cd.uc[xx-4] = z; // Set new percent
			pcan->cd.uc[1] |= (1<<(xx-8)); // Set update bit

//printf(" %08X %01d:",pcan->id,pcan->dlc); for (int i = 0; i < pcan->dlc; i++) printf (" %02X",pcan->cd.uc[i]);

			print_motor_settings(&can_motry_tx,can_motry_tx.cd.uc[1]); //Change status
			printf("Do another motor? (y/N) ");
			retx = 1;
			rety = inputYN();
			rety = inputYN();
			retz |= 1;
		} while (retx == 0);
	} while (rety == 1);
	return retz;
}
/******************************************************************************
 * static int8_t Cs_inputs(void);
 * @brief 	: Get Relay and Motor settings
 * @return  : 0 = no changes; not zero = changes
 ******************************************************************************/
static int8_t Cs_inputs(void)
{
	int8_t ret;
	uint8_t retx = 0;

	if (flag_can_motry_rx == 0)
	{
		printf("The latest motor & relay updates for %08X have not been received\n"
			"Wait a few seconds for heartbeat CAN msg\n"
			"and enter Cs again\n",can_motry_rx.id);
		return 0;
	}
	/* RELAYS */
	printf("Recently Received");
	print_relay_settings_rcv(&can_motry_rx);
	can_motry_tx = can_motry_rx; // Copy
	can_motry_tx.id = cantx.id;
	can_motry_tx.dlc = 8; // jic
	printf("Change relay settings? (y/N) ");
	ret = inputYN();
	if (ret != 0)
	{
		retx = input_relay(&can_motry_tx);
	}
	/* MOTORS */
	printf("Recently Received");
	print_motor_settings(&can_motry_rx,0xFF);	
	printf("Change Motor settings? (y/N) ");
	ret = inputYN();
	if (ret != 0)
	{
		retx = input_motor(&can_motry_tx);
	}
	printf("\n\n========= CHANGE settings ===========\n");
	print_relay_settings_chg(&can_motry_tx);
	print_motor_settings(&can_motry_tx, can_motry_tx.cd.uc[1]);	
	printcanmsg(&can_motry_tx);
	printf(    "\n====== DONE Enter next command ======\n\n");
	return retx;
}
/******************************************************************************
 * static void printhelp(void);
 * @brief 	: Print cmd_C.txt
 ******************************************************************************/
static void printhelp(void)
{
	system("cat cmd_C.txt");
}
/******************************************************************************
 * static void printheading_temperature(void);
 * @brief 	: Cc and Cf columng heading
 ******************************************************************************/
static uint8_t oto1_sw; // One-time header print switch
static int repctr1;
static void printheading_temperature(void)
{
	if ((sw_print_col_hdr == 0) && (oto1_sw != 0))
		return;
	repctr1 += 1;
	if ((repctr1 >= sw_print_col_hdr) || (oto1_sw == 0))
	{
		repctr1 = 0;
		printf(      
		"         Alert over-temperature/Status (bits)\n"
		"         |  Pump Outlet\n"
		"         |  |  Motor Oulet\n"
		"         |  |  |  Heat Exchanger Outlet\n"
		"         |  |  |  |  Ambient air\n"
		"         |  |  |  |  |  Spare (jic)\n"
        "         |  |  |  |  |  |  | DMOC CAN msg report\n"
        " payload[1][2][3][4][5][6][7]\n"
		);
		oto1_sw  = 1;
	}
	return;
}
/******************************************************************************
 * static void printheading_relayandmotor(struct CANRCVBUF* pcan);
 * @brief 	: Cg: Column heading relay status and motor pct
 ******************************************************************************/
static void printheading_relayandmotor(struct CANRCVBUF* pcan)
{
	if ((sw_print_col_hdr == 0) && (oto1_sw != 0))
		return;
	repctr1 += 1;
	if ((repctr1 >= sw_print_col_hdr) || (oto1_sw == 0))
	{
		repctr1 = 0;

		printf(      
		"        Group A  Group B    Group C pct\n"
        "        _______  _______  ______________\n"
        "        1 2 3 4  1 2 3 4   1   2   3   4\n"
		);
		oto1_sw  = 1;
	}
	return;
}


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
 * static void printsettings
 * @brief 	: Print static variable settings that can be set
*******************************************************************************/
static void printfsettings(void)
{
	printf("Poll duration: %d (ms)\n",polldur);
	printf("Poller CAN ID: %08X\n",candid_poller); // Pollster
	printf("Responder  ID: %08X\n",canid_whoresp); // Who responds
	printf("Column hdr ct: %8d (0 = off)\n",sw_print_col_hdr);
	return;
}

static void printmenu(char* p)
{
	printf("C - Cooling function\n\t"
				"Cc Temperature (C) \n\t"
				"Cf Temperature (F) \n\t"
				"Cg GET: Cooling motor pct and relay status\n\t"
				"Cs SET: Relays and cooling motors percent\n\t "
				"Cz Stop poll msgs from sending (sets polldur = 0)\n\t"
				"Cp <pppppppp> set Poller CAN ID (EMC1-PC:A1600000(default),(EMC2-PC:A1E00000\n\t"
				"Cw <pppppppp> set Responder CAN ID (EMC1:A0000000(default),(EMC2:A0200000\n\t"
				"Cd <pppppppp> set duration (ms) between polls (default: 0 = no polling)\n\t"
				"Cm <nnn> Printf Column header: 0 = no; n = print every n lines\n\t "
				"Ch HELP: printf cmd_C.txt\n\t"
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
int cmd_C_init(char* p)
{
	int rets;
	uint32_t len = strlen(p);

//	printf("Input replay: %c%c %i\n",*p,*(p+1),len);
	cantx.cd.ull   = 0; // Clear all payload bytes
	cantx.id       = candid_poller; // Pollster ID (Default CANID_TX_DEFAULT)
	cantx.dlc      = 8;
	cantx.cd.uc[1] = cmd_code;

//	flag_can_motry_rx = 0; // motor & relay update flag.

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
			cantx.cd.uc[1] = EMCL_COOLING_STATUS1;
			return 0;

		case 'c': // Report temperature in deg C
			subchar    = 't';
			subsubchar = 'c';
			cantx.cd.uc[1] = EMCL_COOLING_STATUS1;
			return 0;	

		case 'm': // Print column header counts				
			sscanf((p+3), "%d",&sw_print_col_hdr);
			if (sw_print_col_hdr < 0)
				printf("Column header count negative! %d\n",sw_print_col_hdr);
			return -1;

		case 'g': // GET: Cooling motor pct and relay status
			subchar    = 'g';
			cantx.cd.uc[1] = EMCL_MOTOR_RY_STATUS2; //38 GET: Relay status groups OA, OB, and PWM PCT for OC motors		
			break;

		case 's': // Set Relay and motor pct
			rets = Cs_inputs();
			if (rets == 0) return 0;
			subchar    = 's';
			cantx.cd.uc[1] = EMCL_MOTOR_RY_SET; //37 SET: Relays and PWM PCT for motors
			polldur = 1000; // Needs to be at least keep-alive rate
			break;

		case 'z': // Stop polling
			polldur = 0;
			break;			

		case ' ': // Null
			printmenu(p);
			return 0;					

		case 'h': // 'Ch' Help
			printhelp();
			return 0;					

		default:
			printf("2rd char not recognized: %c\n", *(p+1));
			return -1;
		}
	}
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
	printheading_temperature(); // Column header
	printf("%6d ",lctr++);
	printf(" %02X",pcan->cd.uc[1]);
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
 * static void print_g(struct CANRCVBUF* pcan);
 * @brief 	: Display motor pct and relay status
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void print_g(struct CANRCVBUF* pcan)
{
	int i;
	printheading_relayandmotor(pcan); // Column header
	printf("%6d ",lctr++);

	uint8_t tmp = pcan->cd.uc[3]; // Relay status
	for (i = 0; i < 4; i++)
	{
		if ((tmp & (1<<i)) == 0)
			printf(" .");
		else
			printf(" #");
	}
	printf(" ");
	for (i = 4; i < 8; i++)
	{
		if ((tmp & (1<<i)) == 0)
			printf(" .");
		else
			printf(" #");
	}
	for (i = 4; i < 8; i++)
	{
		printf("%4i",pcan->cd.uc[i]);
	}
	return;
}
/******************************************************************************
 * void cmd_C_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs from Cooling function
*******************************************************************************/
void cmd_C_do_msg(struct CANRCVBUF* p)
{
	if ((p->id & 0xfffffffc) != canid_whoresp)
		return; // Not responder we want

//	printf("%5d ",lctr++);
#ifdef PRINTCANMSG
	printcanmsg(p);
#endif	

	if (p->cd.uc[0] == EMCL_MOTOR_RY_STATUS2)	
	{
#ifdef PRINTCANMSG
	printf(" ||");
#endif	
		can_motry_rx = *p; // Save motor and relay settings
		flag_can_motry_rx = 1; // Show we have it.
	}

	switch(subchar)
	{
	case 't': // Temperature display
		if (p->cd.uc[0] != EMCL_COOLING_STATUS1)
			return;
		print_t(p);
		break;

	case 's':
	case 'g': // Motor and relay display
		if (p->cd.uc[0] != EMCL_MOTOR_RY_STATUS2)
			return;	
		print_g(p);
		break;

	default:
		printf("cmd_C_do_msg switch error: CAN command byte not recognized: %c\n",subchar);
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
	struct PCTOGATEWAY pctogateway; 
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
		if (polldur == 0)
		{ // Here, polldur got set to zero after timer was started
			timernext += 1000; // Dummy repeat
		}
		else
		{
			timernext += polldur/10;
			sendcanmsg(&cantx);	// Send next request
		}
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
