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
//static void printcanmsg(struct CANRCVBUF* p);
#if 0
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
#endif
static void cmd_e_timerthread(void);
static int starttimer(void);
/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds; */
#define CANID_RX_DEFAULT CANID_UNIT_BMS03 // B0A00000','UNIT_BMS03', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #01
#define CANID_TX_DEFAULT CANID_UNI_BMS_PC_I //CANID_UNI_BMS_PC_I        // AEC00000 //BMSV1 UNIversal From PC,  Incoming msg to BMS: X4=target CANID // Default pollster

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
//static struct CELLMSG cellmsg[NCELL];
static struct CANRCVBUF cantx;
static uint32_t canid_rx;
static uint32_t canid_tx; // POLLer CAN ID (is one who "polls")

static uint32_t canid_whoresp = CANID_RX_DEFAULT; // Who responds?
static uint32_t candid_poller = CANID_TX_DEFAULT; // Default pollster

static uint8_t reqtype; // Request type (miscq code)
static uint8_t canseqnumber;
static uint8_t cd_uc1 = (0x1 << 6); // Who responds: canid_tx.cd.uc[1] default: only specified CAN ID

/* Readings returned determines if string and modules are present. */
//static uint8_t yesstring[NSTRING]; // 0 = string number (0-NSTRING) not present
//static uint8_t nstring; // String number-1: 0-3
//static uint8_t nmodule; // Module number-1: 0-15

uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static uint32_t timerctr;
static uint32_t timernext; // Next timer count

static uint8_t ncell_prev;
static uint8_t headerctr;

// zero duration = one-time only poll.
#define DEFAULT_POLLDUR 1000 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Duration between polls

static uint8_t groupctr; // The six cell readings are sent in a group.

static uint8_t walkfets_ctr;
static uint8_t walkfets_sw;

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
/* Menu for selection of MISCQ codes that poll for readings. 
   The first three chars in the following are MISCQ codes from
   ~/GliderWinchItems/BMS/bmsadbms1818/OurTasks/cancomm_items.h
   slightly revised to put code at beginning of line.

NOTE: Not all of the selections are implemented (08/17/23) 
 " 17 TRICKL_OFF   // Turn trickle charger off for no more than payload [3]’ secs\n\t",

*/
static char* preadmenu[] = {
 "  1 STATUS       // status\n\t",
 "  2 CELLV_CAL    // cell voltage: NOTE: poll cell readings use eaa command\n\t",
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
 " 18 TOPOFSTACK   // BMS top-of-stack voltage\n\t",
 " 19 PROC_CAL     // Processor ADC calibrated readings\n\t",
 " 20 PROC_ADC     // Processor ADC raw adc counts for making calibrations\n\t",
 " 21 R_BITS       // Dump, dump2, heater, discharge bits\n\t",
 " 24 CURRENT_CAL  // Below cell #1 minus, current resistor: calibrated\n\t",
 " 25 CURRENT_ADC  // Below cell #1 minus, current resistor: adc counts\n\t",
 " 32 PRM_MAXCHG   // Get Parameter: Max charging current\n\t",
 " 34 READ_AUX     // BMS responds with A,B,C,D AUX register readings (12 msgs)\n\t",
 " 36 PROC_TEMP    // Processor calibrated internal temperature (deg C)\n\t",
 " 37 MISCQ_CHG_LIMITS // Show params: Module V max, Ext chg current max, Ext. chg bal\n",
 "256 END_TABLE\n"
};
/* Menu for MISCQ codes that set something in the BMS. */
static char* psetmenu[] = {
 " 13 SET_DUMP	    // Turn on Dump FET for no more than payload [3]’ secs\n\t",
 " 14 SET_DUMP2     // Set DUMP2 FET FET: on|off\n\t",
 " 15 SET_HEATER    // Enable Heater mode to payload [3] temperature\n\t",
 " 28 SET_DCHGTST   // Set discharge test with heater fet load: ON|OFF\n\t",
 " 30 SET_DCHGFETS  // Set discharge FETs: all on or off, or single\n\t",
 " 31 SET_SELFDCHG  // Set ON|OFF self-discharge mode\n\t",
 " 33 SET_ZEROCUR   // 1 = Zero external current in effect; 0 = maybe not.\n",
 "256 END_TABLE\n" 
};
/******************************************************************************
 * static int get01m9(char* pset);
 * @brief 	: Keyboard input 0, 1, or -9 to abort
 * @param   : pset = pointer to associated string
 * @return  : 0, 1, -9
*******************************************************************************/
static int get01m9(char *pset)
{
	char buf[256];
	int k;
	int subcode;

	printf(" ON|OFF: Enter: 0 = OFF, 1 = ON, -9 = abort\n");
	k = read (STDIN_FILENO, buf, 256);	// Read chars from keyboard
	if (k > 1)
	{ // Get code entered
		sscanf(buf,"%d",&subcode);
		printf("select:%s: set code %d\n",pset,subcode);
	}
	else 
		subcode = 0;// No code entered assumes all off.	
	if ((subcode == 0) || (subcode == 1) || (subcode == -9))
	{
		cantx.cd.uc[3] = subcode;
		return subcode;
	}
	printf("Oops! Entry was %d and not 0, 1, or -9\n",subcode);
	return subcode;
}	
/******************************************************************************
 * static int printsetmenu2(int j);
 * @brief 	: Display menu for MISCQ settings, and get selection
 * @return	: -1 = selection out-of-range, or selection MISCQ code
*******************************************************************************/
static int printsetmenu2(int j,int code)
{
	char buf[256];
	int subcode;
	int k;
	char* pset = psetmenu[j];
	walkfets_sw = 0;
	switch(code)
	{
	case MISCQ_SET_DCHGFETS: // Set discharge FET bits
/* MISCQ_SET_DCHGFETS Sub code for sending request. 
Requester payload[3] 
  0 = All FETs off
  1-18 = FET number to turn ON.
  111  = All FETs on
*/  	
		while(1==1) // Loop until Op gets something right
		{
			printf("SET DISCHARGE FETs, Enter:\n\t"
				"0 = all OFF,  111 = all ON, 1-18 = individual cell, 99 = walk cells, -9 = abort\n");
			k = read (STDIN_FILENO, buf, 256);	// Read chars from keyboard
			if (k > 1)
			{ // Get code entered
				sscanf(buf,"%d",&subcode);
				printf("select:%s: set code %d\n",pset,subcode);
			}
			else 
				subcode = 0;// No code entered assumes all off.

			if (subcode == 0)
			{// All discharge FETs off
				cantx.cd.uc[3] = 0; // SET_FETBITS SUBCODE: ALL Discharge FETS OFF
				printf("Set All FETs OFF\n");
				break;
			}
			if ((subcode > 0) && (subcode < 19))
			{// Here, individual discharge FET cell code is to be set
				cantx.cd.uc[3] =  subcode; // SET_FETBITS CODE: Set single FET ON
				printf("Set FET #%d ON\n",subcode);
				break;
			}
			if (subcode == 111)
			{ // Set all FETs ON
				cantx.cd.uc[3] = subcode; // SET_FETBITS CODE: ALL Discharge FETS ON
				printf("Set ALL FETs ON\n");
				break;
			}
			if (subcode == 99)	
			{ // Here, code for command 'e' to "walk" the discharge FETs
				walkfets_sw = 1;
				walkfets_ctr = 1;
				cantx.cd.uc[3] = 1; // Start with cell #1
				printf("WALK FETs\n");
				subcode = 1; // ret 1 instead of 99
				break;
			}
			if (subcode == -9)
			{
				printf("Abort this subcommand\n");
				break;
			}
			printf("FET entry not not recognized\n");
		}
		break;

	case MISCQ_SET_DUMP: // 13 DUMP FET on/off
			printf("SET DUMP FET");
			subcode = get01m9(pset);
		break;

	case MISCQ_SET_DUMP2: // 14 DUMP FET on/off
			printf("SET DUMP2 FET");
			subcode = get01m9(pset);
		break;

	case MISCQ_SET_HEATER: // 15 Heater FET on/off
			printf("SET HEATER FET");
			subcode = get01m9(pset);
			break;

	case MISCQ_SET_DCHGTST: // Discharge test on/off	
			printf("SET DISCHARGE TEST MODE");
			subcode = get01m9(pset);
polldur = 0; // One shot			
		break;

	case MISCQ_SET_SELFDCHG: // Set self discharge mode
		printf("SET SELF-DISCHARGE MODE");
polldur = 0; // One shot		
			subcode = get01m9(pset);
			break;

	case MISCQ_PRM_MAXCHG: // 32 Get Parameter: Max charging current
			printf("GET PARAMETER: MAXIMUM CHARGE RATE");
polldur = 0; // One shot	
			subcode = get01m9(pset);
			break;

	default:
			printf("menu2 error: j: %d code: %d\n",j,code);
	}
	return subcode;
}
/******************************************************************************
 * static int printsetmenu(void);
 * @brief 	: Display menu for MISCQ settings, and get selection
 * @return	: -1 = selection out-of-range, or selection MISCQ code
*******************************************************************************/
static int printsetmenu(void)
{
	int i,j;
	int select;
	printf("Poll selected misc SETtings.\n");
	int code;
	char buf[256];
	int ret;
	// Print menu, find size
	printf("\t");
	for (i = 0; i < 256; i++)
	{
		printf("%s",psetmenu[i]);
		sscanf(psetmenu[i],"%d",&code);
		if (code == 256) // LAST menu code
			break;
	}
	printf("\nEnter code number: n<enter>\n");
	read (STDIN_FILENO, buf, 256);	// Read one or more chars from keyboard
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
		sscanf(psetmenu[j],"%d",&code);
//printf("%2d %2d %2d %s",j,code,select,preadmenu[j]);
		if (code == select) // Found
		{
			printf("\nMISCQ code: %d: menu line: %s",select,psetmenu[j]);
			ret = printsetmenu2(j,code);
			if (ret >= 0)
			{
				cantx.cd.uc[2] = select;
				cantx.cd.uc[3] = ret;
				return ret;
			}
			if (ret == -9)
				return -1;
		}
	}
	printf("\nSomething went wrong with this menu lookup mess!\n");
	printf("Selection: %d, no match\n",select);
	return -1;
}
/******************************************************************************
 * static int printreadmenu(void);
 * @brief 	: Display menu for MISCQ readings, and get selection
 * @return	: -1 = selection out-of-range, or selection MISCQ code
*******************************************************************************/
static int printreadmenu(void)
{
	int i,j,k;
	int select;
	printf("Poll selected misc READings.\n");
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
	if (k < 1)
	{
		printf("no code number entered\n");
		return -1;
	}
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
			cantx.cd.uc[2] = code;
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
				"ep  <set: pppppppp> Poller CAN ID (AEC00000(default),B0000000(EMC1),B0200000(EMC2)) \n\t"
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
				"e x  default  (CANID: BMS B0A00000 POLL AEC00000)\n\t"
				"emx  aaaaaaaa <pppppppp>(CANID: BMS aaaaaaaa POLL pppppppp) \n\t"
				"eax  <pppppppp> All BMS nodes respond (poll default: AEC00000) a \n\t"
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

//	printf("Input replay: %c%c %i\n",*p,*(p+1),len);

	cantx.cd.ull   = 0; // Clear all payload bytes
	cantx.id       = candid_poller; // Pollster ID (Default CANID_TX_DEFAULT)
	cantx.dlc      = 8;
	cantx.cd.uc[0] = CMD_CMD_TYPE2; // Payload code: (42) Request BMS responses

	/* Specify BMS nodes that will respond to poll. */
	cantx.cd.uc[1] = cd_uc1; // Default: only specified unit responds, module #1
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
			if (len < 3)
			{
				printf("Need more info--Enter 0 or a CAN ID\n");
				return -1;
			}
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
			if (canid_whoresp == 0)
			{
				printf("New: All nodes respond: %d\n",canid_whoresp );
				printfsettings();
				cd_uc1 = (0x3 << 6); // Save for re-entry init
				cantx.cd.uc[1] = cd_uc1; // Specified BMS responds
				cantx.cd.ui[1] = canid_whoresp; // CAN ID of BMS
				return 0;				
			}
			printf("New: BMS CAN ID: 0x%08X only responds\n",canid_whoresp);
			printfsettings();
			cd_uc1 = (0x1 << 6); // Save for re-entry init
			cantx.cd.uc[1] = cd_uc1; // Specified BMS responds
			cantx.cd.ui[1] = canid_whoresp; // CAN ID of BMS
			printfsettings();
			return 0;

		case 'r': // 'er' Set ADC "rate" code (default = 0: 422 Hz)
			if (len < 4)
			{
				printf("ADC rate code 0-7 and I got nothing.\n" );
				for (i = 0; i < 8; i++)
				{
					printf("\t%d: %s\n",i,pADCrate[i] );
				}
				return 0;
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
				printf ("Example: ep AEC00000\n");
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
			cantx.cd.ui[1] = canid_whoresp; // CAN ID of BMS
			cantx.cd.uc[1] = (0x1 << 6); // Use CAN ID in [4]-[7]
			cd_uc1 = (0x1 << 6); // Save for command re-entry
			printf("Single CANID: 0x%08X\n",cantx.cd.ui[1]);
			printfsettings();
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
			cantx.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xf) << 0);
			break;

		case 'a': // 'eax' All BMS nodes
			cantx.cd.ui[1] = 0; // NULL target CANID	
			cantx.cd.uc[1] = (0x3 << 6); // Use CAN ID in [4]-[7]
			cd_uc1 = (0x3 << 6); // Save for command re-entry
			printf("eaa: All BMS nodes\n");
			printfsettings();
			break;

		default:
			printf("2nd char not recognized: %c\n", *(p+1));
			canid_rx = 0;
			return -1;
		}
		printf ("\tCANID: BMS %08X  POLLER %08X\n",canid_rx,cantx.id);

		/* All but, 'x' == 'a' are CMD_CMD_TYPE2, so load [0] here. */
		cantx.cd.uc[0] = CMD_CMD_TYPE2; //  TYPE2 payload format 

		reqtype = *(p+2); // Save 'x' request code
		switch (reqtype)
		{
		case 'a': // Cell readings are CMD_CMD_CELLPOLL commands
			printf("Poll: Cells: calibrated readings\n");
			cantx.cd.uc[0] = CMD_CMD_CELLPOLL; // (42) Request BMS responses
			cantx.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xF) << 0);
			break;

		case 'j': // Set code to READ something menu (CMD_CMD_TYPE2 format)\n\t"
			ret = printreadmenu();
			if (ret < 0)
			{
				printf("Selection j failed: %d\n",ret);
				return -1; // Failed, or aborted
			}
printf("poll j\n");
			cantx.cd.uc[2] = ret; //  [1] TYPE2 MISCQ code	
			break;

		case 'k': // Misc SET something menu (CMD_CMD_TYPE2 format)\n\t"
			ret = printsetmenu();
			if (ret < 0)
			{
				printf("Selection k failed: %d\n",ret);
				return -1; // Failed, or aborted
			}
printf("poll k %d\n",ret);
//			cantx.cd.uc[2] = ret; //  [1] TYPE2 sub-code	
			break;			

		case 's': // Poll for status msgs
			cantx.cd.uc[2] = MISCQ_STATUS; //  [1] TYPE2 sub-code			
			printf("Poll: MISCQ_STATUS status code %d\n",cantx.cd.uc[2]);
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
		{
			cantx.cd.uc[2] = (groupctr & 0x0f);
			groupctr += 1;
		}
		if ((cantx.cd.uc[0] == CMD_CMD_TYPE2) && 
			(cantx.cd.uc[2] == MISCQ_SET_DCHGFETS) &&
			(walkfets_sw == 1))
		{ // Here, walking FETs in progress
			cantx.cd.uc[3] = walkfets_ctr;
			printf("WALK FET #%02d\n",walkfets_ctr);
			walkfets_ctr += 1;
			if (walkfets_ctr > 18)
				walkfets_ctr = 1;
		}
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
	int ret = timer_thread_init(&cmd_e_timerthread, 10000); // 10 ms
	if (ret != 0)
	{
		printf("\ncmd_e: timer thread startup unsucessful\n");
		return -1;
	}
	return 0;
}
