/******************************************************************************
* File Name          : cmd_e.h
* Date First Issued  : 02/03/2022
* Board              : PC
* Description        : Display BMS misc polled command responses
*******************************************************************************/
/*
*/

#include "cmd_e.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"

extern int fdp;	/* port file descriptor */

static void sendcanmsg(struct CANRCVBUF* pcan);
static void printcanmsg(struct CANRCVBUF* p);
static float extractfloat(uint8_t* puc);
static uint32_t extractu32(uint8_t* puc);
static void miscq_cellv_cal(struct CANRCVBUF* p);
static void miscq_cellv_adc(struct CANRCVBUF* p);
static void miscq_temp_cal(struct CANRCVBUF* p);
static void miscq_temp_adc(struct CANRCVBUF* p);
static void miscq_topofstack(struct CANRCVBUF* p);
static void miscq_miscq_dcdc_v(struct CANRCVBUF* p);
static void miscq_fetbalbits (struct CANRCVBUF* p);
static void miscq_proc_cal(struct CANRCVBUF* p);
static void miscq_proc_adc(struct CANRCVBUF* p);
//static void miscq_bits_r(struct CANRCVBUF* p);
//static void timer_printresults(void);
//static void print_cal_adc(char* pfmt, uint8_t ncol);
//static void print_processor_adc_header(void);
//static void print_bits_r(void);

static void cmd_e_timerthread(void);
static int starttimer(void);
/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds; */
#define CANID_RX_DEFAULT CANID_UNIT_BMS03 // B0A00000','UNIT_BMS03', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #01
#define CANID_TX_DEFAULT CANID_UNI_BMS_PC_I //CANID_UNI_BMS_PC_I        // B0200000 //BMSV1 UNIversal From PC,  Incoming msg to BMS: X4=target CANID // Default pollster

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
 #define MISCQ_FETBALBITS 12 // Read FET on/off discharge bits
 #define MISCQ_DUMP_ON	  13 // Turn on Dump FET for no more than ‘payload [3]’ secs
 #define MISCQ_DUMP_OFF	  14 // Turn off Dump FET
 #define MISCQ_HEATER_ON  15 // Enable Heater mode to ‘payload [3] temperature
 #define MISCQ_HEATER_OFF 16 // Turn Heater mode off.
 #define MISCQ_TRICKL_OFF 17 // Turn trickle charger off for no more than ‘payload [3]’ secs
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage
 #define MISCQ_PROC_CAL   19 // Processor ADC calibrated readings
 #define MISCQ_PROC_ADC   20 // Processor ADC raw adc counts for making calibrations
 #define MISCQ_R_BITS     21 // Dump, dump2, heater, discharge bits
 #define MISCQ_CURRENT_CAL 24 // Below cell #1 minus, current resistor: calibrated
 #define MISCQ_CURRENT_ADC 25 // Below cell #1 minus, current resistor: adc counts
 #define MISCQ_UNIMPLIMENT 26 // Command requested is not implemented
 #define MISCQ_SETFETBITS  27 // Set FET on/off discharge bits
 #define MISCQ_SETDCHGTST_ON  28 // Set discharge test with heater fet load: ON
 #define MISCQ_SETDCHGTST_OFF 29 // Set discharge test with heater fet load: OFF


#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

#define NSTRING  2 // Max number of strings in a winch
#define NMODULE  8 // Max number of modules in a string
#define NCELL   18 // Max number of cells   in a module
#define NTHERM   3 // Max number of thermistors in a module
#define TIMERNEXTCOUNT 101 // 10 ms tick count between outputs
#define ADCDIRECTMAX 10   // Number of ADCs read in one scan with DMA

#define HEADERMAX 16 // Number of print groups between placing a header

static uint32_t adcrate;
static char* pADCrate[] =
{
	"  8538 usec,  422 Hz",
	"  4814 usec, 1KHz",
	"   742 usec, 27KHz",   
	"   858 usec, 14KHz",   
	"  1556 usec,  7KHz",   
	"  2022 usec,  3KHz",   
  	"134211 usec,   26 Hz",  
	"  2953 usec,  2KHz"
};

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
static struct CELLMSG cellmsg[NCELL];
static struct CANRCVBUF cantx;
static uint32_t canid_rx;
static uint32_t canid_tx; // POLLer CAN ID (is one who "polls")

static uint32_t canid_whoresp = CANID_RX_DEFAULT; // Who responds?
static uint32_t candid_poller = CANID_TX_DEFAULT; // Default pollster

static uint8_t reqtype; // Request type (miscq code)
static uint8_t canseqnumber;
static uint8_t cd_uc1 = (0x1 << 6); // Who responds: canid_tx.cd.uc[1] default: only specified CAN ID

/* Readings returned determines if string and modules are present. */
static uint8_t yesstring[NSTRING]; // 0 = string number (0-NSTRING) not present
static uint8_t nstring; // String number-1: 0-3
static uint8_t nmodule; // Module number-1: 0-15

uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static uint32_t timerctr;
static uint32_t timernext; // Next timer count

static uint8_t ncell_prev;
static uint8_t headerctr;

// zero duration = one-time only poll.
#define DEFAULT_POLLDUR 1000 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Duration between polls

static uint8_t groupctr; // The six cell readings are sent in a group.

/******************************************************************************
 * static uint8_t storeandcheckstringandmodule(struct CANRCVBUF* p);
 * @brief 	: CAN msg 
 * @param	: p = pointer to can msg
 * @return  : 0 = OK; -1 = string; -2 = module
*******************************************************************************/
/*	      payload[1] U8: Module identification
    	[7:6] 
    	   11 = All modules respond
    	   10 = All modules on identified string respond
    	   01 = Only identified string and module responds
    	   00 = spare; no response expected
    	[5:4] Battery string number (0 – 3) (string #1 - #4)
    	[3:0] Module number (0 – 7) (module #1 - #16)
*/
static uint8_t storeandcheckstringandmodule(struct CANRCVBUF* p)
{
	uint8_t err = 0;
	nstring = ((p->cd.uc[2] >> 4) & 0x3);
	nmodule = (p->cd.uc[2] & 0xf);
	if (nstring > NSTRING)
	{
		printcanmsg(p); // CAN msg
		printf("Out-of-range: string number: %d\n", nstring);
		err = 0x1;
	}
	if (nmodule > NMODULE)
	{
		printcanmsg(p); // CAN msg
		printf("Out-of-range: module number: %d\n", nmodule);
		err |= 0x2;
	}
	yesstring[nstring] = 1;
	return err;
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
 * static printmenu(char* p);
 * @brief 	: Print boilerplate
 * @param	: p = pointer to line
*******************************************************************************/
static char* puc1[] = {
    	   "00 = spare; no response expected",
    	   "01 = Only identified string and module responds",
    	   "10 = All modules on identified string respond",
	   	   "11 = All modules respond" 
	   	};
static void printfsettings(void)
{
	printf("BMS %08X  POLLer %08X who responds 0x%02X %s\n",canid_whoresp,cantx.id,cd_uc1,puc1[(cd_uc1 >> 6)&0x3] );
	printf("ADC rate set: code: %d, ADC rate: %s\n",adcrate, pADCrate[adcrate] );
	printf("Poll duration: %d (ms)\n",polldur);
	return;
}
/*
				"  x = A: Cell ADC raw counts for making calibration\n\t"
				"  x = b: Bits: fet status, opencell wires, installed cells\n\t"
				"  x = h: Help menu\n\t"				
				"  x = s: Status\n\t"
				"  x = t: Temperature calibrated readings (T1, T2, T3)\n\t"
				"  x = T: Temperature ADC raw counts for making calibration (T1, T2, T3)\n\t"
				"  x = v: BMS measured top-of-stack voltage\n\t"
				"  x = V: PCB charger high voltage\n\t"				
				"  x = d: DC-DC converter voltage\n\t"
				"  x = w: Processor ADC calibrated readings\n\t"
				"  x = W: Processor ADC raw counts making calibration\n\t"
*/
static char* psetmenu[] = {
 " 28 SETDCHGTST_ON // Set discharge test with heater fet load: ON\n\t",
 " 29 SETDCHGTST_OFF// Set discharge test with heater fet load: OFF	\n\t",
};

static void printsetmenu(char *p)
{

}

/* Menu for selection of MISCQ codes that poll for readings. 
   The first three chars in the following are MISCQ codes from
   ~/GliderWinchItems/BMS/bmsadbms1818/OurTasks/cancomm_items.h
   slightly revised to put code at beginning of line.

NOTE: Not all of the selections are implemented (08/17/23)   
*/
static char* preadmenu[] = {
 "  1 STATUS       // status\n\t",
 "  2 CELLV_CAL    // cell voltage: calibrated\n\t",
 "  3 CELLV_ADC    // cell voltage: adc counts\n\t",
 "  4 TEMP_CAL     // temperature sensor: calibrated\n\t",
 "  5 TEMP_ADC     // temperature sensor: adc counts for making calibration\n\t",
 "  6 DCDC_V       // isolated dc-dc converter output voltage\n\t",
 "  7 CHGR_V       // charger hv voltage\n\t",
 "  8 HALL_CAL     // Hall sensor: calibrated\n\t",
 "  9 HALL_ADC     // Hall sensor: adc counts for making calibration\n\t",
 " 10 CELLV_HI     // Highest cell voltage\n\t",
 " 11 CELLV_LO     // Lowest cell voltage\n\t",
 " 12 FETBALBITS   // Read FET on/off discharge bits\n\t",
 " 13 DUMP_ON	   // Turn on Dump FET for no more than payload [3]’ secs\n\t",
 " 14 DUMP_OFF	   // Turn off Dump FET\n\t",
 " 15 HEATER_ON    // Enable Heater mode to payload [3] temperature\n\t",
 " 16 HEATER_OFF   // Turn Heater mode off.\n\t",
 " 17 TRICKL_OFF   // Turn trickle charger off for no more than payload [3]’ secs\n\t",
 " 18 TOPOFSTACK   // BMS top-of-stack voltage\n\t",
 " 19 PROC_CAL     // Processor ADC calibrated readings\n\t",
 " 20 PROC_ADC     // Processor ADC raw adc counts for making calibrations\n\t",
 " 21 R_BITS       // Dump, dump2, heater, discharge bits\n\t",
 " 24 CURRENT_CAL  // Below cell #1 minus, current resistor: calibrated\n\t",
 " 25 CURRENT_ADC  // Below cell #1 minus, current resistor: adc counts\n\t",
 "256 END_TABLE\n"
};
/******************************************************************************
 * static int printreadmenu(void);
 * @brief 	: Display menu for MISCQ readings, and get selection
 * @return	: -1 = selection out-of-range, or selection MISCQ code
*******************************************************************************/
static int printreadmenu(void)
{
	int i,j,k;
	int select;
	printf("Poll selected misc readings.\n");
	int code;
	char buf[256];
	// Print menu, find size
	printf("\t");
	for (i = 0; i < 256; i++)
	{
		printf("%s",preadmenu[i]);
		sscanf(preadmenu[i],"%d",&code);
		if (code == 256) // LAST menu code
			break;
	}
	printf("\nEnter code number: n<enter>\n");
	k = read (STDIN_FILENO, buf, 256);	// Read one or more chars from keyboard
	sscanf(buf,"%d",&select);
	printf("select: %d  ct: %d\n",select,i);
	if ((select == 0) || (select > 255))
	{
		printf("Selection %d is not list, abort\n",select);
		return -1;
	}
	// Print menu, find size
	for (j = 0; j < i; j++)
	{
		sscanf(preadmenu[j],"%d",&code);
//printf("%2d %2d %2d %s",j,code,select,preadmenu[j]);
		if (code == select) // Found
		{
			printf("\nMISCQ code: %d: menu line: %s",select,preadmenu[j]);
			return select;
		}
	}
	printf("\nSomething went wrong with this menu lookup mess!\n");
	printf("Selection: %d, no match\n",select);
	return -1;
}

static void printmenu(char* p)
{
	printf("e - BMS => POLLING <== msgs\n\t"
				"emx  Only one BMS responds (using current settings) \n\t"
				"eax  All BMS nodes respond (using current settings) \n\t"
				" where--\n\t"
				"  x = h: Help menu\n\t"				
				"  x = a: Cell readings (CMD_CMD_CELLPOLL format)\n\t"
				"  x = s: Status\n\t"
				"  x = j: Misc read something menu (CMD_CMD_TYPE2 format)\n\t"
				"  x = k: Set something menu (CMD_CMD_TYPE2 format)\n\t"
				"  x = fn: Set FET discharge status bits n: 0 = off; 1-18 = set #, 19 all on\n\t"
				"  x = gn: Set discharge test: n: 0 = OFF, 1 = ON\n\t"
				"ed  <set: Set duration between polls (default: 1000 ms)\n\t"
				"ep  <set: pppppppp> Poller CAN ID (default: B0200000) \n\t"
				"er  <set: r> Set ADC Rate code (0-7, default 0 = 422Hz, 8 = display rate list\n\t"
				"es  <set: s> Set battery String number: 1-n, (default 1)\n\t"
				"ew  <set: w> Set Who responds (BMS ID: aaaaaaaa, 0 = all)\n"
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
	printf("e - BMS polling msgs\n\t"
				"e x  default  (CANID: BMS B0A00000 POLL B0200000)\n\t"
				"emx  aaaaaaaa <pppppppp>(CANID: BMS aaaaaaaa POLL pppppppp) \n\t"
				"eax  <pppppppp> All BMS nodes respond (poll default: B0200000) a \n\t"
				"esx  <s> (Battery string number: 1-n, default 1)\n\t"
				" where--\n\t"
				"  x = a: Cell calibrated readings\n\t"
				"  x = A: Cell ADC raw counts for making calibration\n\t"
				"  x = h: Trickle charger high voltage\n\t"
				"  x = t: Temperature calibrated readings (T1, T2, T3)\n\t"
				"  x = T: Temperature ADC raw counts for making calibration (T1, T2, T3)\n\t"
				"  x = s: BMS measured top-of-stack voltage\n\t"
				"  x = d: DC-DC converter voltage\n\t"
				"  x = f: FET discharge status bits\n\t"
				"  x = w: Processor ADC calibrated readings\n\t"
				"  x = W: Processor ADC raw counts making calibration\n"
				"  x = b: Bits: fet status, opencell wires, installed cells"
				"  x = g: Set discharge test"
				"ed  <duration (ms)> Set duration between polls (Default: 1000 ms)\n"
				);
*/
/*	POLLER msg
          payload[2] U8: Module identification
    	[7:6] 
    	   11 = All modules respond
    	   10 = All modules on identified string respond
    	   01 = Only identified string and module responds
    	   00 = spare; no response expected
    	[5:4] Battery string number (0 – 3) (string #1 - #4)
    	[3:0] Module number (0 – 7) (module #1 - #16) 
 */

int cmd_e_init(char* p)
{
	uint32_t tmp;
	uint32_t len = strlen(p);
	int i;
	int ret;

	printf("%c%c %i\n",*p,*(p+1),len);

	/* This 'e' command */
	cantx.cd.ull   = 0; // Clear all payload bytes
	cantx.id       = candid_poller; // Pollster ID (Default CANID_TX_DEFAULT)
	cantx.dlc      = 8;
	cantx.cd.uc[0] = CMD_CMD_TYPE2; // Payload code: (42) Request BMS responses

	/* Specify BMS nodes that will respond to poll. */
	cantx.cd.uc[2] = cd_uc1; // Default: only specified unit responds, module #1
	if (cd_uc1 == (0x1 << 6)) // Only specified CAN ID responds?
		cantx.cd.ui[1] = canid_whoresp; // BMS CANID (Default )
	else
		cantx.cd.ui[1] = 0; // All, or string nodes, respond.

	if (len < 3)
	{
		printmenu(p);
		printf("Too few chars: %d\n",len);
		return -1;
	}
	else
	{	
		/* POLLER requests BMS node, string, or all. */
/* 2nd char commands that do not have an 'x' return; others 'break' */
		switch ( *(p+1) )
		{ 
		case 'w': // 'ew' Set "Who" responds
			if (len < 12)
			{ // Not enough chars for a CAN ID
				canid_whoresp = 0;
				printf("New: All nodes respond: %d\n",canid_whoresp );
				printfsettings();
				cd_uc1 = (0x3 << 6); // Save for re-entry init
				cantx.cd.uc[1] = cd_uc1; // All BMSs respond
				cantx.cd.ui[1] = 0; // CAN ID payload NULL
				return 0;
			}
			sscanf((p+2),"%x",&canid_whoresp);
			printf("New: BMS CAN ID: 0x%08X only responds\n",canid_whoresp);
			printfsettings();
			cd_uc1 = (0x1 << 6); // Save for re-entry init
			cantx.cd.uc[1] = cd_uc1; // Specified BMS responds
			cantx.cd.ui[1] = canid_whoresp; // CAN ID of BMS
			printfsettings();
			return 0;

		case 'r': // 'er' Set ADC "rate" code (default = 0: 422 Hz)
			if (len < 3)
			{
				adcrate = 0;
			}
			sscanf((p+2),"%d",&adcrate);
			if (adcrate > 7)
			{
				printf("OOPs! ADC rate code 0-7 and I got %d\n",adcrate );
				for (i = 0; i < 8; i++)
				{
					printf("\t%d: %s\n",i,pADCrate[i] );
				}
				adcrate = 0; // 7KHz (normal) mode
				return 0;
			}
			printf("New ADC rate set: code: %d, ADC rate: %s\n",adcrate, pADCrate[adcrate] );
			printfsettings();
				return 0;

		case 'p': // 'ep' Set POLL'er CAN ID
			if (len < 12)
			{
				printf ("Set POLLer CAN ID: input too short: %d\n", len);
				printf ("Example: ep B0200000\n");
				return -1;
			}
			sscanf( (p+2), "%x",&cantx.id);
			printf("New POLLer CAN ID set to: 0x%08X\n",cantx.id);
			candid_poller = cantx.id;
			printfsettings();
			return 0;

		case 'd': // 'ed' Set "Duration" between poll msgs (ms)
			if (len < 5)			
			{
				printf("\nNot enough input chars to set poll duration\n");
				return -1;
			}
			sscanf( (p+3), "%d",&polldur);
			printf("\nPoll duration: %d\n", polldur);
			if (polldur < 20)
			{
				printf("Less than 20 (ms) is too short. ");
			}
			if (polldur > 4000)
			{
				printf("Over 4000 (ms) may be longer than BMS heartbeats! ");
			}
			printf("New value set is: %d (ms)\n",polldur);
			printfsettings();
			return 0;

		case 'h': // 'eh' Help
			printmenu(p);
			return 0;					

		case 'm': // 'emx' Set single BMS response for 'x' command
			cantx.cd.uc[1] = (0x1 << 6); // Use CAN ID in [4]-[7]
			cd_uc1 = (0x1 << 6); // Save for command re-entry
			cantx.cd.ui[1] = canid_whoresp;
			break;

		case 's': // 'esx' String response
			if (len > 13)
			{
				sscanf( (p+3), "%d %x",&tmp,&canid_tx);
				cantx.id = canid_tx;
				candid_poller = canid_tx; // Save for command re-enty
			}
			// Strings respond | String number
			cantx.cd.uc[1] = (0x2 << 6) | ((tmp & 3) << 4); 
			cantx.cd.ui[1] = 0; // NULL target CANID
			cantx.cd.uc[2] = (adcrate << 4) | (groupctr << 0);
			break;

		case 'a': // 'eax' All BMS nodes
			cantx.cd.ui[1] = 0; // NULL target CANID	
			cantx.cd.uc[1] = (0x1 << 6); // Use CAN ID in [4]-[7]
			cd_uc1 = (0x3 << 6); // Save for command re-entry
			printf("eaa: All BMS nodes\n");
			break;

		default:
			printf("2nd char not recognized: %c\n", *(p+1));
			canid_rx = 0;
			return -1;
		}
		printf ("\tCANID: BMS %08X  POLLER %08X\n",canid_rx,cantx.id);

		/* All but, 'x' == 'a' are CMD_CMD_TYPE2. */
		cantx.cd.uc[0] = CMD_CMD_TYPE2; //  TYPE2 payload format 
		cantx.cd.uc[1] = cd_uc1; // Poll: All, string, single

		reqtype = *(p+2); // Save 'x' request code
		switch (reqtype)
		{
		case 'a': // Cell readings are CMD_CMD_CELLPOLL commands
			printf("Poll: Cells: calibrated readings\n");
			cantx.cd.uc[0] = CMD_CMD_CELLPOLL; // (42) Request BMS responses
			cantx.cd.uc[1] = cd_uc1; // Poll: All, string, single
			cantx.cd.uc[2] = (adcrate << 4) | (groupctr << 0);
			break;

		case 'j': // Misc read something menu (CMD_CMD_TYPE2 format)\n\t"
			ret = printreadmenu();
			if (ret < 0)
				return -1; // Failed, or aborted
			cantx.cd.uc[2] = ret; //  [1] TYPE2 sub-code	
			break;

		case 's': // Poll for status msgs
			cantx.cd.uc[2] = MISCQ_STATUS; //  [1] TYPE2 sub-code			
			printf("Poll: MISCQ_STATUS status code %d\n",cantx.cd.uc[2]);
			break;				

		case 'g': // x = g: Set discharge test
			cantx.cd.uc[2] = MISCQ_SETDCHGTST_ON; // Sub-code for BMS
			polldur = 0; // One-Time-Only poll
			break;			

		case 'm': // x = m: Set mode
//			cantx.cd.uc[2] = MISCQ_SETDCHGTST; // Sub-code for BMS
//			cantx.cd.uc[4] = 0; // Set it OFF
//			polldur = 0; // One-Time-Only poll
			break;			


		default:
			printf("3rd char not recognized: %c\n", *(p+2));
			return -1;
		}
	}
	printf("Poll duration (ms) is: %d\n",polldur);
	ncell_prev =   0;
	headerctr  = HEADERMAX;
	kaON       =   1;
	timerctr   = 0;
	timernext  = polldur/10;

	sendcanmsg(&cantx);	// Send first request
	if (polldur != 0)
		starttimer(); // Start timer for repetitive requests
	return 0;
}

/******************************************************************************
 * void cmd_e_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_e_do_msg(struct CANRCVBUF* p)
{
//		int i;
		uint8_t err;
	/* Expect the BMS node CAN msg format TYPE2, etc
	     and skip other CAN IDs.
	   These #defines originate from the processing of the .sql file
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CMD_CODES_INSERT.sql
	   which generates the file
	     ../../../../../svn_common/trunk/db/gen_db.h */
//printf("\n%08X %d: ", p->id,p->dlc);for (i=0; i<p->dlc; i++) printf(" %02X",p->cd.uc[i]);
	uint32_t utmp = (p->id & 0xfffffffc);
	if ((utmp < (uint32_t)CANID_UNIT_BMS01) || (utmp > (uint32_t)CANID_UNIT_BMS18))
		return; // CAN ID is not a BMS module function.

	/* Here, CAN msg is from a BMS module. */
	/* Ignore msgs that are not the type requested. */	
	if ( !((p->cd.uc[0] == CMD_CMD_TYPE2) || (p->cd.uc[0] == CMD_CMD_CELLPOLL)) )  
	{ // Here, not what we are looking for.
		return;
	}

	// Extract string and module numbers and 
	// check for out-of-range string and module.
	err = storeandcheckstringandmodule(p);
	if (err != 0)
	{
		printf("\nerr return: storeandcheckstringandmodule %02X",err);
		printcanmsg(p);
		return;
	}

//* debug
printcanmsg(p);
printf(" :%1d:%1d:",nstring,nmodule);
int jdx = p->cd.uc[3];
float ftmp = extractfloat(&p->cd.uc[4]);	
printf("%2d %f\n",jdx,ftmp);
//*/

	// Store payload based on request code
	switch (p->cd.uc[1])
	{
	case MISCQ_CELLV_CAL: // 'a' Cell voltage: calibrated
		miscq_cellv_cal(p);
		break;

	case MISCQ_CELLV_ADC: // 'A' Cell ADC readings
		miscq_cellv_adc(p);
		break;

	case MISCQ_TEMP_CAL: // 't' Temperature calibrated readings
		miscq_temp_cal(p);
		break;

	case MISCQ_TEMP_ADC: // 'T' Temperature ADC for calibration
		miscq_temp_adc(p);
		break;

	case MISCQ_TOPOFSTACK: // 's' BMS measured top-of-stack voltage 	
		miscq_topofstack(p);
		break;

	case MISCQ_DCDC_V: // 'd' DC-DC converter voltage
		miscq_miscq_dcdc_v(p);
		break;

	case MISCQ_FETBALBITS: // 'f' discharge FET status bits
		miscq_fetbalbits(p);
		break;

	case MISCQ_PROC_CAL: // 'w' Processor ADC calibrated readings
		miscq_proc_cal(p);
		break;

	case MISCQ_PROC_ADC: // 'W' Processor ADC raw adc counts for making calibrations
		miscq_proc_adc(p);	
		break;

//	case MISCQ_R_BITS: // 'r' Bits: fets, cells open wire, cells installed	
//		miscq_bits_r(p);	
//		break;		

	default:
		printcanmsg(p); // CAN msg
		printf("TYPE2 command code not in switch statement: %d\n", p->cd.uc[1]);
		break;
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
	struct PCTOGATEWAY pctogateway; 
//	pcan->id = canid_tx;
//	pcan->dlc = 1;

	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
//printf("TX: %08x %d %02X\n",pcan->id, pcan->dlc, pcan->cd.u8[0]);
	return;
}
/******************************************************************************
 * static void cmd_e_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_e_timerthread(void)
{
	if (polldur == 0)
		return;

	if (kaON == 0) return; // No timer generated msgs


	timerctr += 1; // 10 ms tick counter
	if ((int)(timerctr - timernext) > 0)
	{ // Time to output accumulated readings
		timernext += polldur/10;
//		timer_printresults();

		// Cell group 4 bit counter 
		if (cantx.cd.uc[0] == CMD_CMD_CELLPOLL)
			cantx.cd.uc[2] = (groupctr & 0x0f);

		sendcanmsg(&cantx);	// Send next request
		groupctr += 1;
		groupctr &= 0xff;
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
	int ret = timer_thread_init(&cmd_e_timerthread, 10000); // 10 ms
	if (ret != 0)
	{
		printf("\ncmd_e: timer thread startup unsucessful\n");
		return -1;
	}
	return 0;
}
/* *************************************************************************
 * static float extractfloat(uint8_t* puc);
 *	@brief	: Extract payload bytes into a float
 *  @param  : puc = pointer to CAN msg payload
 *  @return : float
 * *************************************************************************/
static float extractfloat(uint8_t* puc)
{
	union UF uf;
	uf.uc[0] = *(puc+0);
	uf.uc[1] = *(puc+1);
	uf.uc[2] = *(puc+2);
	uf.uc[3] = *(puc+3);
	return uf.f;
}
/* *************************************************************************
 * static uint32_t extractu32(uint8_t* puc);
 *	@brief	: Extract payload bytes into a uint32_t
 *  @param  : puc = pointer to CAN msg payload
 *  @return : float
 * *************************************************************************/
static uint32_t extractu32(uint8_t* puc)
{
	return ((*(puc+0) <<  0) |
		    (*(puc+1) <<  8) |
		    (*(puc+2) << 16) |
		    (*(puc+3) << 24) );
}
/******************************************************************************
 * static void miscq_cellv_adc(struct CANRCVBUF* p);
 * @brief 	: Request: Cell ADC readings
 * @param	: p = pointer to CANRCVBUF with CAN msg
*******************************************************************************/
/* 
The response CAN msgs are one cell per CAN msg (i.e. 16 msgs per module). 
The ADC count is sent as a float.
*/
static void miscq_cellv_adc(struct CANRCVBUF* p)
{
		int idx = p->cd.uc[3];
		if (idx >= NCELL)
		{ // Here, index is too high.
			printcanmsg(p); // CAN msg
			printf("Out-of-range: Cell index too high: %d\n",p->cd.uc[3]);
		}
		else
		{
			cellmsg[idx].d = extractfloat(&p->cd.uc[4]);
			cellmsg[idx].flag = 1; // Reset new readings flag
		}
	return;		
}
/******************************************************************************
 * static void miscq_cellv_cal(struct CANRCVBUF* pcan);
 * @brief 	: Request: Cell calibrated voltages
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_cellv_cal(struct CANRCVBUF* p)
{
		int idx = p->cd.uc[3];
		if (idx >= NCELL)
		{ // Here, index is too high.
			printcanmsg(p);
			printf("Cell index too high: %d\n",p->cd.uc[3]);
			return;
		}
		else
		{
			// Convert payload to float and scale to volts
			cellmsg[idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[idx].flag = 1; // Reset new readings flag		
		}
		return;
}
/******************************************************************************
 * static void miscq_temp_cal(struct CANRCVBUF* pcan);
 * @brief 	: Request: Thermistor temperatures, calibrated
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_temp_cal(struct CANRCVBUF* p)
{
	int idx = (p->cd.uc[3]);
		if ((idx > 2) || (idx < 0))
		{ // Here, index is too high.
			printcanmsg(p);
			printf("T1-T3 index should be [0-2] but was too high or low: %d\n",p->cd.uc[3] - 16);
			return;
		}
		else
		{  
			// Convert payload to float and scale to deg F
			cellmsg[idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[idx].flag = 1; // Reset new readings flag
		}
		return;
}
/******************************************************************************
 * static void miscq_temp_adc(struct CANRCVBUF* p);
 * @brief 	: Request: Thermistor temperatures, ADC readings
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
/*
Calibrated voltages are sent as 
*/
static void miscq_temp_adc(struct CANRCVBUF* p)
{
	int idx = (p->cd.uc[3]);
		if ((idx > 2) || (idx < 0))
		{ // Here,  is not 0-2
			printcanmsg(p);
			printf("T1-T3 index should be [0-2] but was too high or low: %d\n",idx);
			return;
		}
		else
		{  
			// Convert payload to float and scale to deg F
			cellmsg[idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[idx].flag = 1; // Reset new readings flag
		}
		return;
}		
/******************************************************************************
 * static void miscq_topofstack(struct CANRCVBUF* p);
 * @brief 	: Request: BMS top-of-stack measurement
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_topofstack(struct CANRCVBUF* p)
{
	// Convert payload to float and scale to deg F
	cellmsg[0].d = (extractfloat(&p->cd.uc[4]));
	cellmsg[0].flag = 1; // Reset new readings flag
	return;
}
/******************************************************************************
 * static void miscq_miscq_dcdc_v(struct CANRCVBUF* p);
 * @brief 	: Request: Isolated DC-DC converter voltager
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_miscq_dcdc_v(struct CANRCVBUF* p)
{
	// Convert payload to float  and scale to deg F
	cellmsg[0].d = (extractfloat(&p->cd.uc[4]));
	cellmsg[0].flag = 1; // Show new reading is present
	return;
}
/******************************************************************************
 * static void miscq_fetbalbits(struct CANRCVBUF* p);
 * @brief 	: Request: Discharge FET status
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_fetbalbits (struct CANRCVBUF* p)
{
	uint32_t tmp = extractu32(&p->cd.uc[4]);
	int i;
	for (i = 0; i < NCELL; i++)
	{
		if ((tmp & 0x1) == 0)
		{
			cellmsg[i].flag = 0;
		}
		else
		{
			cellmsg[i].flag = 2;
		}
		tmp = (tmp >> 1);
	}
	return;
}
	/******************************************************************************
 * static void miscq_proc_cal(struct CANRCVBUF* p);
 * @brief 	: Request: Processor ADC readings, calibrated
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_proc_cal(struct CANRCVBUF* p)
{
	int idx = (p->cd.uc[3]);
		if (idx > 9)
		{ // Here,  is not 0-9
			printcanmsg(p);
			printf("Processor ADC array index should be [0-9] but was too high: %d\n",idx);
			return;
		}
		else
		{  
			// Convert payload to double
			cellmsg[idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[idx].flag = 1; // Reset new readings flag
		}
		return;
}
/******************************************************************************
 * static void miscq_proc_adc(struct CANRCVBUF* p);
 * @brief 	: Request: Processor ADC readings: raw adc counts
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_proc_adc(struct CANRCVBUF* p)
{
	int idx = (p->cd.uc[3]);
		if (idx > 9)
		{ // Here,  is not 0-9
			printcanmsg(p);
			printf("Processor ADC array index should be [0-9] but was too high: %d\n",idx);
			return;
		}
		else
		{  
			// Convert payload to double
			cellmsg[idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[idx].flag = 1; // Reset new readings flag
		}
		return;
}
/******************************************************************************
 * static void miscq_bits_r(struct CANRCVBUF* p);
 * @brief 	: Request: bits for fets, open wire, installed cells
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
#if 0
static void miscq_bits_r(struct CANRCVBUF* p)
{
	int idx = (p->cd.uc[3]);
		if (idx > 2)
		{ // Here,  is not 0-9
			printcanmsg(p);
			printf("Index too high. Should be 0-2, was %d\n",idx);
			return;
		}
		else
		{  
			cellmsg[idx].u32 = (extractu32(&p->cd.uc[4]));
			cellmsg[idx].flag = 1; // Reset new readings flag
		}
		return;
}
#endif
/******************************************************************************
 * static void timer_printresults(void);
 * @brief 	: Timer triggers output accumulated readings
*******************************************************************************/
#if 0
static void timer_printresults(void)
{
	if (cantx.cd.uc[0] == CMD_CMD_TYPE2)
		return;

		switch (reqtype)
		{
		case 'a': // Cell calibrated readings
			print_cal_adc("%8.4f",NCELL);
			break;

		case 'A': // Cell ADC accumlation for calibation
			print_cal_adc("%8.1f",NCELL);
			break;

		case 't': // Temperature calibrated readings
			print_cal_adc("%8.2f",NTHERM);
			break;

		case 'T': // Temperature ADC for calibration
			print_cal_adc("%8.1f",NTHERM);
			break;

		case 's': // BMS measured top-of-stack voltage MISCQ_TOPOFSTACK	
			print_cal_adc("%8.2f",1);
 			break;		

		case 'd': // Isolated dc-dc converter voltage
			print_cal_adc("%8.2f",1);
 			break;	

		case 'f': // discharge FET status bits
			print_cal_adc("%8.0f",NCELL);
			break;

 		case 'w': // Processor ADC calibrated readings
 			print_processor_adc_header();
			print_cal_adc("%8.3f",ADCDIRECTMAX);
			break;

		case 'W': // Processor ADC raw adc counts for making calibrations
 			print_processor_adc_header();
			print_cal_adc("%8.0f",ADCDIRECTMAX);
			break;

		case 'r': // Processor ADC raw adc counts for making calibrations
			print_bits_r();
			break;			

		default:
			printf("Timer oops: not coded: %c\n",reqtype);
			break;
		}
	return;
}
#endif
/******************************************************************************
 * static void print_cal_adc(char* pfmt, uint8_t ncol);
 * @brief 	: Output accumulated readings
 * @param   : pfmt = pointer to format string
 * @param   : ncol = number of columns (readings) on a line
*******************************************************************************/
#if 0
static void print_cal_adc(char* pfmt, uint8_t ncol)
{
	int i,j,k; // FORTRAN reminder

	for (i = 0; i < NSTRING; i++)
	{
		// Skip if this (battery) string not encountered
		if (yesstring[i] == 0 ) continue;

		/* Column header. */
		headerctr += 1;
		if (headerctr > HEADERMAX)
		{
			headerctr = 0;
			printf("    ");
			for (j = 0; j < ncol; j++)
				printf("%8d",j+1);
			printf("\n");
		}

		for (j = 0; j < NMODULE; j++)
		{
			// Start line with string and module number
			printf("%1d:%1d:",i,j);
			for (k = 0; k < ncol; k++)
			{
				if (cellmsg[i][j][k].flag == 0)							
				{ // No cell reading
					printf(" .......");
				}
				else
				{   if (cellmsg[i][j][k].flag == 1)
					{
						printf(pfmt, cellmsg[i][j][k].d);
 					}
 					else
 					{
 						printf(" ...####");
 					}
 					cellmsg[i][j][k].flag = 0; // Reset new readings flag
				}
			}
			printf("\n");	
		}
	}
	return;
}
#endif
/******************************************************************************
 * static void print_processor_adc_header(void);
 * @brief 	: Insert descriptive header for processor ADC
*******************************************************************************/
/*
#define ADC1IDX_INTERNALVREF  0	// IN0   247.5  1   Internal voltage reference
#define ADC1IDX_INTERNALTEMP  1	// IN17  247.5  2   Internal temperature sensor
#define ADC1IDX_PA4_DC_DC     2 // IN9   247.5  3   Isolated DC-DC 15v supply
#define ADC1IDX_PA7_HV_DIV    3	// IN12  640.5  4   HV divider (FET side of diode)
#define ADC1IDX_PC1_BAT_CUR   4	// IN2    47.5  5   Battery current sense op-amp
#define ADC1IDX_PC4_THERMSP1  5	// IN13  247.5  6   Spare thermistor 1: 10K pullup
#define ADC1IDX_PC5_THERMSP2  6	// IN14  247.5  7   Spare thermistor 2: 10K pullup
#define ADC1IDX_PC3_OPA_OUT   7	// IN4     2.5  8   FET current sense 0.1 ohm:COMP2_INP
#define ADC1IDX_PA0_OPA_INP	  8 // IN5    24.5  9   FET current sense RC 1.8K|0.1u
#define ADC1IDX_PA3_FET_CUR1  9 // IN8    12.5  10  OPA_OUT (PA0 amplified) 
*/
#if 0
static void print_processor_adc_header(void)
{		
	if (headerctr >= HEADERMAX)
	{
		printf ("    "
			    "    VREF"
				" INTTEMP"
				"   DC-DC"
				"  HV DIV"
				"  CUR OP"
				"  SPARE1"
				"  SPARE2"
				" FET CUR"
				"  FET RC"
				" OPA_OUT\n"
			);
	}
	return;
}
#endif
/******************************************************************************
 * static void print_cal_adc(char* pfmt, uint8_t ncol);
 * @brief 	: Output accumulated readings
 * @param   : pfmt = pointer to format string
 * @param   : ncol = number of columns (readings) on a line
*******************************************************************************/
#if 0
static void print_bits_r(void)
{
	int i,j,k,m; // FORTRAN reminder
	uint32_t u32tmp;

	for (i = 0; i < NSTRING; i++)
	{
		// Skip if this (battery) string not encountered
		if (yesstring[i] == 0 ) continue;

		/* Column header. */
		headerctr += 1;
		if (headerctr > HEADERMAX)
		{
			headerctr = 0;

			printf("\t\t "
				"\t   Discharge FET "
				"\t      open wires"
				"\t       installed"
				"\n                ");
			for (k = 0; k < 3; k++)
			{
				printf("      ");
				for (m = 1; m < NCELL+1; m++)
				{
					if (m < 10)
						printf(" ");
					else
						printf("%1d",(m/10));
				}
			}
			printf("\n                ");
			for (k = 0; k < 3; k++)
			{
				printf("      ");
				for (m = 1; m < NCELL+1; m++)
					printf("%1d",(m%10));
			}
			printf("\n");
		}

		/* Print bit order: (0->7) or (0->17) */
		for (j = 0; j < NMODULE; j++)
		{
			if (yesmodule[i][j] == 0) continue;

			// Start line with string and module number
			printf("%1d:%1d:",i,j);
			for (k = 0; k < 3; k++)
			{
				printf("      ");
				if (cellmsg[i][j][k].flag == 1)
				{
					/* fet status: Dump, Dump2, Heater, etc. */
					if (k == 0)
					{ // High byte has special fets
						u32tmp = (cellmsg[i][j][k].u32 >> 24);
						for (m = 0; m < 8; m++)
						{
							if ((u32tmp & 1) == 0)
							{
								printf(".");
							}	
							else	
							{	
								printf("#");
							}	
							u32tmp >>= 1;
						}
						printf("    ");
					}
					u32tmp = cellmsg[i][j][k].u32;
					for (m = 0; m < NCELL; m++)
					{
						if ((u32tmp & 1) == 0)
						{
							printf(".");
						}	
						else	
						{	
							printf("#");
						}	
						u32tmp >>= 1;
					}
					cellmsg[i][j][k].flag = 0; // Reset new readings flag
				}
				else
				{ // CAN msg missing, so fill in the space
					printf("missing     ---missing--------");
				}
			}
			printf("\n");	
		}
	}
	return;
}
		case 'A': // Cell ADC raw adc counts for making calibrations
			printf("Poll: Cells: ADC Readings\n");
			cantx.cd.uc[2] = MISCQ_CELLV_ADC; //  TYPE2 code
			break;

		case 't': // Temperature calibrated readings
			printf("Poll: calibrated temperatures\n");
			cantx.cd.uc[2] = MISCQ_TEMP_CAL; //  TYPE2 code			
			break;

		case 'T': // Temperature raw adc counts for making calibrations
			printf("Poll: thermistor ADC readings\n");
			cantx.cd.uc[2] = MISCQ_TEMP_ADC; //  TYPE2 code			
			break;

		case 'v': // BMS measured top-of-stack voltage MISCQ_TOPOFSTACK	
			printf("Poll: BMS measured top-of-stack voltage\n");
			cantx.cd.uc[2] = MISCQ_TOPOFSTACK;  //  TYPE2 code
			break;

		case 'd': // Isolated dc-dc converter output voltage
			printf("Poll: Isolated dc-dc converter output voltage\n");
			cantx.cd.uc[2] = MISCQ_DCDC_V;  //  TYPE2 code
			break;

		case 'f': // FET status for discharge
			printf("Poll: discharge FET status bits\n");
			cantx.cd.uc[2] = MISCQ_FETBALBITS;  //  TYPE2 code
			break;		

		case 'w': // Processor ADC calibrated readings
			printf("Poll: Processor ADC calibrated readings\n");
			cantx.cd.uc[2] = MISCQ_PROC_CAL;  //  TYPE2 code
			break;

		case 'W': // Processor ADC raw adc counts for making calibrations
			printf("Poll: Processor ADC raw counts for making calibrations\n");
			cantx.cd.uc[2] = MISCQ_PROC_ADC;  //  TYPE2 code
			break;	

		case 'r': // Bits: fets, open cell wires, installed cells
			printf("Poll: Bits: fet status, disicharge fets, open cell wires, installed cells\n");
			cantx.cd.uc[2] = MISCQ_R_BITS;  //  TYPE2 code
			break;

		case 's': // Poll for status msgs
			cantx.cd.uc[2] = MISCQ_STATUS; //  [1] TYPE2 sub-code			
			printf("Poll: MISCQ_STATUS status code %d\n",cantx.cd.uc[2]);
			break;				


#endif