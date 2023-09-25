/******************************************************************************
* File Name          : cmd_E.c
* Date First Issued  : 09/23/23
* Board              : PC
* Description        : PC charging BMS string with ELCON charger
*******************************************************************************/
/*
*/

#include "cmd_E.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"

 #define MISCQ_HEARTBEAT   0   // reserved for heartbeat
 #define MISCQ_STATUS      1 // status
 #define MISCQ_CELLV_CAL   2 // cell voltage: calibrated
 #define MISCQ_CELLV_ADC   3 // cell voltage: adc counts
 #define MISCQ_TEMP_CAL    4 // temperature sensor: calibrated
 #define MISCQ_TEMP_ADC    5 // temperature sensor: adc counts for making calibration
 #define MISCQ_DCDC_V      6 // isolated dc-dc converter output voltage
 #define MISCQ_CHGR_V      7 // charger hv voltage
 #define MISCQ_HALL_CAL    8 // Hall sensor: calibrated
 #define MISCQ_HALL_ADC    9 // Hall sensor: adc counts for making calibration
 #define MISCQ_CELLV_HI   10 // Highest cell voltage
 #define MISCQ_CELLV_LO   11 // Lowest cell voltage
 #define MISCQ_FETBALBITS 12 // Read FET on|off discharge bits
 #define MISCQ_SET_DUMP	  13 // Set ON|OFF DUMP FET on|off
 #define MISCQ_SET_DUMP2  14 // Set ON|OFF DUMP2 FET FET: on|off
 #define MISCQ_SET_HEATER 15 // Set ON|OFF HEATER FET on|off
 #define MISCQ_TRICKL_OFF 17 // Turn trickle charger off for no more than ‘payload [3]’ secs
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage
 #define MISCQ_PROC_CAL   19 // Processor ADC calibrated readings
 #define MISCQ_PROC_ADC   20 // Processor ADC raw adc counts for making calibrations
 #define MISCQ_R_BITS     21 // Dump, dump2, heater, discharge bits
 #define MISCQ_CURRENT_CAL 24 // Below cell #1 minus, current resistor: calibrated
 #define MISCQ_CURRENT_ADC 25 // Below cell #1 minus, current resistor: adc counts
 #define MISCQ_UNIMPLIMENT 26 // Command requested is not implemented
 #define MISCQ_SET_FETBITS  27 // Set FET on/off discharge bits
 #define MISCQ_SET_DCHGTST  28 // Set discharge test via heater fet load on|off
 #define MISCQ_SET_DCHGFETS 30 // Set discharge FETs: all, on|off, or single
 #define MISCQ_SET_SELFDCHG 31 // Set ON|OFF self-discharge mode
 #define MISCQ_PRM_MAXCHG   32 // Get Parameter: Max charging current
 #define MISCQ_SET_ZEROCUR  33 // 1 = Zero external current in effect; 0 = maybe not.
 #define MISCQ_READ_AUX     34 // BMS responds with A,B,C,D AUX register readings (12 msgs)
 #define MISCQ_READ_ADDR    35 // BMS responds with 'n' bytes sent in [3]
 #define MISCQ_PROC_TEMP    36 // Processor calibrated internal temperature (deg C)
 #define MISCQ_CHG_LIMITS   37 // Show params: Module V max, Ext chg current max, Ext. chg bal

FILE* fpIn;
char *paramlist = "../../../../../../../GliderWinchItems/BMS/bmsadbms1818/params/paramIDlist";

extern int fdp;	/* port file descriptor */

static void sendcanmsg(struct CANRCVBUF* pcan);
static void printcanmsg(struct CANRCVBUF* p);
static void discovery(struct CANRCVBUF* p);
static void discovery_init(void);

static uint8_t canseqnumber;

static void cmd_E_timerthread(void);
static int starttimer(void);
/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
INSERT INTO CANID VALUES ('CANID_ELCON_TX','C7FA872E','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger transmit: ');
INSERT INTO CANID VALUES ('CANID_ELCON_RX','C0372FA4','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger receive: '); */
#define CANID_RX_DEFAULT CANID_ELCON_TX  // C7FA872C' This cmd RECEIVES; ELCON transmits
#define CANID_TX_DEFAULT CANID_ELCON_RX  // C0372FA4' This cmd SENDS; ELCON receives

#define CANID_STATUS_POLL CANID_UNI_BMS_PC_I // AEC00000 PC originates

union UF
{
	uint8_t uc[4];
	float f;
};

struct BMSNODE
{
	struct CANRCVBUF canchg; // Discovery: charger limits
	struct CANRCVBUF can;    // Last received CAN msg for this 'id'
	struct CANRCVBUF canstatus; // Last received BMS status msg
	uint32_t id;	         // CAN ID for this node
	uint32_t discoveryct;    // Number of BMS msgs during discovery
	uint32_t discoverylimits;// Number of BMS mgs with limits
	uint32_t timeout;	     // Time when failure to report occurs
   	uint8_t present;         // Reception of status msg has not timed out	
};
static uint8_t bmsnodes_online; // Number of discovered BMS nodes

#define BMSNODESZ 16 // Size of BMS node array
static struct BMSNODE bmsnode[BMSNODESZ];

/* Use this struct for the ELCON. */
static struct BMSNODE elcon; //


#define PARAMLISTSZ 32 // Possible size of paramlist
static uint32_t paramid[PARAMLISTSZ];
static uint8_t paramsize; // Number loaded into list

/* Line buffer size */
#define LINESIZE 2048
static char buf[LINESIZE];

static int8_t state; // 

#define SECSTOWAIT 50 // Duration to monitor
#define BMSPOLL_INTERVAL 30 // Number of 0.1 secs between BMS poll for status
#define DISCOVERY_INTERVAL 90 // Number of 0.1 secs to wait for discovery
#define DISCOVERY_POLL 10 // Polling for discovery interval
#define ELCON_INTERVAL 30 // Number of 0.1 secs to wait for ELCON return msgs
#define ELCON_KEEP_ALIVE 10 // Number of 0.1 secs between ELCON keep-alive
#define CANMSGSTIMEOUT 60
#define PROGRESSTIME 10 // Hapless op keep-alive
static uint32_t timerctr; // Running count of 0.1 sec ticks
static uint32_t polltime;
static uint32_t discoverytime;
static uint32_t discoverytimepoll;
static uint32_t elcon_timer_keep_alive; 
static uint32_t canmsgstimeout; // Input CAN msg timeout
static uint32_t progresstime; 




/* Supply a default number of modules on the string. */
static uint8_t num_bms_modules = 6; // Number of BMS modules on string

static struct CANRCVBUF cantx_status;

static struct CANRCVBUF cantx;
static uint32_t canid_rx;

static float fvolts;
static float famps;
static uint32_t ivolts;
static uint32_t iamps;

static float fmsgvolts;
static float fmsgamps;


uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.

/******************************************************************************
 * static int scansize(int len, int x);
 * @brief 	: Check input chars
*******************************************************************************/
static int scansize(int len, int x)
{
	if (len < x)
	{
		printf("Too few chars in command. Need %d, got %d\n",x,len);
		return -1;
	}
	return 0;
}
/******************************************************************************
 * static void printhelp(void);
 * @brief 	: Help 
*******************************************************************************/
static void printhelp(void)
{
	printf("\n\t"
	"Eh = prints this help\n\t"
	"En <count> = Set new count of BMS nodes on string\n\t"
		);
}
/******************************************************************************
 * static void printsettings(void);
 * @brief 	: Help 
*******************************************************************************/
static void printsettings(void)
{
	printf("Setting----\n\tExpected number BMS modules on string: %d\n",num_bms_modules);
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
 * static int getbmslist(void);
 * @brief 	: Read in file
 * @return  : 0 = success and paramsize is count; -1 = fail
*******************************************************************************/
static int getbmslist(void)
{
	uint32_t id;
	if ( (fpIn = fopen (paramlist,"r")) == NULL)
	{
		printf ("\nFile with possible BMS node CAN IDs did not open: %s\n",paramlist); 
		printf ("Run mklist1.c to generate file\n");
		return -1;
	}
	/* Load list from file. */
	paramsize = 0; 
	while ( (fgets (&buf[0],LINESIZE,fpIn)) != NULL)	// Get a line from stdin
	{
		sscanf(buf,"%8X",&id);
		paramid[paramsize] = id;
//printf("%2d %08X: %s",paramsize,id,buf);
		paramsize += 1;
		if (paramsize >= PARAMLISTSZ)
		{
			printf("Number of lines in CAN ID (paramlist) is too large: %d\n",paramsize);
			return -1;
		}
	}
	printf("Parameter list: paramsize %d\n",paramsize);
	for (int j = 0; j < paramsize; j++)
		printf("%2d %08X\n",j+1,paramid[j]);
	printf("\n");
	return 0;
}
/******************************************************************************
 * int cmd_E_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_E_init(char* p)
{
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

	/* Set command for polling BMS status. */
	cantx_status.id = CANID_STATUS_POLL;
	cantx_status.dlc = 8;
	cantx_status.cd.ui[1] = 0; // CAN ID payload zero
	cantx_status.cd.uc[0] = CMD_CMD_TYPE2;	
	cantx_status.cd.uc[2] = MISCQ_CHG_LIMITS;
	cantx_status.cd.uc[1] = (0x3 << 6); // Nodes respond


	if (len < 3)
	{
		printhelp();
		printsettings();
		return 0;

	}	
	/* POLLER requests BMS node, string, or all. */
	switch ( *(p+1) )
	{ 
	case 'h':
		printhelp();
		printsettings();
		return 0;

	case 'n': // Set number of modules on string
		{
			if (scansize(len,6) != 0) return -1;
			printf("\nUntil cangateBMS restarted, number of modules on string now: %d\n",num_bms_modules);
		}
		return 0;

	case 'v': // Discovery only
		if (scansize(len,2) != 0) return -1;
		printf("Start discovery of BMS nodes\n");
		if (getbmslist() != 0) // Get list of possible BMS nodes
			return -1;
		discovery_init();
		sendcanmsg(&cantx_status);
		break;

	case 's': // Module String whom
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

	case 'o': // et charger off bit to 0	
		cantx.cd.uc[4] = 0;
		sendcanmsg(&cantx);
		break;

	default:
		printf("2nd char not recognized: %c\n", *(p+1));
		return -1;
	}
	timerctr   = 0;
	elcon_timer_keep_alive  = ELCON_KEEP_ALIVE;
	starttimer();
	return 0;
}
/******************************************************************************
 * static void discovery(struct CANRCVBUF* p);
 * @brief 	: Discovery handling of CAN msgs
 * @param   : p = pointer to CAN msg
 * @return  : 
*******************************************************************************/
static void charging_int(void)
{
	int i;
	/* Find minimum charge currents, and compute max string voltage. */


	return;
}

/******************************************************************************
 * static int discovery(void);
 * @brief 	: Discovery time duration expired.
 * @return  : 
*******************************************************************************/
static int discovery_end(void)
{
	int i;
	printf("elcon: %08X\n",elcon.can.id);
	for (i = 0; i < bmsnodes_online; i++)
	{
		printf("%2d %08X code 37: %08X\n",i+1,bmsnode[i].id,bmsnode[i].canchg.id);
	}
	if ((elcon.present ==1) && (bmsnodes_online == num_bms_modules))
	{ // Here, ELCON and expected number of BMS nodes found
		printf("ELCON present and expected BMS node count match\n");
		charging_int(); // Initialize charging phase
		return 2; // Set state to charging phase
	}

	/* Don't keep repeating error msg. */
	if (state == 9) return 9;

	if ((elcon.present !=1))
	{
		printf("ELCON not discovered\n");
	}
	if (bmsnodes_online != num_bms_modules)
	{
		printf ("Discovered BMS nodes count %d not equal expected count %d\n",
			bmsnodes_online,num_bms_modules);	
	}
	return 9; // Discovery failed
}
/******************************************************************************
 * static void discovery_init(void);
 * @brief 	: Initialize for discovery phase
*******************************************************************************/
static void discovery_init(void)
{
	cantx_status.id = CANID_STATUS_POLL;
	cantx_status.dlc = 8;
	cantx_status.cd.ui[1] = 0; // CAN ID payload zero
	cantx_status.cd.uc[0] = CMD_CMD_TYPE2;	
	cantx_status.cd.uc[1] = (0x3 << 6); // Nodes respond
	cantx_status.cd.uc[2] = MISCQ_CHG_LIMITS;
	cantx.cd.ull = 0; // Volts, current and etc. off, but poll ELCON
	bmsnodes_online = 0; // Count BMS nodes found
	bmsnode[0].can.id = 0; // 
	bmsnode[0].canchg.id = 0; // 
	elcon.present = 0; // Show ELCON not yet reported
	elcon.can.id = 0;
	state = 0; // Discovery state
	discoverytime = timerctr + DISCOVERY_INTERVAL;
	discoverytimepoll = timerctr + DISCOVERY_POLL;
	return;
}

/******************************************************************************
 * static void discovery(struct CANRCVBUF* p);
 * @brief 	: Discovery handling of CAN msgs
 * @param   : p = pointer to CAN msg
*******************************************************************************/
static void discovery(struct CANRCVBUF* p)
{
	int i;
	int j;

// NOTE: DMOC will go here also?

	if (CANID_ELCON_RX == p->id)
	{ // ELCON found
		elcon.present = 1;
		elcon.id = p->id;
		elcon.can = *p;
		elcon.timeout = timerctr + ELCON_INTERVAL;
		return;
	}

	// See if CAN ID is in the list of BMS nodes possible
	for (i = 0; i < paramsize; i++)
	{
		if (paramid[i] == p->id)
		{ // Here, found in list of possible BMS nodes
			bmsnode[bmsnodes_online].can = *p; // Save most recent CAN msg
			bmsnode[bmsnodes_online].discoveryct += 1;
			for (j = 0; j < bmsnodes_online; j++)
			{ // Is this CAN ID in our discovery list?
				if (bmsnode[j].id == p->id)
					break;
			}
			if (j == bmsnodes_online)
			{ // Not found in list. Add to online list
				bmsnode[j].id = p->id;
				bmsnodes_online += 1;
				if (bmsnodes_online >= BMSNODESZ)
				{
					printf("ERR: Discovery is finding more than %d\n",BMSNODESZ);
					state = 9;
					return;
				}
				bmsnode[bmsnodes_online].can.id = 0;
				bmsnode[bmsnodes_online].canchg.id = 0;
			}

			if (p->cd.uc[1] == MISCQ_CHG_LIMITS)
			{ // Here msg has the current and voltage limits
				bmsnode[j].canchg = *p; // Save msg with charger limits
				bmsnode[j].timeout = timerctr + BMSPOLL_INTERVAL;
printf("UPD: %2d %2d %2d %08X",i, j,bmsnodes_online,p->id);						
printcanmsg(p);printf("\n");

			}
			return;
				
//printf("%2d %2d %08X",i, bmsnodes_online,p->id);
//printcanmsg(p);
//printf("\n");
		}
	}
	return;
}

/******************************************************************************
 * void cmd_E_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
static char* tsw[5] = {
	"HW FAIL: ",
	"OVR TEMP ",
	"INPUT ",
	"BATT DISC ",
	"COMM T.O. "};

void cmd_E_do_msg(struct CANRCVBUF* p)
{
		int i;

	canmsgstimeout = timerctr + CANMSGSTIMEOUT;

	switch(state)
	{
	case 0: // Discovery 
		discovery(p);
		break;

	case 1: // End Discovery phase Initialize charging
		break;

	case 2: // Charging	
		break;
	
	case 3:
		break;

	case 9:
		break;

	default:
		printf("cmd_E_do_msg: switch statement error: %d\n",state);
		state = 9;
		break;
	}
	return;

	if ((p->id & 0xfffffffc) != canid_rx)
		return; // CAN ID is not ELCON Charger

	printcanmsg(p);

	uint32_t itmpvolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
	uint32_t itmpamps = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
	fmsgvolts = itmpvolts;
	fmsgamps  = itmpamps;	
	printf ("%6.1f V %6.1f I ", fmsgvolts * 0.1, fmsgamps * 0.1);
	int ts;
	for (i = 0; i < 5; i++)
	{
		ts = ((p->cd.uc[4] >> i) & 1);
		printf(" :%s %d",tsw[i],ts);
	}
	printf("\n");

	return;
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
 * static void cmd_E_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_E_timerthread(void)
{
	timerctr += 1; // 100 ms tick running counter
	if ((int)(timerctr - elcon_timer_keep_alive) > 0)
	{ // Time to output accumulated readings
		elcon_timer_keep_alive += ELCON_KEEP_ALIVE;
		sendcanmsg(&cantx);	// Send ELCON poll
	}

	if ((int)(timerctr - canmsgstimeout) > 0)
	{
		printf("CAN msgs are not coming in\n");
		canmsgstimeout += CANMSGSTIMEOUT;
	}

	switch(state)
	{
	case 0: // Discovery phase
		if ((int)(timerctr - progresstime) > 0)
		{ // Something to keep hapless Op occupied
			printf("Discovery: monitoring\n");
			progresstime += PROGRESSTIME;
		}
		if ((int)(timerctr - discoverytime) > 0)
		{ // Here, end of Discovery phase
			state = 1; // Switch how CAN msgs handled
			discovery_end();
		}
		if ((int)(timerctr - discoverytimepoll) > 0)
		{
			discoverytimepoll = timerctr + DISCOVERY_POLL;
			sendcanmsg(&cantx_status);
		}
		break;

	case 1:
	case 2:
	case 3:		
		break;
	case 9:
		break;		

	default: 
		printf("timerthread state err: %d\n",state);
		break;
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
	int ret = timer_thread_init(&cmd_E_timerthread, 100000); // 100ms
	if (ret != 0)
	{
		printf("\ncmd_p: timer thread startup unsucessful\n");
		return -1;
	}
	printf("START TIMER\n");
	return 0;
}
