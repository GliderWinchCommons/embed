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

/* See BQTask.h */
/* Battery status bits: 'battery_status' */
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 4.3v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Low power charger ON
#define BSTATUS_DUMPTOV   (1 << 6)  // Discharge to a voltage in progress
#define BSTATUS_CELLVRYLO (1 << 7)  // One or more cells very low

/* FET status bits" 'fet_status' */
#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

/* Mode status bits 'mode_status' */
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
 #define MISCQ_SET_SELFDCHG 31 // Set ON|OFF self-discharge mode
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
#define ELCON_STATUS_INPUT     (1 << 2) // ?
#define ELCON_STATUS_BATT_DISC (1 << 3) // Battery disconnected
#define ELCON_STATUS_COMM_TO   (1 << 4) // Communications timeout

FILE* fpIn;
char *paramlist = "../../../../../../../GliderWinchItems/BMS/bmsadbms1818/params/paramIDlist";

extern int fdp;	/* port file descriptor */

static void sendcanmsg_bits(uint32_t bits, uint8_t code);
static void sendcan_type2(uint8_t code);
static void sendcanmsg(struct CANRCVBUF* pcan);
static void printcanmsg(struct CANRCVBUF* p);
static void discovery(struct CANRCVBUF* p);
static void discovery_init(void);
static void cmd_E_timerthread(void);
static int starttimer(void);

/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
INSERT INTO CANID VALUES ('CANID_ELCON_TX','C7FA872E','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger transmit: ');
INSERT INTO CANID VALUES ('CANID_ELCON_RX','C0372FA4','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger receive: '); */
#define CANID_RX_DEFAULT CANID_ELCON_TX  // C7FA872C' This cmd RECEIVES; ==> ELCON transmits <==
#define CANID_TX_DEFAULT CANID_ELCON_RX  // C0372FA4' This cmd SENDS; ==> ELCON receives <==
#define CANID_STATUS_POLL CANID_UNI_BMS_PC_I // AEC00000 PC originates

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
static struct CHGVALUES chgfull;
static struct CHGVALUES chgbalance;


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
static uint32_t module_celltrip;     // 1 = Self discharge; 0 = charging
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

// The following are 0.1 sec timer ticks
#define SECSTOWAIT       50 // Duration to monitor
#define BMSPOLL_INTERVAL 30 // BMS poll for status
#define BMSPOLL_TIMEOUT  50 // BMS report failure
#define DISCOVERY_INTERVAL 20 // Number of 0.1 secs to wait for discovery
#define DISCOVERY_POLL   10 // Polling for discovery interval
#define ELCON_INTERVAL   18 // ELCON timeout: it should report once per second
#define ELCON_KEEP_ALIVE 10 // Number of 0.1 secs between ELCON keep-alive
#define CANMSGSTIMEOUT   60
#define PROGRESSTIME     10 // Hapless op keep-alive
#define CHGSTATPOLL      20 // Charging: wait to start next status update
#define CHGSTATUSPOLL     5 // Charging: wait to for BMS nodes to respond
#define TIMEOUT_ERR_IDLE 20 // Throttle error msgs

/* Time counters for timerctr ticks. */
static uint32_t timerctr; // Running count of 0.1 sec ticks
static uint32_t discoverytime;
static uint32_t discoverytimepoll;
static uint32_t elcon_timer_keep_alive;
static uint32_t elcon_timer_interval;
static uint32_t canmsgstimeout; // Input CAN msg timeout
static uint32_t progresstime; 
static uint32_t timechgstatuspoll;
static uint32_t timechgloop;

/* Supply a default number of modules on the string. */
static uint8_t num_bms_modules = 6; // Number of BMS modules on string

static struct CANRCVBUF cantx_type2; // Generic TYPE2 msg to read
static struct CANRCVBUF cantx_type2k; // Generic TYPE2 msg to set
static struct CANRCVBUF cantx_cells; // Poll for cell readings
static struct CANRCVBUF cantx_elcon; // poll ELCON

//The following are for getting cell readings
static uint32_t adcrate; // *1818 ADC rate. 
static uint8_t groupctr; // The six cell readings are sent in a group.
static uint8_t canseqnumber;

static uint32_t canid_rx; // Receive ELCON

//static float fvolts;
//static float famps;
//static uint32_t ivolts;
//static uint32_t iamps;

static uint32_t elcon_ivolts;
static uint32_t elcon_iamps;
static uint8_t elcon_overvolts; // 0 == OK; 1 = last report was over voltage

static double fmin_chg_cur;
static double fmin_bal_cur;
static double fmax_string_v;

static float fmsgvolts;
static float fmsgamps;


uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.

/******************************************************************************
 * static void sendcanmsg_bits(uint32_t bits, uint8_t code);
 * @brief 	: Send CAN msgs to turn DUMP on/off
 * @param   : bits: 1 = DUMP ON, 0 = DUMP OFF
 * @param   : code = MISCQ code (see #define)
 ******************************************************************************/
static void sendcanmsg_dump(uint8_t bits)
{
	sendcanmsg_bits(bits,MISCQ_SET_DUMP);
	return;
}
/******************************************************************************
 * static void sendcanmsg_bits(uint32_t bits, uint8_t code);
 * @brief 	: Send CAN msgs to turn DUMP on/off
 * @param   : bits: 1 = DUMP ON, 0 = DUMP OFF
 * @param   : code = MISCQ code (see #define)
 ******************************************************************************/
static void sendcanmsg_bits(uint32_t bits, uint8_t code)
{
	int i;

if ((~module_mask & bits) != 0)	
{
	printf("something wrong with module_mask and bits to set: mod: %08X bits: %08X\n",module_mask,bits);
	state = -9;
}

	cantx_type2.cd.uc[2] = code;
	for (i = 0; i < bmsnodes_online; i++)
	{
		if ((bits & (1 << i)) == 0)
		{
			cantx_type2.cd.uc[3] = 0;
		}
		else
		{
			cantx_type2.cd.uc[3] = 1;
		}
		sendcanmsg(&cantx_type2);
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
	sendcanmsg(&cantx_elcon);
	return;
}
/******************************************************************************
 * static void sendcan_type2(uint8_t code);
 * @brief 	: Send TYPE2 msg to elicit response from all BMS nodes
 * @param   : code = MISCQ code (see #define)
*******************************************************************************/
static void sendcan_type2(uint8_t code)
{
	cantx_type2.cd.uc[2] = code;
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
	int i;
	uint32_t len = strlen(p);

	printf("%c%c %i\n",*p,*(p+1),len);

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

		/* Set CAN msg for polling for cell voltage readings. */
	adcrate = 0; // *1818 ADC rate. 0 = 422 Hz
	cantx_cells.id       = CANID_TX_DEFAULT; // 
	cantx_cells.cd.ui[1] = 0; // NULL target CANID	
	cantx_cells.cd.uc[0] = CMD_CMD_CELLPOLL; // (42) Request BMS responses
	cantx_cells.cd.uc[1] = (0x3 << 6); // Use CAN ID in [4]-[7]
	cantx_cells.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xF) << 0);	



	/* Check keyboard input. */
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
		if (scansize(len,5) != 0) return -1; // check length
		sscanf((p+2),"%d",&i); // get what they want as int
		num_bms_modules = i; // egads a uint8_t!
		printf("\nUntil cangateBMS restarted, number of modules on string now: %d\n",num_bms_modules);
		return 0;

	case 'v': // Discovery only
		if (scansize(len,2) != 0) return -1;
		printf("Start discovery of BMS nodes\n");
		if (getbmslist() != 0) // Get list of possible BMS nodes
			return -1;
		discovery_init();
		sendcan_type2(MISCQ_CHG_LIMITS);
		sendcan_type2(MISCQ_STATUS);
		break;

	case 'j': // Set charger off bit to 1
		cantx_elcon.cd.uc[4] = 1;
		sendcanmsg(&cantx_elcon);
		break;

	case 'o': // et charger off bit to 0	
		cantx_elcon.cd.uc[4] = 0;
		sendcanmsg(&cantx_elcon);
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
 * @return  : 
*******************************************************************************/
void elcondatacheck(struct CANRCVBUF* p)
{
	double dtmp;

	elcon_ivolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
	elcon_iamps  = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
	/* Check reported voltage. */
	dtmp = elcon_ivolts * 0.1;
	if (dtmp < FNOC_STRING_V)
	{ // Here, ELCON presumed not connected to string
		printf("ELCON reports: %0.1f volts and presumed not connected to string",dtmp);
	}
	if (dtmp < FMIN_STRING_V)
	{ // Here, string voltage too low for charging
		printf("ELCON reports: %0.1f volts which is too low for charging.",dtmp);
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
	uint32_t min_chg_cur = 1000;
	uint32_t min_bal_cur = 1000;
	uint32_t max_string_v = 0;
	uint32_t tmp;

	/* Start with all modules in not tripped state. */
	module_tripped = 0;

	/* Find minimum charge currents, and compute max string voltage. */
	for (i = 0; i < bmsnodes_online; i++)
	{
		if (bmsnode[i].canchg.id == 0)
		{ 
			printf("%08X did not report charger limits (code %d\n",MISCQ_CHG_LIMITS,bmsnode[i].id);
			state = 9;
			return;
		}
		/* Sum max module voltages for string. */
		max_string_v += bmsnode[i].canchg.cd.us[3];
		/* Find minimum charge current for string. */
		tmp = bmsnode[i].canchg.cd.uc[4];
		/* Find minimum balancing current for string. */
		if (tmp < min_chg_cur) min_chg_cur = tmp;
		tmp = bmsnode[i].canchg.cd.uc[5];
		if (tmp < min_bal_cur) min_bal_cur = tmp;
	}

	/* Load two sets of values for setting ELCON. */
	chgfull.ivolts    = max_string_v;
	chgfull.iamps     = min_chg_cur;
	chgbalance.ivolts = max_string_v;
	chgbalance.iamps  = min_bal_cur;

	fmax_string_v = max_string_v * 0.1;
	fmin_chg_cur  = min_chg_cur * 0.1;
	fmin_bal_cur  = min_bal_cur * 0.1;
	printf("Charging  current: %7.1fa\n",fmin_chg_cur);
	printf("Balancing current: %7.1fa\n",fmin_bal_cur);
	printf("Charger max volts: %7.2fv\n",fmax_string_v);

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
		return;	
	}
	if (min_chg_cur > 100)
	{
		printf("ERR: Charge amps limit is 10.0\n");
		state = 9;
		return;
	}
	if ((min_bal_cur == 0) || (min_chg_cur == 0))
	{
		printf("ERR: Charge amps or Balance amps must not be zero\n");
		state = 9;
		return;			
	}
	if (min_bal_cur > min_chg_cur)
	{
		printf("ERR: Balance amps greater than charge amps makes no sense\n");
		state = 9;
		return;					
	}

	/* Set keep-alive timeouts. */
	for (i = 0; i < bmsnodes_online; i++)
	{
		bmsnode[i].timeout = timerctr + BMSPOLL_INTERVAL;
	}

	/* Charging state. */
	state = 2;
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

	/* Check if ELCON present. */
	printf("ELCON: %08X\n",elcon.can.id);
		if (elcon.present == 0)
		{ // Here ELCON msg received
			flag |= 1;
			printf("ELCON missing\n");
		}

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
		printf("  ELCON    OK: "); 
		printcanmsg(&elcon.can);printf("\n");
		for (i = 0; i < bmsnodes_online; i++)
		{
			printf("%2d OK: LIMITS: ",i); 
			printcanmsg(&bmsnode[i].canchg);
			printf(" STATUS: ");
			printcanmsg(&bmsnode[i].canstatus);printf("\n");
		}
		/* Make a bit mask for BMS nodes. */
		module_mask = ((1 << bmsnodes_online) - 1);
printf("module_mask: %02X\n",module_mask);
		state = 2;
		charging_int(); // Initialize charging phase
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
		printf ("Discovered BMS nodes count %d not equal expected count %d\n",
			bmsnodes_online,num_bms_modules);	
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
	bmsnodes_online = 0; // Count BMS nodes found
	bmsnode[0].can.id = 0; // 
	bmsnode[0].canchg.id = 0; // 
	bmsnode[0].canstatus.id = 0; // 
	elcon.can.id  = 0; // 0 = not yet found
	elcon.present = 0; // Show ELCON not yet reported
	module_mask   = 0; // No modules as yet found
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
	// Set bit for this node in celltoohi 
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
 * @brief 	: Charging phase
 * @param   : p = pointer to CAN msg
*******************************************************************************/
static void charging_poll(struct CANRCVBUF* p)
{
	int j;
	if (CANID_ELCON_TX == p->id)
	{ // Here, ELCON
		elcon.can = *p; // Save latest response from ELCON
		elcon.present = 1;
		elcon.timeout = timerctr + ELCON_INTERVAL;
		return;
	}

	// See if this CAN msg is in the BMS online list
	for (j = 0; j < bmsnodes_online; j++)
	{ // Here, the msg is in the list
		if (bmsnode[j].id == p->id)
		{
			if (p->cd.uc[1] == MISCQ_STATUS)
			{ // Here msg has the status
				// Reset timeout for this BMS node
				bmsnode[j].timeout = timerctr + BMSPOLL_TIMEOUT;
				// Update last received status msg
				bmsnode[j].canstatus = *p; 
				module_status_update(p,j);
			}
			return;
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
// NOTE: DMOC will go here also?
	/* Check for ELCON msgs. */
	if (CANID_ELCON_TX == p->id)
	{ // Here, ELCON tx and we rx
		elcon.can = *p; // Save latest response from ELCON
		elcon.present = 1;
		elcon.timeout = timerctr + ELCON_INTERVAL;
		elcondatacheck(p); // Update ELCON payload data
		return;
	}

	canmsgstimeout = timerctr + CANMSGSTIMEOUT;

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
		break;

	case 4: // One or more modules did not respond to status updatge
		break;	

	case 8: // Successful completion.
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

	uint32_t elcon_ivolts = (p->cd.uc[0] << 8) | (p->cd.uc[1]);
	uint32_t elcon_iamps = (p->cd.uc[2] << 8) | (p->cd.uc[3]);
	fmsgvolts = elcon_ivolts;
	fmsgamps  = elcon_iamps;	
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
 * static void chgstatpoll(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void chgstatusget(void)
{ // Here end of wait for BMS nodes to respond to status request
	// Check all that responded
	if (module_mask != module_responded)
	{ // Here, one or more modules did not update status
		state = 4; // Failure
		module_dump = 0; // Set all dumps off JIC
		sendcanmsg_dump(0);
		sendcanmsg_elcon(0); // ELCON shutdown
		return;
	}
	// Here all nodes updated their status
	if (module_celltoohi != 0)
	{ // Here one of more modules are showing one or more cells over target
		sendcanmsg_elcon(0); // Set ELCON off
		module_celltrip |= module_celltoohi; // Show module tripped max
		if (module_mask == module_celltrip)
		{ // Here, all modules have tripped their max.
		sendcanmsg_dump(0);
		sendcanmsg_elcon(0); // Shutdown ELCON
		printf("All modules have tripped their max. ELCON set off. DONE!\n");
		state = 8; // DONE
		}
		else
		{ // Here, not all modules tripped max
			// Turn BMS DUMP on for modules 'toohi'
			sendcanmsg_dump(module_celltoohi);
		}
	}
	else
	{ // Here, no modules report celltoohi
		sendcanmsg_dump(0); // Turn off DUMPs
		if (module_tripped == 0)
		{
			sendcanmsg_elcon(&chgfull); // Turn charger on
		}
		else
		{
			sendcanmsg_elcon(&chgbalance); // Turn charger on			
		}
	}

	return;
}
/******************************************************************************
 * static void cmd_E_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_E_timerthread(void)
{
	int i;
	timerctr += 1; // 100 ms tick running counter

	if ((int)(timerctr - canmsgstimeout) > 0)
	{
		printf("CAN msgs are not coming in\n");
		canmsgstimeout += CANMSGSTIMEOUT;
		state = 9;
	}

	/* Send ELCON commands */
	if ((int)(timerctr - elcon_timer_keep_alive) > 0)
	{ // Time to output accumulated readings
		elcon_timer_keep_alive += ELCON_KEEP_ALIVE;
		sendcanmsg(&cantx_elcon);	// Send ELCON poll
	}

	/* Check ELCON msgs not being received. */
	if ((int)(timerctr - elcon_timer_interval) > 0)
	{
		elcon.present = 0;
		elcon.id = 0;
		printf("ELCON timeout. It is not reporting\n");
		elcon_timer_interval =timerctr + ELCON_INTERVAL;
		state = 9;
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
			sendcan_type2(MISCQ_CHG_LIMITS);
			sendcan_type2(MISCQ_STATUS);
//			sendcanmsg(&cantx_chglmt);
//			sendcanmsg(&cantx_status);
		}
		break;

	case 1:
		break;

	case 2:
		for (i = 0; i < bmsnodes_online; i++)
		{
			if ((timerctr - timechgloop) > 0)
			{ // Units should have responded to status request
				// Check response and update charging 
				chgstatusget();
				// Set wait for next cycle
				state = 3; // Waiting for next cycle
				timerctr = timechgloop + CHGSTATPOLL;
			}
		}
		break;

	case 3:	// Check for end of wait-for-next-cycle
		if ((timerctr - timechgstatuspoll) > 0)		
		{ // Here, start a new charging update cycle
			// Request cell readings (which triggers a status update)
			cantx_cells.cd.uc[2] = (adcrate << 4) | ((groupctr & 0xF) << 0);
			sendcanmsg(&cantx_cells);    // Cell readings
			cantx_cells.cd.uc[2] = (groupctr & 0x0f);
			groupctr += 1;
			// Request status
			sendcan_type2(MISCQ_STATUS); 
			// Wait for units to respond
			timechgstatuspoll = timerctr + CHGSTATUSPOLL;
			state = 2;
		}
		break;

	case 9: // Idle end
		break;

	default: // Something seriously wrong
		printf("timerthread state err: %d\n",state);
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
