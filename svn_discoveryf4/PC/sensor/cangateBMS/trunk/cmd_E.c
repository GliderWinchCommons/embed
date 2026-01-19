/******************************************************************************
* File Name          : cmd_E.c
* Date First Issued  : 09/23/23
* Board              : PC
* Description        : PC charging BMS string with ELCON charger
*******************************************************************************/
/*
*/
#include <time.h>
#include <errno.h>
#include "cmd_E.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"

/* The following are defaults which can be changed with commands. The changes
   remain in effect until cangateBMS is restarted. */
#define EXPECTED_NUM_MODULES_DEFAULT 4  // Default number of modules expected
#define DEFAULT_ELCON_INPUT_VOLTAGE 120 // Default ELCON input power voltage (volts)
#define DEFAULT_ELCON_INPUT_CURRENT  8  // Default ELCON input power max current (amps)
#define DEFAULT_ELCON_INPUT_POWER 1250  // Default ELCON input power max (watts)
#define DEFAULT_ELCON_OUTPUT_CURRENT_MAX 8 // Max charging current

/* See BQTask.h
Payload--
can.cd.uc[4] = Battery status
can.cd.uc[5] = FET status
can.cd.uc[6] = Mode status
can.cd.uc[7] = Temp
*/
/* Battery status bits: 'battery_status'  uc[4] */
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 4.3v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Low power charger ON
#define BSTATUS_DUMPTOV   (1 << 6)  // Discharge to a voltage in progress
#define BSTATUS_CELLVRYLO (1 << 7)  // One or more cells very low

/* FET status bits" 'fet_status' uc[5] */
#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

/* Mode status bits 'mode_status' uc[6] */
#define MODE_SELFDCHG  (1 << 0) // 1 = Self discharge; 0 = charging
#define MODE_CELLTRIP  (1 << 1) // 1 = One or more cells tripped max

/* See cancomm_items.h */
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
 #define MISCQ_SET_SELFDCHG 31 // Set self-discharge mode: OFF = charge; ON = self-discharge
 #define MISCQ_PRM_MAXCHG   32 // Get Parameter: Max charging current
 #define MISCQ_SET_ZEROCUR  33 // 1 = Zero external current in effect; 0 = maybe not.
 #define MISCQ_READ_AUX     34 // BMS responds with A,B,C,D AUX register readings (12 msgs)
 #define MISCQ_READ_ADDR    35 // BMS responds with 'n' bytes sent in [3]
 #define MISCQ_PROC_TEMP    36 // Processor calibrated internal temperature (deg C)
 #define MISCQ_CHG_LIMITS   37 // Show params: Module V max, Ext chg current max, Ext. chg bal

#define FMIN_STRING_V 109.0 // Below this string voltage ELCON will not charge
#define FNOC_STRING_V 80.0  // Below this ELCON NOT CONNECTED to string

/* EMC(?) status bytes. */
// cd.uc[4] 1st payload byte
#define STRING_ERR      (1 << 0)
#define STRING_OVR_V    (1 << 1)
#define STRING_CHG_FUL  (1 << 2)
#define STRING_CHG_BAL  (1 << 3)
#define STRING_MOD_TRIP (1 << 4)
#define STRING_MODE     (1 << 5)

/* ELCON status byte. p->cd.uc[4] */
#define ELCON_STATUS_HW_FAIL   (1 << 0) // Obvious
#define ELCON_STATUS_OVR_TEMP  (1 << 1) // Obvious
#define ELCON_STATUS_INPUT     (1 << 2) // Input AC is off
#define ELCON_STATUS_BATT_DISC (1 << 3) // Battery disconnected
#define ELCON_STATUS_COMM_TO   (1 << 4) // Communications timeout

FILE* fpIn;
char *paramlist = "/GliderWinchItems/BMS/bmsadbms1818/params/paramIDlist";

extern int fdp;	/* port file descriptor */

static void sendcan_type2(uint8_t code, uint8_t set);
static void sendcanmsg(struct CANRCVBUF* pcan);
static void printcanmsg(struct CANRCVBUF* p);
static void discovery(struct CANRCVBUF* p);
static void discovery_init(void);
static void cmd_E_timerthread(void);
static int starttimer(void);
static void printfbits(uint32_t x, uint8_t n);

// Time delay for consecutive CAN msgs
static struct timespec ts;

/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
INSERT INTO CANID VALUES ('CANID_ELCON_TX','C7FA872E','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger transmit: ');
INSERT INTO CANID VALUES ('CANID_ELCON_RX','C0372FA4','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger receive: '); */
#define CANID_RX_DEFAULT  CANID_ELCON_TX  // C7FA872C' This cmd RECEIVES; ==> ELCON transmits <==
#define CANID_TX_DEFAULT  CANID_ELCON_RX  // C0372FA4' This cmd SENDS; ==> ELCON receives <==
#define CANID_STATUS_POLL CANID_UNI_BMS_PC_I // AEC00000 PC originates
#define CANID_EMCMMC_CTL  CANID_CMD_PC_EMC //'98000000','UNIT_PC2EMC', 1,1,'U8_U8_U8_X4','PC sends commands to EMC

/* printf for debugging */
#define SKIPPRINT
#define SKIPPRINT2

union UF
{
	uint8_t uc[4];
	float f;
};

struct CHGVALUES
{
	uint32_t ivolts;
	uint32_t iamps;
};
static struct CHGVALUES chgfull;    // Charge rate gleaned from modules
static struct CHGVALUES chgbalance; // Charge rate for balancing 
static struct CHGVALUES chgzero;    // Charge volts & current for setting ELCON to zero
static struct CHGVALUES chgwork;    // Current working values
static struct CHGVALUES* pchgcmd;   // Values in last CAN command msg


struct BMSNODE
{
	struct CANRCVBUF can;    // Last received CAN msg for this 'id'
	struct CANRCVBUF canchg; // Discovery: charger limits
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
// Module bits positioned index in BMSnode
static uint32_t module_celltoohi; // One or more cells above max limit
static uint32_t module_selfdchg;     // 1 = One or more cells tripped max
//static uint32_t module_celltrip;     // 1 = Self discharge; 0 = charging
// String control: bits positioned by module index BMSnode list
static uint32_t module_tripped; // 1 = Module reported one or cells over max
static uint32_t module_mask; // Bits set for modules present
static uint32_t module_responded; // Status msgs received update bits
static uint32_t module_dump; // 1 = set bms dump on; 0 = off

/* Use this struct for the ELCON. */
static struct BMSNODE elcon; //


#define PARAMLISTSZ 32 // Possible size of paramlist
static uint32_t paramid[PARAMLISTSZ];
static uint8_t paramsize; // Number loaded into list


/* Line buffer size */
#define LINESIZE 2048
static char buf[LINESIZE];

static int8_t state; // 
static int8_t doneflag; //

// The following are 0.1 sec timer ticks
#define SECSTOWAIT       50 // Duration to monitor
#define BMSPOLL_INTERVAL 30 // BMS poll for status
#define BMSPOLL_TIMEOUT  50 // BMS report failure
#define DISCOVERY_INTERVAL 20 // Number of 0.1 secs to wait for discovery
#define DISCOVERY_POLL   10 // Polling for discovery interval
#define ELCON_INTERVAL   18 // ELCON timeout: it should report once per second
#define ELCON_KEEP_ALIVE 10 // Number of 0.1 secs between ELCON keep-alive
#define CANMSGSTIMEOUT  100 // Timeout for no CAN msgs coming in (gateway problems?)
#define PROGRESSTIME     10 // Hapless op keep-alive
#define CHGSTATPOLL      20 // Charging: wait to start next status update
#define CHGWAITREPLY     10 // Charging: wait to for BMS nodes to respond
#define TIMEOUT_ERR_IDLE 20 // Throttle error msgs

/* Time counters for timerctr ticks. */
static uint32_t timerctr; // Running count of 0.1 sec ticks
static uint32_t discoverytime;
static uint32_t discoverytimepoll;
static uint32_t elcon_timer_keep_alive;
//static uint32_t elcon_timer_interval;
static uint32_t canmsgstimeout; // Input CAN msg timeout
static uint32_t progresstime; 
static uint32_t timechgstatuspoll;
static uint32_t timechgloop;

/* Supply a default number of modules on the string. */
static uint8_t num_bms_modules = EXPECTED_NUM_MODULES_DEFAULT; // Number of BMS modules expected on string

/* Start with default values for ELCON input and output limitations. */
static float input_volts = DEFAULT_ELCON_INPUT_VOLTAGE; // 120 Default ELCON input power voltage (volts)
static float input_amps  = DEFAULT_ELCON_INPUT_CURRENT; // 15  // Default ELCON input power max current (amps)
static float input_watts = DEFAULT_ELCON_INPUT_POWER;   // 1250  // Default ELCON input power max (watts)
static float output_amps = DEFAULT_ELCON_OUTPUT_CURRENT_MAX; // 8 // Max charging current

static struct CANRCVBUF cantx_type2; // Generic TYPE2 msg to read
static struct CANRCVBUF cantx_type2k; // Generic TYPE2 msg to set
static struct CANRCVBUF cantx_cells; // Poll for cell readings
static struct CANRCVBUF cantx_elcon; // poll ELCON
static struct CANRCVBUF cantx_dump;  // Specific module DUMP update
static struct CANRCVBUF cantx_Emx;   // Send commands to EMCMMC (bmsmot)

//The following are for getting cell readings
static uint32_t adcrate; // *1818 ADC rate. 
static uint8_t groupctr; // The six cell readings are sent in a group.
static uint8_t canseqnumber;

static uint32_t canid_rx; // Receive ELCON

static uint32_t elcon_ivolts;
static uint32_t elcon_iamps;
static uint8_t elcon_overvolts; // 0 == OK; 1 = last report was over voltage

static float fmin_chg_cur;
static float fmin_bal_cur;
static float fmax_string_v;

static int32_t donect; // Throttle rate of "DONE" msgs

static int8_t EmMode; // 0 = Direct ELCON charging control; 1 = EMCMMC (bmsmot)

/******************************************************************************
 * static void print_chgingdisplay(void);
 * @brief 	: Display progress
 ******************************************************************************/
static uint32_t dsctr;
static void print_chgingdisplay(void)
{
	uint32_t ivolts;
	uint32_t iamps;
	float fvolts;
	float famps;
	struct CANRCVBUF* p = &elcon.can;
	printf("%4d ",dsctr++);
	if (elcon.present != 0)
	{ // Extract values from last CAN msg received
		ivolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
		iamps  = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
		/* Check reported voltage. */
		fvolts = ivolts * 0.1;
		famps  = iamps  * 0.1;
		printf("RCV: %5.1fv %5.1fa ",fvolts,famps);
		printf("status 0x%02X",p->cd.uc[4]);

		/* Display last command */
		p = &cantx_elcon;
		if (p == NULL)
		{
			fvolts = 0; ivolts = 0;
		}
		else
		{ // Extract values from last CAN msg sent
			ivolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
			iamps  = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
			/* Check reported voltage. */
			fvolts = ivolts * 0.1;
			famps  = iamps  * 0.1;
		}
		printf("  CMD: %5.1fv %5.1fa ",fvolts,famps);
		printf(" TOOHI: ");
		printfbits(module_celltoohi,num_bms_modules);
		printf(" TRIP: ");
		printfbits(module_tripped,num_bms_modules);
	}
	else
	{
		printf("print_chgingdisplay(): ELCON: no received msgs");
		printfbits(module_celltoohi,num_bms_modules);
	}
	printf("\n");
	return;
}
/******************************************************************************
 * static void printfbits(uint32_t x, uint8_t n );
 * @brief 	: Print bits : 0 - n i
 * @param   : x = value 
 * @param   : n = number of bits to print
 ******************************************************************************/
static char bitline[66];
static void printfbits(uint32_t x, uint8_t n )
{
	int i;
	if (n > 32) n = 32;
	memset(bitline,' ',n*2);
	for (i = 0; i < n; i++)
	{
		if ((x & (1 << i)) == 0)
			bitline[2*i] = '.';
		else
			bitline[2*i] = '1';
	}
	bitline[2*n-1] = 0; //Add string terminator
	printf("|%s|",bitline);
	return;
}
/******************************************************************************
 * static void printdiscovered(void);
 * @brief 	: printf discovered online BMS nodes
 ******************************************************************************/
static void printdiscovered(void)
{
	int i;
	printf("Discovery list,bmsnodes_online: %d\n",bmsnodes_online);	
	for (i = 0; i < bmsnodes_online; i++)
	{
		printf("%2d OK: LIMITS: ",i); 
		printcanmsg(&bmsnode[i].canchg);
		printf(" STATUS: ");
		printcanmsg(&bmsnode[i].canstatus);printf("\n");
	}
	return;
}
/******************************************************************************
 * static void sendcanmsg_dump(uint8_t bits);
 * @brief 	: Send CAN msgs to each online module to turn DUMP on/off
 * @param   : bits: 1 = DUMP ON, 0 = DUMP OFF
 ******************************************************************************/
static void sendcanmsg_dump(uint8_t bits)
{
	int i;
	// for each BMS node discovered
	for (i = 0; i < bmsnodes_online; i++)
	{
		if ((bits & (1<<i)) != 0)
		{ // Turn dump fet on 
			cantx_dump.cd.uc[3] = 1;
		}
		else
		{ // Turn dump fet off
			cantx_dump.cd.uc[3] = 0;
		}
		cantx_dump.cd.ui[1] = bmsnode[i].id;	
		sendcanmsg(&cantx_dump);
#ifndef SKIPPRINT		
printf("SEND-DUMP: %d %d 0x%08X: ",i,(bits & (1<<i)),cantx_dump.id);printcanmsg(&cantx_dump);printf("\n");
#endif
	}
	return;
}
/******************************************************************************
 * static void sendcanmsg_elcon(struct CHGVALUES* pchgrate);
 * @brief 	: Update payload and send command to ELCON
 * @param   : pchgrate = pointer to value struct; NULL = shutdown
 ******************************************************************************/
static void sendcanmsg_elcon(struct CHGVALUES* pchgrate)
{
	pchgcmd = pchgrate; // Save for printout monitoring

	if (pchgrate == NULL)
	{ // Shutdown ELCON
		cantx_elcon.cd.ull   = 0; // Clear all payload bytes
		cantx_elcon.cd.uc[4] = 1; // Battery protection, charger close output
	}
	else
	{ // Here, set charging rate values in payload
		cantx_elcon.cd.uc[0] = (pchgrate->ivolts >> 8);
		cantx_elcon.cd.uc[1] = (pchgrate->ivolts & 0xFF);
		cantx_elcon.cd.uc[2] = (pchgrate->iamps >> 8);
		cantx_elcon.cd.uc[3] = (pchgrate->iamps & 0xFF);
		cantx_elcon.cd.uc[4] = 0; // Charging on
	}
//printf("state: %d Sent to ELCON: ",state); printcanmsg(&cantx_elcon);printf("\n");
	sendcanmsg(&cantx_elcon);
	return;
}
/******************************************************************************
 * static void sendcan_type2(uint8_t code, uint8_t set);
 * @brief 	: Send TYPE2 msg to elicit response from all BMS nodes
 * @param   : code = MISCQ code (see #define)
 * @param   : set = cd.uc[3] (usually 0 or 1 to turn something off|on)
*******************************************************************************/
static void sendcan_type2(uint8_t code, uint8_t set)
{
	cantx_type2.cd.uc[2] = code;
	cantx_type2.cd.uc[3] = set;
	sendcanmsg(&cantx_type2);
	return;
}
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
 * static void print_Emx_List(void);
 * @brief 	: Help 
*******************************************************************************/
char* pEmx[] = {
	"0 = EMCMMC Control: BMS Self discharging mode, ELCON stop charging ",
	"1 = EMCMMC Control: ELCON start charging",
	"2 = EMCMMC Control: BMS Node Low Current: set charging mode",
	"3 = EMCMMC Control: BMS Node Low Current: self discharging/stop",
	"4 = EMCMMC Control: Debug: ELCON: Force volts & amps setting",
	"5 = EMCMMC Control: Debug: Set BMS poll rate",
};
static void print_Emx_List(void)
{
	int i;
	for (i = 0; i < 6; i++)
	{
		printf("\t%s\n",pEmx[i]);
	}
	return;
}
/******************************************************************************
 * static void printhelp(void);
 * @brief 	: Help 
*******************************************************************************/
/* Modes see StringChgrTask.h */
#define EMCCMD_ELIDLE   0 // ELCON: Self discharging/stop 
#define EMCCMD_ELCHG    1 // ELCON: Start charging
#define EMCCMD_LCCHG    2 // Node Low Current: set charging mode
#define EMCCMD_LCIDLE   3 // Node Low Current: self discharging/stop 
#define EMCCMD_DBGSET   4 // Debug: ELCON: Force volts & amps setting
#define EMCCMD_POLLRATE 5 // Debug: Set BMS poll rate

static void printhelp(void)
{
	printf("\n\t"
	"Eh: prints this command detail\n\t"
	"En <count> = Set new count of BMS nodes on string\n\t"
	"Ec Set ELCON: input volts input amps limit input input_watts limit\n\t"
	"Eo Set ELCON: output (charging) amps limit\n\t"
	"Ev: BMS discovery followed by charging\n\t"
	"Ex: Shutdown ELCON\n\t"
	"Emx: EMCMMC (bmsmot) control mode for ELCON\n\t"
	"  0: ELCON: Self discharging/stop \n\t"
	"  1: ELCON: Start charging\n\t"
	"  2: BMS Node Low Current: set charging mode\n\t"
	"  3: BMS Node Low Current: self discharging/stop\n\t"
	"  4: Debug: ELCON: Force volts & amps setting\n\t"
	"  5: Debug: Set BMS poll rate\n"
		);
}
/******************************************************************************
 * static void printsettings(void);
 * @brief 	: Help 
*******************************************************************************/
static void printsettings(void)
{
	printf("Setting----\n\tExpected number BMS modules on string: %d\n",num_bms_modules);
	printf("\tControl with EMCMMC (bmsmot) [reset with Ev or restart cangateBMS]: EmMode = %d\n",EmMode);
	return;
}
/******************************************************************************
 * static void printpowersettings(void);
 * @brief 	: Help 
*******************************************************************************/
static void printpowersettings(void)
{
	printf("Setting----\n\tExpected number BMS modules on string: %d\n",num_bms_modules);
	printf("\tControl with EMCMMC (bmsmot) [reset with Ev or restart cangateBMS]: EmMode = %d\n",EmMode);
	printf("ELCON INPUT: %8.1f volts  %8.1f amps limit  %8.0f watts limit\n", input_volts, input_amps, input_watts );
	printf("ELCON OUTPUT: %8.1f amps limit\n", output_amps);
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
	char *home_dir  = getenv("HOME");
	char *fullpath = malloc(strlen(home_dir)+1 + strlen(paramlist)+1);
    strcpy(fullpath, home_dir);
    strcat(fullpath, paramlist);
    printf("paramlist: fullpath: %s\n", fullpath);

	if ( (fpIn = fopen (fullpath,"r")) == NULL)
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
	printf("paramIDlist (BMS nodes): table size (paramsize): %d\n",paramsize);
	int j;
	for (j = 0; j < paramsize; j++)
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
	int i;
	int ret = 0;
	uint32_t len = strlen(p);
	float tmpv;
	float tmpa;
	float tmpw;
	float tmpo;

	printf("%c%c %i\n",*p,*(p+1),len);

	#define USEC 1500 // micoseconds wait
	ts.tv_sec = 0;
    ts.tv_nsec = (USEC * 1000);	

	canid_rx = CANID_RX_DEFAULT; // Default ID: ELCON tx, we rx

	/* Set default CAN msg to send to ELCON */
	cantx_elcon.id       = CANID_TX_DEFAULT; // 
	cantx_elcon.dlc      = 8;
	cantx_elcon.cd.ull   = 0; // Clear all payload bytes
	cantx_elcon.cd.uc[4] = 1; // Battery protection, charger close output

	/* Set command for polling BMS charger limits. */
	cantx_type2.id = CANID_STATUS_POLL;
	cantx_type2.dlc = 8;
	cantx_type2.cd.ui[1] = 0; // CAN ID payload zero
	cantx_type2.cd.uc[0] = CMD_CMD_TYPE2;	
	cantx_type2.cd.uc[2] = MISCQ_CHG_LIMITS;
	cantx_type2.cd.uc[1] = (0x3 << 6); // Nodes respond

	cantx_type2k = cantx_type2;

	/* BMS command to set specific module DUMP ON|OFF. */
	cantx_dump.id       = CANID_STATUS_POLL; // 
	cantx_dump.dlc      = 8;
	cantx_dump.cd.ull   = 0; // Clear payload
	cantx_dump.cd.uc[0] = CMD_CMD_TYPE2;
	cantx_dump.cd.uc[1] = (0x1 << 6); // Use CAN ID in [4]-[7]
	cantx_dump.cd.uc[2] = MISCQ_SET_DUMP;


	/* Set CAN msg for polling for cell voltage readings. */
	adcrate = 0; // *1818 ADC rate. 0 = 422 Hz
	cantx_cells.id       = CANID_STATUS_POLL; // 
	cantx_cells.cd.ui[1] = 0; // NULL target CANID	
	cantx_cells.cd.uc[0] = CMD_CMD_CELLPOLL; // (42) Request BMS responses
	cantx_cells.cd.uc[1] = (0x3 << 6); // 0xC0 = All BMS nodes respond
	cantx_cells.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xF) << 0);	

	/* Set command for EMCMMC (bmsmot) ELCON control. */
	cantx_Emx.id = CANID_EMCMMC_CTL;
	cantx_Emx.dlc = 8; // uc[0] amd uc[1] carry the ctl codes
	cantx_Emx.cd.ull = 0; //jic
	cantx_Emx.cd.uc[0] = CMD_EMC_SETMODE; // 53 [1]-[7] [0] = misc data: EMC TYPE2
	cantx_Emx.cd.uc[1] = EMCCMD_ELIDLE;   //  0  ELCON: Self discharging/stop  

	bmsnodes_online  = 0; // Number of discovered BMS nodes
	module_celltoohi = 0; // One or more cells above max limit
	module_selfdchg  = 0; // 1 = One or more cells tripped max
	module_tripped   = 0; // 1 = Module reported one or cells over max
	module_mask      = 0; // Bits set for modules present
	module_responded = 0; // Status msgs received update bits
	module_dump      = 0; // 1 = set bms dump on; 0 = off
	state            = 0; // 
	doneflag         = 0; //

	chgzero.ivolts = 0;
	chgzero.iamps  = 0;

	timerctr   = 0;	

	/* Check keyboard input. */
	if (len < 3)
	{ 
		printhelp();
		printsettings();
		printpowersettings();
		return 0;

	}	
	/* POLLER requests BMS node, string, or all. */
	switch ( *(p+1) )
	{ 
	case '\n':
	case ' ':
	case 'h':
		printhelp();
		printsettings();
		printpowersettings();		
		return 0;

	case 'm': // Send EMCMMC (bmsmot) commands
		EmMode = 1; // Mode 
		switch (*(p+2))
		{
		case '0': // EMCCMD_ELIDLE:  // 0 // ELCON: Self discharging/stop 
		case '1': // EMCCMD_ELCHG:   // 1 // ELCON: Start charging
		case '2': // EMCCMD_LCCHG:   // 2 // Node Low Current: set charging mode
		case '3': // EMCCMD_LCIDLE:  // 3 // Node Low Current: self discharging/stop 
		case '4': // EMCCMD_DBGSET:  // 4 // Debug: ELCON: Force volts & amps setting
		case '5': // EMCCMD_POLLRATE:// 5 // Debug: Set BMS poll rate
			cantx_Emx.cd.uc[1] = *(p+2)- '0'; // Set numeric code
			printf("Send CAN msg %08X %s\n", cantx_Emx.id, pEmx[*(p+2)-'0']);
			sendcanmsg(&cantx_Emx);
			return 0; //break;
		default:
			printf("Emx ERR: x = %c, not in case statement\n", *(p+2));
			print_Emx_List();
			return 0;
		}
		break;


	case 'n': // Set number of modules on string
		if (scansize(len,5) != 0) return -1; // check length
		sscanf((p+2),"%d",&i); // get what they want as int
		num_bms_modules = i; // egads a uint8_t!
		printf("\nUntil cangateBMS restarted, number of modules on string now: %d\n",num_bms_modules);
		break;

	case 'v': // Discovery then charge
		EmMode = 0; // Set direct ELCON control
		if (scansize(len,2) != 0) return -1;
		printpowersettings();
		printf("==================================================================================\n");
		printf("Start discovery of BMS nodes\nLooking for nodes with CAN IDs in the following list\n");
		if (getbmslist() != 0) // Get list of possible BMS nodes
		{
			ret = -1;
			break;
		}
		discovery_init();
		sendcan_type2(MISCQ_CHG_LIMITS,0);
		sendcan_type2(MISCQ_STATUS,0);
		canmsgstimeout = timerctr + CANMSGSTIMEOUT;
		break;

	case 'c': // Set ELCON max current limit
		ret = sscanf((p+2),"%f %f %f", &tmpv,&tmpa,&tmpw);
		if (ret < 3)
		{
			printf("Try again: I did not see three values: %d\n",ret);
			return 1;
		}
		if (tmpv >= 260) 
		{
			printf("Try again: Input voltage must be less than or equal 260 volts: %f\n",tmpv);
			return 1;
		}
		if (tmpv < 90)
		{
			printf("Try again: Input voltage must be greater than 90 volts: %f\n",tmpv);
			return 1;
		}
		if (tmpv > 0) 
		{
			input_volts = tmpv;
		}

		if ((tmpa > 15) || (tmpa < 0))
		{
			printf("Try again: Input current must be greater than zero and less or equal to 15 amps: %f\n",tmpa);
			return 1;
		}
		if (tmpa != 0) 
		{
			input_amps = tmpa;
		}
		if ((tmpw > 3300) || (tmpw < 0))
		{
			printf("Try again: Input power must be greater than zero and less or equal 3300 watts: %f\n",tmpv);
			return 1;
		}
		if (tmpw > 0) 
		{
			input_watts = tmpw;
		}
		printpowersettings();
		break;

	case 'o': // Set output charge current limit
		ret = sscanf((p+2),"%f",&tmpo);
		if (ret < 1)
		{
			printf("Try again: I did not see a value: %d\n",ret);
			return 1;
		}
		if (tmpo == 0)
		{
			break;
		}
		if ((tmpo > 16) || (tmpo < 0.1))
		{
			printf("Try again: Output charging current must be greater than 0.1a and less or equal to 16a: %f\n",tmpo);
			return 1;
		}
		if (tmpo > 0.1)
		{
			output_amps = tmpo;
		}
		printpowersettings();
		break;

	case 'x': // Close down
		sendcanmsg_dump(0);
		sendcanmsg_elcon(&chgzero); // ELCON shutdown
		return -1;


	default:
		printf("2nd char not recognized: %c\n", *(p+1));
		{
			ret = -1;
			break;
		}
	}
	
	elcon_timer_keep_alive  = ELCON_KEEP_ALIVE;
	sendcanmsg(&cantx_elcon);	// Send ELCON poll
	starttimer();
	return ret;
}
/******************************************************************************
 * static void charging_status(void);
 * @brief 	: Summarize status
*******************************************************************************/
/*
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 4.3v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Low power charger ON
#define BSTATUS_DUMPTOV   (1 << 6)  // Discharge to a voltage in progress
#define BSTATUS_CELLVRYLO (1 << 7)  // One or more cells very low

 FET status bits" 'fet_status' 
#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

 Mode status bits 'mode_status' 
#define MODE_SELFDCHG  (1 << 0) // 1 = Self discharge; 0 = charging
#define MODE_CELLTRIP  (1 << 1) // 1 = One or more cells tripped max
*/
/******************************************************************************
 * static void elcondatacheck(struct CANRCVBUF* p);
 * @brief 	: Update elcon payload and check voltage
 * @param   : p = pointer to CAN msg
 ******************************************************************************/
void elcondatacheck(struct CANRCVBUF* p)
{
	float dtmp;

	elcon_ivolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
	elcon_iamps  = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
	/* Check reported voltage. */
	dtmp = elcon_ivolts * 0.1;
	if (dtmp < FNOC_STRING_V)
	{ // Here, ELCON presumed not connected to string
		printf("ELCON reports: %0.1f volts and presumed not connected to battery\n",dtmp);
	}
	if ((dtmp < FMIN_STRING_V) && (dtmp > FNOC_STRING_V))
	{ // Here, string voltage too low for charging
		printf("ELCON reports: %0.1f volts which is too low for charging.\n",dtmp);
	}

	/* Check ELCON status byte. */
	if ((p->cd.uc[4] & ((1 << 5) - 1)) != 0)
	{ // Here one or more error flags
#ifndef SKIPPRINT		
		printf("ELCON status byte [4] = 0x%02X ",p->cd.uc[4]);
#endif		
		printfbits(p->cd.uc[4],5); printf("\n");
		if ((p->cd.uc[4] & ELCON_STATUS_HW_FAIL) != 0)
			printf("\tELCON reports HW_FAIL: hardware failure!\n");

		if ((p->cd.uc[4] & ELCON_STATUS_OVR_TEMP) != 0)
			printf("\tELCON reports ELCON_STATUS_OVR_TEMP: over temperature\n");

		if ((p->cd.uc[4] & ELCON_STATUS_INPUT) != 0)
			printf("\tELCON reports ELCON_STATUS_INPUT: no input power?\n");

		if ((p->cd.uc[4] & ELCON_STATUS_BATT_DISC) != 0)
			printf("\tELCON reports ELCON_STATUS_BATT_DISC: battery disconnect\n");

		if ((p->cd.uc[4] & ELCON_STATUS_COMM_TO) != 0)
			printf("\tELCON reports ELCON_STATUS_COMM_TO: communcation timeout\n");
		state = 10; // Avoid repetitive error msgs.
		return;
	}
	if (elcon_ivolts > fmax_string_v)
		elcon_overvolts = 1;
	else
		elcon_overvolts = 0;
	return;
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
	#define NODATA 9999	
	uint32_t min_chg_cur = NODATA;
	uint32_t min_bal_cur = NODATA;
	uint32_t max_string_v = NODATA;
	float ftmp;
	float fminbal = 1e6f;
	float fminchg = 1e6f;
	uint8_t flag;

	/* Start with all modules in not tripped state. */
	module_tripped = 0;

	/* Find minimum charge currents, and compute max string voltage. */
	/* Column header */
	printf("             maxVolts maxAmps balAmps\n");
	max_string_v = 0;
	for (i = 0; i < bmsnodes_online; i++)
	{
		if (bmsnode[i].canchg.id == 0)
		{ 
			printf("%08X did not report charger limits (code %d\n",MISCQ_CHG_LIMITS,bmsnode[i].id);
			state = 9;
			doneflag = 1;
			exit(-1);
		}

		/* Sum max module voltages for string. */
		max_string_v += bmsnode[i].canchg.cd.us[3];
		ftmp = (float)bmsnode[i].canchg.cd.us[3] *0.1f;
		printf(" %d %08X   %5.1f ",i, bmsnode[i].canchg.id,ftmp);

		/* Find minimum charge current for string. */
		ftmp = (float)bmsnode[i].canchg.cd.uc[4];
		if (ftmp < fminchg) fminchg = ftmp;
		{
			min_chg_cur = fminchg;
		}
		printf("   %4.1f",ftmp*0.1f);

		/* Find minimum balancing current for string. */
		ftmp = (float)bmsnode[i].canchg.cd.uc[5];
		if (ftmp < fminbal) fminbal = ftmp;
		{
			min_bal_cur = fminbal;
		}
		printf("   %4.1f\n",ftmp*0.1f);
	}
//#ifndef SKIPPRINT2
//	printf("min_chg_cur %3d min_bal_cur %3d max_string_v %4d\n",min_chg_cur,min_bal_cur,max_string_v);
	printf("String reported:  %10.1f  %6.1f %6.1f\n",(float)max_string_v*0.1f,(float)min_chg_cur*0.1f,(float)min_bal_cur*0.1f);
	ftmp  =  max_string_v;
// This was for boosting value for the proxy pack debugging	
#define VADJUST 1.00f
	float ftmp2 = VADJUST;	
	ftmp *= ftmp2;
	max_string_v = ftmp;
	printf("String adjusted:   adj%7.1f adj factor: %4.2f \n",(float)max_string_v*0.1f, ftmp2);
//#endif	

	/* Check that all modules reported their voltage and current limits, */
	flag = 0;
	for (i = 0; i < bmsnodes_online; i++)
	{
		if ((min_chg_cur == NODATA) || (min_bal_cur == NODATA) )
		{
			printf(" %d %08X NODATA for max/min volts, amps, etc.\n",i,bmsnode[i].id);
			flag = 1;
		}
	}
	for (i = 0; i < bmsnodes_online; i++)
	{
		if ((min_chg_cur == 0) ||
	        (min_bal_cur == 0) ||
	        (max_string_v == 0) )
		{
			printf("PROBLEM WITH MODULE PARAMETERS ZERO\n");
			printf(" %d %08X NODATA for max/min volts, amps, etc. (flag %d)\n",i,bmsnode[i].id,flag);
			flag = 2;
		}
	}
	if (flag != 0) exit (-1);


	/* NOTE: floats are in 1x units, ints (uint_16t) in 10x */
	fmax_string_v = max_string_v * 0.1;
	fmin_chg_cur  = min_chg_cur  * 0.1;
	fmin_bal_cur  = min_bal_cur  * 0.1;
	printf("Min of max Charging  current from BMS reports: %7.1fa\n",fmin_chg_cur);
	printf("Min of max Balancing current from BMS reports: %7.1fa\n",fmin_bal_cur);
	printf("Sum of max module volts(adj) from BMS reports: %7.1fv\n",fmax_string_v);

/* Computer charge current max to be the minimum of string charge max, and ELCON power source
   which can be input voltage & current limited, watts available, or simply
   a charge current that the Op has entered.
 */
	// Minimum of ELCON input (volt * current) watts, default (or Op updates) input power
	float ftmp_in_watts  = (input_volts * input_amps); // Power: input v * i;
	if (ftmp_in_watts < input_watts) ftmp_in_watts = input_watts; // Which is lower?

	// Minimum of above with charging watts, given string voltage
	float ftmp_chg_watts = (fmax_string_v * fmin_chg_cur); // BMS reported v & i
	if (ftmp_chg_watts < ftmp_in_watts) ftmp_chg_watts = ftmp_in_watts; // Which is lower?

	// Charging current as set by the smallest of the max wattages
	float ftmp_cur = ftmp_chg_watts / fmax_string_v; 

	// Mininum of current based on watts and current reported by modules
	if (ftmp_cur < fmin_chg_cur) ftmp_cur = fmin_chg_cur;

	// Update charging current, and scale to 0.1a units
	min_chg_cur = 10.0f * ftmp_cur;

/* DEBUG: Override values sent from nodes */
#if 0
max_string_v = 2160; // (0.1 v)
min_chg_cur  =   13; // (1.3 a)
min_bal_cur  =    1; // (0.1 a)
fmax_string_v = max_string_v * 0.1;
fmin_chg_cur  = min_chg_cur  * 0.1;
fmin_bal_cur  = min_bal_cur  * 0.1;
printf("Charging  current hard-code override:   %7.1fa\n",fmin_chg_cur);
printf("Balancing current hard-code override:   %7.1fa\n",fmin_bal_cur);
printf("Charger max volts hard-code override:   %7.1fv\n",fmax_string_v);
#endif

	chgbalance.ivolts = max_string_v;
	chgbalance.iamps  = min_bal_cur;  // Balancing (min) rate
	chgfull.ivolts    = max_string_v; // Max voltage
	chgfull.iamps     = fmin_chg_cur * 10.0f;  // Max current
	chgwork = chgfull; // Copy to working struct

	printf("Sum of max module volts(adj) from BMS reports: %7.1fv\n",fmax_string_v);
	printf("Final determination:         max string volts: %7.1fv\n",fmax_string_v);
	printf("Final determination:     max charging current: %7.1fv\n",ftmp_cur);
	printf("Final determination:          max ELCON watts: %7.0fv\n",ftmp_chg_watts);

	// Let the hapless Op know what the bozo programmer cobbled together.
	printf("Using these: %10.1f  %6.1f %6.1f\n",(float)max_string_v*0.1f,(float)min_chg_cur*0.1f,(float)min_bal_cur*0.1f);	

	/* Check for bogus parameters received. */
	if (fmax_string_v < FMIN_STRING_V)
	{ // Here, voltage too low for ELCON to charge
		printf("ERR: ELCON requires at least %0.1f string voltage\n",FMIN_STRING_V);
		state = 9;
		return;
	}
	if (max_string_v > 4000)
	{
		printf("ERR: Volts limit is 400.0\n");
		state = 9;
	}
	if (min_chg_cur > 100)
	{
		printf("ERR: Charge amps limit is 10.0\n");
		state = 9;
	}
	if ((min_bal_cur == 0) || (min_chg_cur == 0))
	{
		printf("ERR: Charge amps or Balance amps must not be zero\n");
		state = 9;
	}
	if (min_bal_cur > min_chg_cur)
	{
		printf("ERR: Balance amps greater than charge amps makes no sense\n");
		state = 9;
	}

	if (state == 9)
		exit(-1);

	/* Set keep-alive timeouts. */
	for (i = 0; i < bmsnodes_online; i++)
	{
		bmsnode[i].timeout = timerctr + BMSPOLL_INTERVAL;
	}

	/* Charging state. */
	state = 3;
	return;
}
/******************************************************************************
 * static void discovery(void);
 * @brief 	: Discovery time duration expired.
*******************************************************************************/
static void discovery_end(void)
{
	int i;
	uint8_t flag = 0;

	printf("Discovery End\n");

	/* Check if ELCON present. */
	printf("ELCON: %08X sent: ELCON ",elcon.can.id);
		if (elcon.present == 0)
		{ // Here ELCON msg received
			flag = 1;
			printf("missing\n");
		}
		else
		{
			printf("reporting\n");
		}

	/* Discovered module count must match expected count. */
	if (bmsnodes_online != num_bms_modules)
	{
		printf ("Discovered BMS nodes count %d not equal expected count %d\n",
			bmsnodes_online,num_bms_modules);	
		printdiscovered();
		printf("Change number of expected modules with En command\n");
			state = 9;		
			return;
	}
	printf("SUCCESS: Discovered equals Expected number of BMS nodes %d\n",bmsnodes_online);

	/* Check BMS nodes discovered. */
	for (i = 0; i < bmsnodes_online; i++)
	{
		if ((bmsnode[i].canchg.id == 0))
		{ // Here, msg with charger limits received
			flag |= 2;
			printf("%08X missing charger limits\n",bmsnode[i].id);
		}
		if ((bmsnode[i].canstatus.id == 0))
		{ // Here, status msg received
			flag |= 3;
			printf("%08X missing status\n",bmsnode[i].id);
		}
	}
	if (flag == 0)
	{ // All are OK, display list for hapless Op

		/* Print list of nodes. */
		printf("  ELCON    OK: "); 
		printcanmsg(&elcon.can); printf(" : ");printfbits(elcon.can.cd.uc[4],5);printf("\n");
		printdiscovered();

		/* Make a bit mask for BMS nodes. */
		module_mask = ((1 << bmsnodes_online) - 1);
printf("module_mask: 0x%02X ",module_mask);printfbits(module_mask,8);printf("\n");
		state = 3; // ????
		charging_int(); // Initialize charging phase
		printf("\n"); // Separate init printf from following 
		return; // Set state to charging phase
	}

	/* Don't keep repeating error msg. */
	if (state == 9) return;

	if ((elcon.present !=1))
	{
		printf("ELCON not discovered\n");
	}
	if (bmsnodes_online != num_bms_modules)
	{
		printf ("Discovered BMS nodes count %d NOT EQUAL EXPECTED count %d\n",
			bmsnodes_online,num_bms_modules);	
		printdiscovered();
	}
	state = 9;
	return; // Discovery failed
}
/******************************************************************************
 * static void discovery_init(void);
 * @brief 	: Initialize for discovery phase
*******************************************************************************/
static void discovery_init(void)
{
	state = 0; // Discovery state
	bmsnodes_online = 0; // Count BMS nodes found
	bmsnode[0].can.id = 0; // 
	bmsnode[0].canchg.id = 0; // 
	bmsnode[0].canstatus.id = 0; // 
	elcon.can.id  = 0; // 0 = not yet found
	elcon.present = 0; // Show ELCON not yet reported
	module_mask   = 0; // No modules as yet found
	discoverytime     = timerctr + DISCOVERY_INTERVAL;
	discoverytimepoll = timerctr + DISCOVERY_POLL;
	elcon.timeout     = timerctr + ELCON_INTERVAL + 100;
printf("Discovery init:     discoverytime %3d\n",discoverytime);
printf("Discovery init: discoverytimepoll %3d\n",discoverytimepoll);
printf("Discovery init:     elcon.timeout %3d\n",elcon.timeout);
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
printf("BMS node found %08X\n",p->id);
				bmsnode[j].id = p->id;
				bmsnodes_online += 1;
				if (bmsnodes_online >= BMSNODESZ)
				{
					printf("ERR: Discovery is finding more than possible %d\n",BMSNODESZ);
					state = 9;
					return;
				}
				bmsnode[bmsnodes_online].can.id = 0;
				bmsnode[bmsnodes_online].canchg.id = 0;
				bmsnode[bmsnodes_online].canstatus.id = 0;
			}

			if (p->cd.uc[1] == MISCQ_CHG_LIMITS)
			{ // Here msg has the current and voltage limits
				bmsnode[j].canchg = *p; // Save msg with charger limits
				bmsnode[j].timeout = timerctr + BMSPOLL_INTERVAL;
printf("LIM: %2d %2d %2d %08X",i, j,bmsnodes_online,p->id);						
printcanmsg(p);printf("\n");

			}
			else
			{ 
				if (p->cd.uc[1] == MISCQ_STATUS)
				{
				bmsnode[j].canstatus = *p; // Save msg with charger limits
				bmsnode[j].timeout = timerctr + BMSPOLL_INTERVAL;
printf("STA: %2d %2d %2d %08X",i, j,bmsnodes_online,p->id);						
printcanmsg(p);printf("\n");
				}
			}
			return;
		}
	}
	return;
}
/******************************************************************************
 * static void module_status_update(struct CANRCVBUF* p, uint8_t j);
 * @brief 	: Extract status from CAN msg and set bit for string
 * @param   : p = pointer to BMS node CAN msg with MISCQ_STATUS code
 * @param   : j = index as in BMSnode[j]
*******************************************************************************/
void module_status_update(struct CANRCVBUF* p, uint8_t j)
{
	bmsnode[j].canstatus = *p; // Save latest status response

	// Set bit for module in a word for string
	module_responded |= (1 << j); // Poll response results

	if ((p->cd.uc[4] & BSTATUS_CELLTOOHI) == 0)
		module_celltoohi &= ~(1 << j);
	else
		module_celltoohi |=  (1 << j);

	if ((p->cd.uc[6] & MODE_SELFDCHG) == 0)
		module_selfdchg &= ~(1 << j);
	else
		module_selfdchg |=  (1 << j);

	return;
}
/******************************************************************************
 * static void charging_poll(struct CANRCVBUF* p);
 * @brief 	: Charging phase: check incoming CAN msgs
 * @param   : p = pointer to CAN msg
*******************************************************************************/
static void charging_poll(struct CANRCVBUF* p)
{
	int j;
	// See if this CAN msg is in the BMS online list
	for (j = 0; j < bmsnodes_online; j++)
	{ // Here, the msg is in the list
		if (bmsnode[j].id == p->id)
		{ // Here, this CAN msg is in discovered list
			// Reset timeout for this BMS node
			bmsnode[j].timeout = timerctr + BMSPOLL_TIMEOUT;
			bmsnode[j].can = *p; // Save lastest CAN 
			// We are most interested in status at the moment
			if ((p->cd.uc[1] == MISCQ_STATUS) && (p->cd.uc[0] == 0x31))
			{ // Here msg has the status
				// Update last received status msg
				module_status_update(p,j);	
//printf("STATUS %08X %02X %02X %02X %02X\n",p->id, p->cd.uc[4],p->cd.uc[5],p->cd.uc[6],p->cd.uc[7]);
#ifndef SKIPPRINT				
	printf("STATUS POLL %08X %d:",p->id, p->dlc);
	int w;
	for (w = 0; w < p->dlc; w++) 
		printf(" %02X",p->cd.uc[w]);
	printf("\n");
#endif
			}
			return; // No need to check further
		}
	}
	return;
}

/******************************************************************************
 * void elcon_tx(struct CANRCVBUF* p);
 * @brief 	: Check for ELCON msgs 
*******************************************************************************/
void elcon_tx(struct CANRCVBUF* p)
{
//printcanmsg(p);printf("do_msg: timerctr %d el %d\n",timerctr, elcon.timeout);
	elcon.can = *p; // Save latest response from ELCON
	elcon.present = 1;
	elcon.timeout = timerctr + ELCON_INTERVAL;
	elcondatacheck(p); // Update ELCON payload data
	return;
}

/******************************************************************************
 * void cmd_E_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
void cmd_E_do_msg(struct CANRCVBUF* p)
{
	/* Overall check on gateway functioning. Reset timeout. */
	canmsgstimeout = timerctr + CANMSGSTIMEOUT;

	/* Handle CAN msgs */
	// ELCON sent this msg
	if (CANID_ELCON_TX == p->id)
	{
		elcon_tx(p);
		return;	
	}

	if (EmMode != 0)
	{
		return;
	}

	switch(state)
	{
	case 0: // Discovery 
		discovery(p);
		break;

	case 1: // End Discovery phase Initialize charging
		break;

	case 2: // Charging phase
		charging_poll(p);
		break;
	
	case 3:
		charging_poll(p);	
		break;

	case 4: // One or more modules did not respond to status update
		break;	

	case 8: // Successful completion.
		break; 

	case 9:
		if (donect-- < 0) // Throttle msgs
		{
			donect = 1024;
			printf("DONE, or bombed out %08X\n", p->id);
		}
		break;

	case 10:
		printf("EXIT (state==10)\n");
		exit(0);

	default:
		printf("cmd_E_do_msg: switch statement error: %d\n",state);
		state = 9;
		break;
	}
	return;
}

//xQueueSendToBack(CanTxQHandle,&ps->canbms,4);

/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void sendcanmsg(struct CANRCVBUF* pcan)
{
	uint8_t flag = 0;
	struct PCTOGATEWAY pctogateway; 

#if 1	
// This appears to insert a delay for consecutive calls to this routine????	
if (pcan->dlc == 0)	
{
#ifndef SKIPPRINT2	
	printf("\t\t\tSENDCANMSG dlc err\n");
#endif	
	pcan->dlc = 8;
	flag = 1;
}	
#endif
	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)

// This appears to insert a delay for consecutive calls to this routine????	
#if 1
if (flag == 1)
{	
	printf("TX: %08x %d:",pcan->id, pcan->dlc);
	int w;
	for (w = 0; w < pcan->dlc; w++)
		printf(" %02X",pcan->cd.u8[w]);
	printf("\n");
}
#endif

	return;
}
/******************************************************************************
 * static int checkallresponed(void);
 * @brief 	: 
 * @return  : 0 = OK; -1 = not all responded.
*******************************************************************************/	
static int checkallresponded(void)
{ // Here end of wait for BMS nodes to respond to status request
	// Check all that responded
	if (module_mask != module_responded)
	{ // Here, one or more modules did not update status
		state = 4; // Failure
		module_dump = 0; // Set all dumps off JIC
		sendcanmsg_dump(0);
		sendcanmsg_elcon(&chgzero); // ELCON shutdown
		printf("One or more modules did not respond to status update: 0x%04X\n",module_responded);
		return -1;
	}
	return 0;
}
/******************************************************************************
 * static void reducechgcurrent(void);
 * @brief 	: Cut back charging current command
*******************************************************************************/
#define CHG_REDUCE_FAC 0.625 // Reduction factor
#define CHG_REDUCE_INC 0.1  // Reduction increment (amps)
#define CHG_HILO 1.3 // ELCON switch from low current to hi pulses
static int reducechgcurrent(void)
{
  	float ttt;	
	float ftmpw; // Be lazy and  use floating point
	if (chgwork.iamps != chgbalance.iamps)
	{ // Here, we have not reached the minimum
		ftmpw = (float)chgwork.iamps * 0.1;
		if (ftmpw < (CHG_HILO-0.05) )
		{ // Current level is in the ELCON low step reduction range
			ftmpw -= CHG_REDUCE_INC; // Linear reduction
#ifndef SKIPPRINT2			
	printf("CHG REDUCE LINEAR: %6.2f\n",ftmpw);
#endif	
		}
		else
		{ // Current in the ELCON high reduction step range
			ttt = ftmpw;			
			ftmpw *= CHG_REDUCE_FAC; // Reduce by a factor
			printf("CHG_REDUCE FACTOR: new %6.2f prev %6.2f\n",ftmpw, ttt);
		}

		if (ftmpw < (float)chgbalance.iamps)
		{ // Here, reduction is below minimum possible
			chgwork.iamps = chgbalance.iamps; // Min forever
		}

		chgwork.iamps = ftmpw * 10.0;
		return 1;
	}
#ifndef SKIPPRINT2	
	printf("CHG REDUCE EQUAL chgwork.iamps %d  chgbalance.iamps %d\n",chgwork.iamps, chgbalance.iamps);
#endif	
	return 0; // Continue with minimum forever
}	
/******************************************************************************
 * static void printchgrate(void);
 * @brief 	: printf charge rate "work"
*******************************************************************************/
#ifndef SKIPPRINT
static void printchgrate(void)
{
	float famps = chgwork.iamps;
	printf(" %7.1f",(famps*0.1));
return;
}
#endif
/******************************************************************************
 * static void chgstatusget(void);
 * @brief 	: Deal with responses to status polling
*******************************************************************************/	
static void chgstatusget(void)
{ // Here end of wait for BMS nodes to respond to status request
	if (module_celltoohi != 0)
	{ // Here one of more modules are showing one or more cells over target
		if (reducechgcurrent() != 0)
		{ // We haven't hit minimum charge current
			sendcanmsg_elcon(&chgwork); 
			sendcanmsg_dump(module_celltoohi); // Turn on dump for module
#ifndef SKIPPRINT 
	printf("celltoohi but not min current: reduced charge rate is"); printchgrate();printf("a ");
	printfbits(module_celltoohi,num_bms_modules);printf("\n");
#endif
#ifndef SKIPPRINT2
	printf("chgstatusget: chgwork.iamps %4.1f\n",(float)chgwork.iamps*0.1f);
#endif		
			if ((module_mask == module_tripped) || (module_mask == module_celltoohi))
			{
							printf("\nAll modules have tripped their max. ELCON set off.\n"
				"############################# DONE! #############################\n");
				state = 10; // DONE
				doneflag = 1;
			}

			return;
		}
		// Here, chg current at minimum AND cell too high
		module_tripped |= module_celltoohi; // Show module tripped max

#ifndef SKIPPRINT2
	printf("chgstatusget:module_tripped %04X\n",module_tripped);
#endif	
		if (module_mask == module_tripped)
		{ // Here, all modules have tripped their max.
			sendcanmsg_dump(0);
			sendcanmsg_elcon(&chgzero); // Shutdown ELCON
			sendcan_type2(MISCQ_SET_SELFDCHG,0); // Complete charge balancing
			printf("\nAll modules have tripped their max. ELCON set off.\n"
				"############################# DONE! #############################\n");
			state = 10; // DONE
			doneflag = 1;
			return;
		}
		else
		{ // Here, not all modules tripped max
			// Turn BMS DUMP on for modules 'toohi'
			sendcanmsg_dump(module_celltoohi);

#ifndef SKIPPRINT2
	printf("chgstatusget: celltoohi: not all: %02X\n",module_celltoohi);
#endif
		}
	}
	else
	{ // Here, no modules report celltoohi
		sendcanmsg_dump(0); // Turn off DUMPs JIC

#ifndef SKIPPRINT 
		printf("No modules report celltoohi: charge rate "); printchgrate();printf("\n");
#endif
		sendcanmsg_elcon(&chgwork); // Turn charger on to present working current
	}
	return;
}
/******************************************************************************
 * static void cmd_E_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_E_timerthread(void)
{
	//int i;
	timerctr += 1; // 100 ms tick running counter

	if ((int)(timerctr - canmsgstimeout) > 0)
	{
		printf("CAN msgs are not coming in\n");
		canmsgstimeout += CANMSGSTIMEOUT;
		state = 9;
	}

	if (doneflag == 1)
	{ // Stop 
		return;
	}

	/* Send ELCON commands */
	if ((int)(timerctr - elcon_timer_keep_alive) > 0)
	{ // Time to output accumulated readings
		elcon_timer_keep_alive += ELCON_KEEP_ALIVE;
		sendcanmsg(&cantx_elcon);	// Send ELCON poll
	}

	/* Check ELCON msgs not being received. */
	if ((int)(timerctr - elcon.timeout) > 0)
	{
		elcon.present = 0;
		elcon.id = 0;
#ifndef SKIPPRINT
	printf(" timerctr %d el %d state %d  ",timerctr, elcon.timeout,state);		
#endif
		printf("ELCON timeout. It is not reporting\n");
		elcon.timeout = timerctr + 2000;//ELCON_INTERVAL;
		state = 9;
	}

	switch(state)
	{
	case 0: // Discovery phase
		if ((int)(timerctr - progresstime) > 0)
		{ // Something to keep hapless Op occupied
			printf("Discovery: monitoring %d\n",timerctr);
			progresstime += PROGRESSTIME;
		}
		if ((int)(timerctr - discoverytime) > 0)
		{ // Here, end of Discovery phase
			state = 1; // Switch how CAN msgs handled
			sendcan_type2(MISCQ_SET_SELFDCHG,0); // Set low currrent charge ON
			printf("Discovery: end %d\n",timerctr);
			discovery_end();
		}
		if ((int)(timerctr - discoverytimepoll) > 0)
		{
			discoverytimepoll = timerctr + DISCOVERY_POLL;
			sendcan_type2(MISCQ_CHG_LIMITS,0); // Request charge limit parameters
			sendcan_type2(MISCQ_STATUS,0); // Request status

			printf("Discovery poll %d\n",timerctr);
		}
		break;

	case 1:
#ifndef SKIPPRINT
	printf("tthrd: 1 timerctr %d\n",timerctr);
#endif
		break;

	case 2:
		if ((int)(timerctr - timechgloop) > 0)
		{ // BMS nodes should have responded by now
#ifndef SKIPPRINT			
	printf("tthrd: 2 timerctr %d\n",timerctr);
#endif
			if (checkallresponded() != 0)
			{ // state was set to 4;
				break;
			}
			chgstatusget();
			state = 3; // Waiting for next cycle
			// Set wait for beginning next cycle
			timechgstatuspoll = timerctr + CHGSTATPOLL; // +20
			print_chgingdisplay();

		}
		break;

	case 3:	// Check for end of wait-for-next-cycle
		if ((int)(timerctr - timechgstatuspoll) > 0)		
		{ // Here, start a new charging update cycle
#ifndef SKIPPRINT			
	printf("tthrd: 3 timerctr %d\n",timerctr);
#endif
			// Request cell readings (which triggers a status update)
			cantx_cells.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xF) << 0);
			sendcanmsg(&cantx_cells);    // Cell readings
			cantx_cells.cd.uc[2] = (groupctr & 0x0f);
			groupctr += 1;
			// Check that discovered modules update
			module_responded = 0;
			// Request status
			sendcan_type2(MISCQ_STATUS,0); 
			// Wait for units to respond
			timechgloop = timerctr + CHGWAITREPLY; // +5
			state = 2;
		}
		break;

	case 9: // Idle end
		break;

	default: // Something seriously wrong
		printf("ERR: timerthread state err: %d\n",state);
		state = 9;
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
