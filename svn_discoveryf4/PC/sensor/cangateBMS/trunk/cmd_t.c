/******************************************************************************
* File Name          : cmd_t.h
* Date First Issued  : 06/28/2024
* Board              : PC
* Description        : Display BMS Cell readings for whole battery string
*******************************************************************************/
/*
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>                  /*  for sleep()  */
#include <curses.h>
#include <math.h>
#include "cmd_t.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"
#include "../../../../../svn_common/trunk/common_can.h"
#include "cmd_t_help.c"

extern int fdp;	/* port file descriptor */

static void sendcanmsg(struct CANRCVBUF* pcan);
static void cmd_t_timerthread(void);
static int starttimer(void);
static void build_mod_readings(struct CANRCVBUF* p, int m);
static void init_ncurses(void);
static void displaycell_ncurses(char* str, int n, uint8_t r, uint8_t c);
static void init_fixed_ncurses(void);
static uint32_t bubble_sort (uint32_t *a, uint32_t n);
static void init_fixed_ncurses_rows(void);
static void init_stringsummary_ncurses(void);

/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds; */
#define CANID_RX_DEFAULT CANID_UNIT_BMS03 // B0A00000','UNIT_BMS03', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #01
#define CANID_TX_DEFAULT CANID_UNI_BMS_PC_I //CANID_UNI_BMS_PC_I        // AEC00000 //BMSV1 UNIversal From PC,  Incoming msg to BMS: X4=target CANID // Default pollster

/* See GliderWinchItems/BMS/bmsadbms1818/Ourtasks/cancomm_items.h */
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
 #define MISCQ_MORSE_TRAP   38 // Retrieve stored morse_trap code.
 #define MISCQ_FAN_STATUS   39 // Retrieve fan: pct and rpm 

#define NSTRING  2 // Max number of strings in a winch
#define NMODULE  8 // Max number of modules in a string
#define NCELL   18 // Max number of cells   in a module
#define NTHERM   3 // Max number of thermistors in a module
#define TIMERNEXTCOUNT 101 // 10 ms tick count between outputs
#define ADCDIRECTMAX 10   // Number of ADCs read in one scan with DMA

static uint32_t adcrate;
/*
	"  8538 usec,  422 Hz",
	"  4814 usec, 1KHz",
	"   742 usec, 27KHz",   
	"   858 usec, 14KHz",   
	"  1556 usec,  7KHz",   
	"  2022 usec,  3KHz",   
  	"134211 usec,   26 Hz",  
	"  2953 usec,  2KHz"
*/

#define CELLVOPEN  65533 // Installed, but wire appears open
#define CELLVNONE  65534 // Cell position not installed

#define HEADERMAX 16 // Number of print groups between placing a header

union UF
{
	uint8_t uc[4];
	float f;
};

struct CELLIDICES
{
	uint8_t s; // String index (1-2)
	uint8_t m; // Module index (1-8)
	uint8_t c; // Cell index (1-18)
	uint8_t n; // Color index (1-13)
};

struct CELLPOS // Cell positions in window
{
	uint8_t r;
	uint8_t c;
};

/* Possible cell has its own info struct. */
struct CELLINFO
{
	double     d; // u32 converted to double
	uint32_t u32; // reading
	uint16_t u16; // reading (100uv)
	uint8_t flag; // 0 = no reading; 1 = new reading; 2 = new bit reading
	uint8_t color; // Color index
	uint8_t ok;  // 0 = use reading for stat
};
struct CELLINFO cellinfo[NMODULE][NCELL];

struct STATAT // Statistic "at" 
{
	double stat;
	uint16_t cellnum;
	uint8_t modnum;
	uint8_t strnum;
};

struct STATS
{
	/* Cell voltages. */
	double sum;
	double ave;
	double max;
	double min;
	double std; // Standard deviation
	uint32_t n; // Number of cells in stats
	uint8_t max_at; // Index for max cell
	uint8_t min_at; // Index for min cell

	/* Others */
	double t[3];    // Temperatures
	double current; // Current sense (calibrated)
	uint8_t bat;    // Status battery
	uint8_t fet;    // Status fets
	uint8_t mod;    // Status mode
	uint8_t fanspeed; // Fan speed (0-100%)
	double  fanrpm;   // Fan rpm
	uint8_t responded; // 0 = no response to poll, 1 = OK
};
struct STATS stats_mod[NMODULE];

/* CAN IDs for modules. */
static uint32_t canidtbl[NMODULE];
static uint8_t  canseqtbl[NMODULE];
static uint8_t idx_modtbl; // Index module: 0 - 8
static uint8_t oto_sw_canidtbl; // 0 = initialized to default

//static struct CELLMSG cellmsg[NCELL];
static struct CANRCVBUF cantx;  // Poll: Cell voltage
static struct CANRCVBUF cantxT; // Poll: Temperature
static struct CANRCVBUF cantxS; // Poll: Status
static struct CANRCVBUF cantxC; // Poll: Current sense
static struct CANRCVBUF cantxF; // Poll: Fan speed & rpm


static uint32_t canid_rx;
//static uint32_t canid_tx; // POLLer CAN ID (is one who "polls")
static uint32_t candid_poller = CANID_TX_DEFAULT; // Default pollster
static uint8_t kaON1 = 0;
static uint8_t canseqnumber;
static uint32_t timerctr;
static uint32_t timernext; // Next timer count

#define DEFAULT_POLLDUR 1000 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Duration between polls

static uint8_t groupctr; // The six cell readings are sent in a group.

WINDOW * mainwin;
static uint8_t colorok = 0; // 0 = Color supported; 1 = not supported
uint8_t windowup = 0; // 0 = Window not up; 1 = Window not closed
static uint8_t ssn_update_sw = 0; // 0 = module summary headings need update
/******************************************************************************
 * static void init_canidtbl(void);
 * @brief 	: Initialize CANID to default ONCE.
*******************************************************************************/
static void init_canidtbl(void)
{
	if (oto_sw_canidtbl != 0) return;

	idx_modtbl = 0;

#if 0	
	canidtbl[0] = 0xB0E00000;
	canidtbl[1] = 0xB1600000;
	canidtbl[2] = 0xB1E00000;
	canidtbl[3] = 0xB2000000;
	idx_modtbl = 6;
//#else
	canidtbl[0] = 0xB1000000;
	canidtbl[1] = 0xB2200000;
	idx_modtbl = 6;
#endif
	oto_sw_canidtbl = 1;
	return;
}
/******************************************************************************
 * static void canidtbl_add(struct CANRCVBUF* p);
 * @brief 	: Add to table of modules
*******************************************************************************/
static void canidtbl_add(struct CANRCVBUF* p)
{
	/* Add to table. */
	canidtbl[idx_modtbl] = p->id;
	idx_modtbl += 1;
	if (idx_modtbl >= NMODULE) idx_modtbl = NMODULE-1;

	/* Sort table. */
	if (idx_modtbl > 1)
	{
		bubble_sort (canidtbl, idx_modtbl);
	}

	/* Update rows */
	init_fixed_ncurses_rows();

	/* update module summary fixed info. */
	ssn_update_sw = 0;
	init_stringsummary_ncurses();
	return;
}

/******************************************************************************
 * static printcanmsg(struct CANRCVBUF* p);
 * @brief 	: CAN msg 
 * @param	: p = pointer 
*******************************************************************************/
#if 0
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
 * static printmenu(char* p);
 * @brief 	: Print boilerplate
 * @param	: p = pointer to line
*******************************************************************************/
static void printfsettings(void)
{
	printf("BMS POLLer %08X\t",cantx.id );
	printf("Poll duration: %d (ms)\n",polldur);
	return;
}
/******************************************************************************/
static void printmenu(char* p)
{
	printf("t - Fixed window BMS string display with module summary and cell voltages\n\t"
				"td  <set: Set duration between polls (default: 1000 ms)\n\t"
				"tp  <set: pppppppp> Poller CAN ID (AEC00000(default),B0000000(EMC1),B0200000(EMC2)) \n\t"
				"th  Display a verbose discription of display\n\t"
				);	
	printfsettings();
	return;
}
/******************************************************************************
 * static void init_cantx(struct CANRCVBUF* p, uint8_t cmdcmd);
 * @brief 	: Set fixed items in CAN msg sent out to poll BMS units
 * @param   : p = pointer to CAN msg
 * @param	: cmdcmd = code for poll
*******************************************************************************/
static void init_cantx(struct CANRCVBUF* p, uint8_t cmdcmd, uint8_t subcmd)
{
	p->cd.ull   = 0; // Clear all payload bytes
	p->id       = candid_poller; // Pollster ID (Default CANID_TX_DEFAULT)
	p->dlc      = 8;
	p->cd.uc[0] = cmdcmd; // Request code
	p->cd.uc[1] = 0xC0;   // All BMS nodes respond
	p->cd.uc[2] = subcmd; // Sub-command
	return;
}
/******************************************************************************
 * static void getset_temp(struct CANRCVBUF* p, uint8_t m);
 * @brief 	: Load temperature reading from CAN msg into module array struct
 * @param   : p = pointer to CAN msg
 * @param	: m = index for stats_mod struct array
*******************************************************************************/
static void getset_temp(struct CANRCVBUF* p, uint8_t m)
{
	uint8_t idx = (p->cd.uc[3] & 0x3);
	if (idx > 2) idx = 2; // JIC

	union FI
	{
		float f;
		uint32_t ui;
	}fi;
	fi.ui = p->cd.ui[1];
	float fpay = fi.f;
	double dpay = fpay;
	stats_mod[m].t[idx] = dpay;
	return;
}
/******************************************************************************
 * static void getset_current(struct CANRCVBUF* p, uint8_t m);
 * @brief 	: Load current sense reading from CAN msg into module array struct
 * @param   : p = pointer to CAN msg
 * @param	: m = index for stats_mod struct array
*******************************************************************************/
static void getset_current(struct CANRCVBUF* p, uint8_t m)
{
	union FI
	{
		float f;
		uint32_t ui;
	}fi;
	fi.ui = p->cd.ui[1];
	float fpay = fi.f;
	double dpay = fpay;
	stats_mod[m].current = dpay;
	return;
}
/******************************************************************************
 * static void getset_status(struct CANRCVBUF* p, uint8_t m);
 * @brief 	: Load status bytes from CAN msg into module array struct
 * @param   : p = pointer to CAN msg
 * @param	: m = index for stats_mod struct array
*******************************************************************************/
static void getset_status(struct CANRCVBUF* p, uint8_t m)
{
	stats_mod[m].bat = p->cd.uc[4];
	stats_mod[m].fet = p->cd.uc[5];
	stats_mod[m].mod = p->cd.uc[6];
	return;
}
/******************************************************************************
 * static void getset_fan(struct CANRCVBUF* p, uint8_t m);
 * @brief 	: Load status bytes from CAN msg into module array struct
 * @param   : p = pointer to CAN msg
 * @param	: m = index for stats_mod struct array
*******************************************************************************/
static void getset_fan_status(struct CANRCVBUF* p, uint8_t m)
{
	union FI
	{
		float f;
		uint32_t ui;
	}fi;
	fi.ui = p->cd.ui[1];
	double dpay = fi.f;
	stats_mod[m].fanrpm = dpay;
	stats_mod[m].fanspeed = p->cd.uc[3];
	return;
}
/******************************************************************************
 * int cmd_t_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
/*
	printf("t - BMS display all cells in static window\n\t"
				"tp  Set polling CAN ID: default AEC00000)\n\t"
				"tt  Enter table of CAN IDs for string and module (default: ....)"
				"td  <duration (ms)> Set duration between polls (Default: 1000 ms)\n"
				);
*/
int cmd_t_init(char* p)
{
	uint32_t len = strlen(p);

//	printf("Input replay: %c%c %i\n",*p,*(p+1),len);

	init_canidtbl(); // Initialize default if not previous done.

	/* Cell voltage poll msg. */
	adcrate = 1; // Index for BMS module for adcrate
//	cantx.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xF) << 0);
	init_cantx(&cantx, CMD_CMD_CELLPOLL, (adcrate << 4) | ((groupctr & 0xF) << 0)); // (42) Request cell voltages

	/* Temperature reading poll (04). */
	init_cantx(&cantxT, CMD_CMD_TYPE2, MISCQ_TEMP_CAL);

	/* Status report. (01) */
	init_cantx(&cantxS, CMD_CMD_TYPE2, MISCQ_STATUS);

	/* Below cell #1 minus, current resistor: calibrated (24) */
	init_cantx(&cantxC, CMD_CMD_TYPE2, MISCQ_CURRENT_CAL);

	/* Fan speed (0-100%) and fan rpm. (39) */
	init_cantx(&cantxF, CMD_CMD_TYPE2, MISCQ_FAN_STATUS);


	if (len < 3)
	{ // Here fat fingered Op
		printmenu(p);
		printf("Too few chars: %d\n",len);
		return -1;
	}
	else
	{	
	/* 2nd char commands */
		switch ( *(p+1) )
		{ 
		case 'a':
				init_ncurses();  // Initialize display window
				init_fixed_ncurses(); // Set row and column numbers in window
				init_stringsummary_ncurses(); // Summary row and columns headers
				break;

		case 'p': // 'tp' Set POLL'er CAN ID
			if (len < 12)
			{
				printf ("Set POLLer CAN ID: input too short: %d\n", len);
				printf ("Example: ep AEC00000\n");
				return -1;
			}
			sscanf( (p+2), "%x",&cantx.id);
			printf("New POLLer CAN ID set to: 0x%08X\n",cantx.id);
			candid_poller = cantx.id;
			printfsettings();
			return 0;

		case 'd': // 'td' Set "Duration" between poll msgs (ms)
			if (len < 5)			
			{
				printf("\nNot enough input chars to set poll duration\n");
				return -1;
			}
			sscanf( (p+3), "%d",&polldur);
			printf("\nPoll duration: %d\n", polldur);
			if (polldur < 100)
			{
				printf("Less than 100 (ms) is too short. ");
			}
			if (polldur > 4000)
			{
				printf("Over 4000 (ms) may be longer than BMS heartbeats! ");
			}
			printf("New value set is: %d (ms)\n",polldur);
			printfsettings();
			return 0;

		case 'h': // 'th' Help
			printmenu(p);
			printf("%s",cmd_t_help);
			return 0;					

		default:
			printf("2nd char not recognized: %c\n", *(p+1));
			canid_rx = 0;
			return -1;
		}

	}

	kaON1      =   1;
	timerctr   =   0;
	timernext  =  50;

	sendcanmsg(&cantx);	// Send first request
	if (polldur != 0)
		starttimer(); // Start timer for repetitive requests
	return 0;
}
/******************************************************************************
 * void cmd_t_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/* This routine is entered each time a CAN msg is received.*/
void cmd_t_do_msg(struct CANRCVBUF* p)
{
		uint8_t m; // String, Module indices

//printf("\n%08X %d: ", p->id,p->dlc);for (i=0; i<p->dlc; i++) printf(" %02X",p->cd.uc[i]);

	uint32_t utmp = (p->id & 0xfffffffc);
	if ((utmp < (uint32_t)CANID_UNIT_BMS01) || (utmp > (uint32_t)CANID_UNIT_BMS18))
		return; // CAN ID is not a BMS module

	/* Here, CAN msg is from a BMS module. */

/* The following is reproduced for your reading pleasure and coding convenience--
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLPOLL',	42,	'[1]-[7] [0] = cell readings request: emc or pc');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_TYPE2',		  43,	'[1]-[7] [0] = format 2: request for misc');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLHB',	  44,	'[1]-[7] [0] = cell readings: responses to timeout heartbeat');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCHB',    45,	'[1]-[7] [0] = misc data: timeout heartbeat TYPE2');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLEMC1',  46,	'[1]-[7] [0] = cell readings: response to emc1 sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLPC',    47,	'[1]-[7] [0] = cell readings: response to pc sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCEMC1',  48,	'[1]-[7] [0] = misc data: response to emc1 sent TYPE2');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCPC',    49,	'[1]-[7] [0] = misc data: response to pc sent TYPE2');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_HEARTBEAT', 50,	'[1]-[7] [0] = Send command code with status (see CANLoader1/canloader_proto.c)');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLEMC2',  51,	'[1]-[7] [0] = cell readings: response to emc2 sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCEMC2',  52,	'[1]-[7] [0] = misc data: response to emc2 sent TYPE2');
*/
	/* Return if response is not to a PC generated poll. */
	if (!( (p->cd.uc[0] == CMD_CMD_MISCPC) || (p->cd.uc[0] == CMD_CMD_CELLPC) ))
		return;

	// Find string and module for this CAN id
	for (m = 0; m < idx_modtbl; m++)
	{
		if (canidtbl[m] == utmp)
		{
			stats_mod[m].responded = 1;	// At least one response from poll
			break; // Found!
		}
	}
	/* Table is expected to have all BMS nodes entered. */
	if (m == idx_modtbl)
	{ // Add CAN id to table
		canidtbl_add(p); 
	}

	if (p->cd.uc[0] == CMD_CMD_CELLPC)
	{ // Here, Response to PC requesting Cell voltages
		build_mod_readings(p,m); // Build module cell readings.
		return;
	}
	if (p->cd.uc[0] == CMD_CMD_MISCPC)		
	{ // Here, Response to PC requesting misc (various) readings
		switch(p->cd.uc[1])
		{
		case MISCQ_TEMP_CAL: // (04) Temperature
			getset_temp(p,m);
			break;
		case MISCQ_CURRENT_CAL: // (24) Current sense calibrated
			getset_current(p,m);
			break;
		case MISCQ_STATUS: // (01) Status: battery, fets, mode
			getset_status(p,m);
			break;
		case MISCQ_FAN_STATUS: // (39) Fan speed, fan rpm			
			getset_fan_status(p,m);
			break;
		}
	}
	return;
}

/******************************************************************************
 * static void init_stringsummary_ncurses(void);
 * @brief 	: Set first column, column headers for module summary info
*******************************************************************************/
#define RX 18
static void init_stringsummary_ncurses(void)
{		
	int i;
	int rx = RX; // Place below lines with modules
	char str[256];
//Is this necessary?
	if (ssn_update_sw == 1) return;
	ssn_update_sw = 1;

	/* Module summary statistics headings. */
	sprintf(str,"    total    ave     max  at    min  at  std Tambi Tcell Texit   I  BATFETMOD D2HCL BCD STB  fan  fanrpm");
	displaycell_ncurses(str, 0, rx, 5);		

	/* Column with row ids */
	rx += 1;
	for (i = 0; i < idx_modtbl; i++)
	{
		sprintf(str,"%04X: ",(canidtbl[i] >> 16));
		// Color white on black, row rx, column 0 
		displaycell_ncurses(str, 0, rx, 0);		
		rx += 2;
	}

	/* String summary statistics. */
	sprintf(str,"TOTAL:");
	displaycell_ncurses(str, 0, rx, 0);		

	return;
}
/******************************************************************************
 * static void prepare_n_display_stringsummary(int m);
 * @brief 	: Total and stats for all modules
*******************************************************************************/
void prepare_n_display_stringsummary(int m)
{	
	int i;
	char str[192];

	/* Cell voltage statistics. */
	double dd  = stats_mod[m].std * 1.0;
	double ds  = stats_mod[m].sum * 0.001;
	double da  = stats_mod[m].ave;
	double dx  = stats_mod[m].max;
	double dn  = stats_mod[m].min;
	int ax = stats_mod[m].max_at;
	int an = stats_mod[m].min_at;
	sprintf(str,"%7.2f %7.1f %7.1f %2d %7.1f %2d %4.1f ",
		          ds,   da,  dx,   ax,  dn,  an,  dd);
	displaycell_ncurses(str, 5, m*2+RX+1, 7);

    /* Thermistor temperature readings. */
	double dt1 = stats_mod[m].t[0];
	double dt2 = stats_mod[m].t[1];
	double dt3 = stats_mod[m].t[2];
	sprintf(str," %5.1f %5.1f %5.1f ",dt1,dt2,dt3);
	displaycell_ncurses(str, 5, m*2+RX+1, 7+42);

	/* Current sense (calibrated) reading. */
	double cur = stats_mod[m].current;
	sprintf(str," %5.1f ",cur);
	displaycell_ncurses(str, 5, m*2+RX+1, 7+42+18);

	/* Status bytes. */
	unsigned int s1 = stats_mod[m].bat;
	unsigned int s2 = stats_mod[m].fet;
	unsigned int s3 = stats_mod[m].mod;
	sprintf(str," %02X %02X %02X ",s1,s2,s3);
	displaycell_ncurses(str, 5, m*2+RX+1, 7+42+18+6);

	/* FET status bits. D2HCL */
	// D - DUMP
	// 2 - DUMP2
	// H - Heater
	// C - Charger
	// L - Low current, cells too low
	//
	// bit = 0 green background (n=8)
	// bit = 1 red background (n=9)
	sprintf(str," ");
	for (i = 0; i < 5; i++)
	{	
		if (s2 & (1 << i))
			displaycell_ncurses(str, 8, m*2+RX+1, 7+42+18+6+9+1+i);
		else
			displaycell_ncurses(str, 9, m*2+RX+1, 7+42+18+6+9+1+i);
	}

	/* Battery status BCD. */
	// B - Balancing in progress
	// C - Charging
	// D - Dump in progress
	sprintf(str," ");
	for (i = 0; i < 3; i++)
	{	
		if (s1 & (1 << (i+4)) )
			displaycell_ncurses(str, 8, m*2+RX+1, 7+42+18+6+9+1+6+i);
		else
			displaycell_ncurses(str, 9, m*2+RX+1, 7+42+18+6+9+1+6+i);
	}

	/* Mode status STB. */
	// S - Self-discharge in progress
	// T - Cells tripped
	// B - Cells tripped and below hysteresis
	sprintf(str," ");
	for (i = 0; i < 3; i++)
	{	
		if (s3 & (1 << i) )
			displaycell_ncurses(str, 8, m*2+RX+1, 7+42+18+6+9+1+6+4+i);
		else
			displaycell_ncurses(str, 9, m*2+RX+1, 7+42+18+6+9+1+6+4+i);
	}

	/* Fan speed and rpm. */
	sprintf(str," %3d %5.0f",stats_mod[m].fanspeed,stats_mod[m].fanrpm);
	displaycell_ncurses(str, 5, m*2+RX+1, 7+42+18+6+9+1+6+4+4);	

	/* Total string voltage. */
	double tsum = 0;
	for (i = 0; i < idx_modtbl; i++)
		tsum += stats_mod[m].sum;
	tsum *= .001; // Convert mv to volts
	sprintf(str,"%8.1f",tsum);
	displaycell_ncurses(str,13, (idx_modtbl*2+RX+1), 5);
	return;	
}
/******************************************************************************
 * static void prepare_n_display(uint8_t m);
 * @brief 	: Set color and make string for one module line
 * @param   : m = module index
*******************************************************************************/
static void prepare_n_display(uint8_t m)
{
	double dtmp,dtmp1;
	uint8_t c; // Cell index
	uint8_t n; // Color index
	char str[192]; // Temp string

	// Threshold around average for color highlight
	double dstdx = stats_mod[m].std * 1.0;
	double dave  = stats_mod[m].ave;

	for (c = 0; c < NCELL; c++)
	{
		switch (cellinfo[m][c].ok)
		{
		case 0: // Set color if cell outside 1.0 std 
			dtmp = ((cellinfo[m][c].d * 0.1) - dave);
			dtmp1 = dtmp;
			if (dtmp1< 0) dtmp1 = -dtmp1;
			if (dtmp1 > dstdx)
			{
				if (dtmp > 0)
					n = 11;
				else
					n = 8; // WHITE on RED
			}
			else
				n = 0; // WHITE on BLACK
			sprintf(str,"%7.1f",(cellinfo[m][c].d * 0.1)); 
			break;
		case 1:
			sprintf(str,"  none  ");
			n = 4;
			break;
		case 2:
			sprintf(str,"  open  ");
			n = 4;
			break;
		case 3:
			sprintf(str," no read");
			n = 4;
			break;
		default:
			sprintf(str," OK?");
			break;
		}
		/* Check if module responsded to poll. */
		if (stats_mod[m].responded == 0)
		{ // Here no responses to all the poll msgs
			n  = 0;
			sprintf(str," ------");
		}
		displaycell_ncurses(str, n, (2*m+1), (c*7)+4);
	}
	return;
}
/******************************************************************************
 * static void build_mod_readings(struct CANRCVBUF* p, int m);
 * @brief 	: Send CAN msg
 * @param	  : pcan = pointer to CANRCVBUF with mesg
 * @param   : m = module index
*******************************************************************************/
static void build_mod_readings(struct CANRCVBUF* p, int m)
{
	int c; // Cell index
	int n;
	double dtmp;
	double dave;
	double dsqsum;
	double dstd;
	double dmax,dmin;
	uint8_t max_at,min_at;
	int nc; // 

	// Check msg is for the same reading group
	if ((p->cd.uc[1] & 0xf) != (canseqtbl[m] & 0xf))
	{ // Here sequence number changed. Assume all readings received
		nc = 0; // Count of cells in statistic
		dave=0;dmax=-100000;dmin=+100000;
		for (c = 0; c < NCELL; c++)
		{
			cellinfo[m][c].ok = 0;
			if (cellinfo[m][c].flag != 0)
			{
				cellinfo[m][c].flag = 0;
				if (cellinfo[m][c].u16 == CELLVNONE)
				{
					cellinfo[m][c].ok = 1;
				}
				else
				{
					if (cellinfo[m][c].u16 == CELLVOPEN)
					{
						cellinfo[m][c].ok = 2;
					}
					else
					{
						dtmp = cellinfo[m][c].u16; // Convert to float
						cellinfo[m][c].d = dtmp;
						dave += dtmp; // Sum for eventual average
						if (dtmp > dmax){dmax=dtmp;max_at=c;}
						if (dtmp < dmin){dmin=dtmp;min_at=c;}
						nc += 1;
						cellinfo[m][c].flag = 0; // Clear was-read flag
					}
				}
			}
			else
			{ // No readings for this cell in this readings group
				cellinfo[m][c].ok = 3;
			}
		}
		/* End of first pass through readlings. */
		/* Compute statistics. */
		if (nc > 0)
		{
			stats_mod[m].max = dmax*0.1;
			stats_mod[m].min = dmin*0.1;
			stats_mod[m].max_at = max_at+1; // 1-18
			stats_mod[m].min_at = min_at+1;

			stats_mod[m].sum = (dave * 0.1);
			dave = dave / nc;
			stats_mod[m].ave = (dave *0.1); // (scale 100 uv to mv)
			dsqsum = 0;
			for (c = 0; c < NCELL; c++)
			{
				if (cellinfo[m][c].ok == 0)
				{
					dtmp = (cellinfo[m][c].d - dave);
					dsqsum += (dtmp * dtmp);
				}
			}
			dstd = 0.1*sqrt(dsqsum/nc);
			stats_mod[m].std = dstd;			
		}

		/* Update to new sequence number. */
		canseqtbl[m] = p->cd.uc[1] & 0xf;
	}

	/* Add readings in payload into array cell positions. */
	// Check payload size 
	switch(p->dlc)
	{
//			case 4: n = 1; break; 
//			case 6: n = 2; break; 
		case 8: n = 3; 
			break; 
		default:
				printf("\nDLC not 8 %d",p->dlc);
				return;
	}

	// Cell number of 1st payload U16
	uint8_t celln = (p->cd.uc[1] >> 4) & 0xf;
	for (c = 0; c < n; c++)
	{
		cellinfo[m][celln+c].u16 = p->cd.us[c+1];
		cellinfo[m][celln+c].flag = 1;
	}
	return;
}
/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void sendcanmsg(struct CANRCVBUF* pcan)
{
//int i;printf("%08X %d: ", pcan->id,pcan->dlc);for (i=0; i<pcan->dlc; i++) printf(" %02X",pcan->cd.uc[i]);printf("\n");

	struct PCTOGATEWAY pctogateway; 
//	pcan->id = canid_tx;
//	pcan->dlc = 1;

	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
	return;
}
/******************************************************************************
 * static void cmd_t_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_t_timerthread(void)
{
	if (polldur == 0)
		return;

	if (kaON1 == 0) return; // No timer generated msgs

	timerctr += 1; // 10 ms tick counter
	if ((int)(timerctr - timernext) > 0)
	{ // Time to output accumulated readings
// But before doing this display the previous poll's results
		/* Display w colors based on stats. */
		int m;
		for (m = 0; m < idx_modtbl; m++)
		{
			/* Update cell voltage rows. */
			prepare_n_display(m);

			/* Update module summary. */
			prepare_n_display_stringsummary(m);

			/* Reset flag that shows response. */
			stats_mod[m].responded = 0;	
		}
			refresh();

		timernext += polldur/10;
//		timer_printresults();

		// Cell group 4 bit counter 
		if (cantx.cd.uc[0] == CMD_CMD_CELLPOLL)
		{
			cantx.cd.uc[2] = (groupctr & 0x0f);
			groupctr += 1;
		}

		/* Send msgs to poll various readings. */
		sendcanmsg(&cantx);	 // Cell voltages (6 responses)
		sendcanmsg(&cantxT); // Temperature readings (3 responses)
		sendcanmsg(&cantxS); // Status (1 response)
		sendcanmsg(&cantxC); // Current sense (1 response)
		sendcanmsg(&cantxF); // Fan speed & rpm (1 response)
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
	int ret = timer_thread_init(&cmd_t_timerthread, 10000); // 10 ms
	if (ret != 0)
	{
		printf("\ncmd_t: timer thread startup unsucessful\n");
		return -1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------- */
/******************************************************************************
 * static void init_ncurses(void);
 * @brief 	: Initialize window
*******************************************************************************/
static void init_ncurses(void)
{
	clear();

	if ( (mainwin = initscr()) == NULL ) 
	{
		printf("Error initializing ncurses.\n");
		return;
	}
	windowup = 1; // Show window initialized (for closing later)
	start_color(); //  Initialize colors 

	/*  Make sure we are able to do what we want. If
	has_colors() returns FALSE, we cannot use colors.
	COLOR_PAIRS is the maximum number of color pairs.  */
	if ( has_colors() && COLOR_PAIRS >= 13 ) 
	{	/* Initialize a bunch of color pairs, where:
	     init_pair(pair number, foreground, background);
	  	 specifies the pair. */
			init_pair(1,  COLOR_RED,     COLOR_BLACK);
			init_pair(2,  COLOR_GREEN,   COLOR_BLACK);
			init_pair(3,  COLOR_YELLOW,  COLOR_BLACK);
			init_pair(4,  COLOR_BLUE,    COLOR_BLACK);
			init_pair(5,  COLOR_MAGENTA, COLOR_BLACK);
			init_pair(6,  COLOR_CYAN,    COLOR_BLACK);
			init_pair(7,  COLOR_BLUE,    COLOR_WHITE);
			init_pair(8,  COLOR_WHITE,   COLOR_RED);
			init_pair(9,  COLOR_BLACK,   COLOR_GREEN);
			init_pair(10, COLOR_BLUE,    COLOR_YELLOW);
			init_pair(11, COLOR_WHITE,   COLOR_BLUE);
			init_pair(12, COLOR_WHITE,   COLOR_MAGENTA);
			init_pair(13, COLOR_BLACK,   COLOR_WHITE);
			return;
	}
	printf("Window colors not supported\n");
	colorok = 1;
	return;
}
/******************************************************************************
 * int close_ncurses(void);
 * @brief 	: Clean up and close window
*******************************************************************************/
int close_ncurses(void)
{
  delwin(mainwin);
  endwin();
  refresh();
  return 0;
}
/******************************************************************************
 * static void displaycell_ncurses(char* str, int n, uint8_t r, uint8_t c);
 * @brief 	: Load string into window with color setting
 * @param   : str = pointer to string
 * @param   : n = color selection index
 * @param   : r = row number
 * @param   : c = column number
*******************************************************************************/
static void displaycell_ncurses(char* str, int n, uint8_t r, uint8_t c)
{
	if (colorok != 0) n = 0;
  color_set(n, NULL);
  mvaddstr(r, c, str);	
}
/******************************************************************************
 * static void init_fixed_ncurses_rows(void);
 * @brief 	: Initialize rows in window
*******************************************************************************/
static void init_fixed_ncurses_rows(void)
{
	int i;
	char str[16];
	int rn = 1; // Initial row number

	/* Init row numbers. (modules numbers within string) */
	for (i = 0; i < idx_modtbl; i++)
	{
		sprintf(str,"%04X:",(canidtbl[i] >> 16));

		// Color white on black, row i, column 0
		displaycell_ncurses(str, 0, rn, 0);
		rn += 2;

	}
	return;
}
/******************************************************************************
 * static void init_fixed_ncurses(void);
 * @brief 	: Initialize fixed fields in window
*******************************************************************************/
static void init_fixed_ncurses(void)
{
	int i;
	char str[190];
	int cx = 3; // Initial column index

	/* Init row numbers. (modules numbers within string) */
	init_fixed_ncurses_rows();

	/* Column header (cell numbers within module) */
	for (i = 0; i < NCELL; i++)
	{
		sprintf(str,"%7d",(i+1));
		// Color white on black, row 0, column 
		displaycell_ncurses(str, 0, 0, cx);		
		cx += 7;
	}

	refresh();
	return;
}                                                                                                                                                                      
/******************************************************************************
 * static uint32_t bubble_sort (uint32_t *a, uint32_t n) ;
 * @brief 	: Sort list
 * @param   : a = pointer to list
 * @param   : n = number of items
*******************************************************************************/
static uint32_t bubble_sort (uint32_t *a, uint32_t n) 
{
	uint32_t cmpct = 0;
    int i, t, j = n, s = 1;
    while (s) {
        s = 0;
        for (i = 1; i < j; i++) 
        {
            if (a[i] < a[i - 1]) 
            {
                t = a[i];
                a[i] = a[i - 1];
                a[i - 1] = t;
                s = 1;
            }
        }
        cmpct += j;
        j--;
    }
    return cmpct;
}