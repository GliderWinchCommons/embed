/******************************************************************************
* File Name          : cmd_a.c
* Date First Issued  : 08/13/23
* Board              : PC
* Description        : Display converted BMS CAN msgs 
*******************************************************************************/
/*
*/

#include "cmd_a.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "../../../../../svn_common/trunk/db/gen_db.h"

extern int fdp;	/* port file descriptor */

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
static void print_processor_adc_header(void);
static void miscq_current_cal(struct CANRCVBUF* p);
static void miscq_read_aux(struct CANRCVBUF* p);
static void miscq_proc_temp(struct CANRCVBUF* p);
static void printheader(void);
static void miscq_chg_limits(struct CANRCVBUF* p);
static void miscq_morse_trap(struct CANRCVBUF* p);
//static void print_bits_r(void);

void printf_hdr_status(void);
static void miscq_status(struct CANRCVBUF* p);

/* 16b codes for impossible voltage readings. */
#define CELLVBAD   65535 // Yet to be defined
#define CELLVNONE  65534 // Cell position not installed
#define CELLVOPEN  65533 // Installed, but wire appears open

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
 #define MISCQ_MORSE_TRAP   38 // Retrieve stored morse_trap code & status codes.



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

#if 0
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
#endif

char *pmiscq[] = {
 "MISCQ_HEARTBEAT   0", // reserved for heartbeat
 "MISCQ_STATUS      1", // status
 "MISCQ_CELLV_CAL   2", // cell voltage: calibrated
 "MISCQ_CELLV_ADC   3", // cell voltage: adc counts
 "MISCQ_TEMP_CAL    4", // temperature sensor: calibrated
 "MISCQ_TEMP_ADC    5", // temperature sensor: adc counts for making calibration
 "MISCQ_DCDC_V      6", // isolated dc-dc converter output voltage
 "MISCQ_CHGR_V      7", // charger hv voltage
 "MISCQ_HALL_CAL    8", // Hall sensor: calibrated
 "MISCQ_HALL_ADC    9", // Hall sensor: adc counts for making calibration
 "MISCQ_CELLV_HI   10", // Highest cell voltage
 "MISCQ_CELLV_LO   11", // Lowest cell voltage
 "MISCQ_FETBALBITS 12", // Read FET on/off discharge bits
 "MISCQ_DUMP_ON	   13", // Turn on Dump FET for no more than ‘payload [3]’ secs
 "MISCQ_DUMP_OFF   14", // Turn off Dump FET
 "MISCQ_HEATER_ON  15", // Enable Heater mode to ‘payload [3] temperature
 "MISCQ_HEATER_OFF 16", // Turn Heater mode off.
 "MISCQ_TRICKL_OFF 17", // Turn trickle charger off for no more than ‘payload [3]’ secs
 "MISCQ_TOPOFSTACK 18", // BMS top-of-stack voltage
 "MISCQ_PROC_CAL   19", // Processor ADC calibrated readings
 "MISCQ_PROC_ADC   20", // Processor ADC raw adc counts for making calibrations
 "MISCQ_R_BITS     21", // Dump, dump2, heater, discharge bits
 "MISCQ_CURRENT_CAL 24", // Below cell #1 minus, current resistor: calibrated
 "MISCQ_CURRENT_ADC 25", // Below cell #1 minus, current resistor: adc counts
 "MISCQ_UNIMPLIMENT 26", // Command requested is not implemented
 "MISCQ_SETFETBITS  27", // Set FET on/off discharge bits
 "MISCQ_SETDCHGTST  28", // Set discharge test with heater fet load
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
static struct CELLMSG temperature[3];
static uint8_t hbseq;
//static struct CANRCVBUF cantx;
static uint32_t canid_rx = CANID_RX_DEFAULT;
//static uint8_t  whom;  // To whom is this addressed?
static uint8_t  reqtype; // Request type (miscq code)
static uint8_t reqcode;
//static uint8_t  canseqnumber;
//static uint8_t request;

/* Readings returned determines if string and modules are present. */
//static uint8_t yesstring[NSTRING]; // 0 = string number (0-NSTRING) not present
//static uint8_t yesmodule[NSTRING][NMODULE]; // 0 = module number (0 - NMODULE) not present
//static uint8_t nstring; // String number-1: 0-3
//static uint8_t nmodule; // Module number-1: 0-15
//static uint8_t ncell_prev;
static uint8_t headerctr;
//static uint8_t groupctr; // The six cell readings are sent in a group.
static uint8_t cmdcode;
static uint8_t responder;
static uint8_t respondcell;
static uint8_t reqcode;
static uint8_t oto_sw;

/*
Not implemented--*

Likely never used--
" 17 TRICKL_OFF   // Turn trickle charger off for no more than payload [3]’ secs\n\t",

*/

static char* preadmenu[] = {
 "  1 STATUS       // status\n\t",
 "  2 CELLV_CAL    // cell voltage: NOTE: for all cells use aa command\n\t",
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
 " 37 MISCQ_CHG_LIMITS // Show params: Module V max, Ext chg current max, Ext. chg bal\n\t", 
 " 38 MISCQ_MORSE_TRAP // Retrieve stored morse_trap code & status codes.\n",
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
	printf("Display selected BMS CAN msgs\n");
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
			reqcode = code;
			return select;
		}
	}
	printf("\nSomething went wrong with this menu lookup mess!\n");
	printf("Selection: %d, no match\n",select);
	return -1;
}


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
#if 0
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
	yesmodule[nstring][nmodule] = 1;
	return err;
}
#endif
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
	printf("\n");
	return;
}
/******************************************************************************
 * static void printauxheader(void);
 * @brief 	: Print more detail help.
 ******************************************************************************/
static void printauxheader(void)
{	int i;
	printf("\n"
		"   HALL"
		" THERM1"
		" THERM2"
		" THERM3"
		" 5V REG"
		" 3V REF"
		" CUR SNS"
		"   U$7 "
		"   U$3 "
		"   U$2 "
		"\n");
	for (i = 1; i < 6; i++)
		printf("% 7d", i);

	printf("    REF");

	for (i = 6; i < 10; i++)
		printf("% 7d", i);

	printf("  RCVD ");
	printf(" OV|UV|RDVD1\n");

	return;
}

/******************************************************************************
 * static void printhelp(void);
 * @brief 	: Print more detail help.
 ******************************************************************************/
static void printhelp(void)
{
	printf("Command options\n"
		"aw = Set new CANID\n"
		"aa = Display Cell readings\n"
		"aq = Display TYPE2 MISCQ response\n"
		);

	printf("Help:\nCell reading lines end with code for initiator of msg\n\t"
		"44 = HB (heartbeat) timeout\n\t"
		"46 = EMC1 (B0000000) polled BMS\n\t"
		"47 = PC   (AEC00000) polled BMS\n\t"
		"Heartbeat timeouts are timed from last polled cell readout request\n\t"
		);
}

int cmd_a_init(char* p)
{
//	uint32_t tmp;
	uint32_t len = strlen(p);
	int i;

	if (len <= 2)
	{
		printhelp();
		printf("CANID default: 0x%08X\n",canid_rx);
		return -1;
	}

	if (len > 2)
	{ 
		switch(*(p+1))
		{
		case 'w': // Set new CAN ID
			if (len > 11)
			{
				sscanf((p+2),"%x",&canid_rx);
				printf("New CANID set to %08X\n",canid_rx);
			}
			else
			{
				printf("Not enough chars for setting CAN ID\n"
					"Example: ew B0A00000<enter>\n");
			}
			return 0;
		case 'a': // Cell reading msgs
			printf("CELL readings for CANID: %08X\n",canid_rx);
			for (i = 0; i < 18; i++)
				printf("%8d",i+1);
			printf("\n");
			cmdcode = CMD_CMD_CELLPOLL;
			break;
	
		case 'q': // Type 2 msgs
			reqtype = printreadmenu();
			cmdcode = CMD_CMD_TYPE2;
			printf("TYPE2 msg code: %d for CANIDs %08X\n",reqtype,canid_rx);
			printheader();
			break;
		}
	}

	oto_sw = 0; // One time switch for printing headers
return 0;
}
/******************************************************************************
 * static void printheader(void);
 * @brief 	: Print appropriate header for TYPE2 selections
 ******************************************************************************/
static void printheader(void)
{
	oto_sw = 0;
	switch (reqtype)
	{
	case MISCQ_CELLV_ADC: // Cell ADC raw adc counts for making calibrations
		printf("Poll: Cells: ADC Readings\n");
		break;

	case MISCQ_PROC_TEMP: // Processor internal temperature (calibrated)
		printf("Processor internal temperature (deg C)\n");
		break;

	case MISCQ_HALL_CAL: // Hall effect current (AUX[0])
		printf("GPIO1: Hall effect current\n");
		break;

	case MISCQ_CURRENT_CAL:
		printf("Current sense resistance: calibrated\n");
		break;

	case MISCQ_TEMP_CAL: // Temperature calibrated readings
		printf("Poll: calibrated temperatures\n");
		break;

	case MISCQ_TEMP_ADC: // Temperature raw adc counts for making calibrations
		printf("Poll: thermistor ADC readings\n");
		break;

	case MISCQ_STATUS: // BMS measured top-of-stack voltage MISCQ_TOPOFSTACK	
		printf("Display Status, %s\n",pmiscq[MISCQ_STATUS]);
		break;

	case MISCQ_DCDC_V: // Isolated dc-dc converter output voltage
		printf("Poll: Isolated dc-dc converter output voltage\n");
		break;

	case MISCQ_FETBALBITS: // FET status for discharge
		printf("Poll: discharge FET status bits\n");
		break;

	case MISCQ_PROC_CAL: // Processor ADC calibrated readings
		printf("Poll: Processor ADC calibrated readings\n");
		print_processor_adc_header(); // Place a header
		break;

	case MISCQ_PROC_ADC: // Processor ADC raw adc counts for making calibrations
		printf("Poll: Processor ADC raw counts for making calibrations\n");
		break;	

	case MISCQ_READ_AUX: // Display AUX registers
		printf("Display AUX registers\n");
		printauxheader();
		break;	

	case MISCQ_CHG_LIMITS: // 37 Show params: Module V max, Ext chg current max, Ext. chg bal
		printf("Display external charger limits\n");
		break;

	case MISCQ_MORSE_TRAP: // 38  Stored morse_trap code & status codes.
		printf("Display rtc registers: morse_trap and status bytes\n");
		break;
	}
	headerctr  = HEADERMAX;
	return;
}

/******************************************************************************
 * void cmd_e_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_a_do_msg(struct CANRCVBUF* p)
{
	int i;
	uint8_t n;
	uint8_t dlc = p->dlc;
	uint8_t celln;
	double dtmp;
//	uint8_t err;
	/* Expect the BMS node CAN msg format TYPE2, etc
	     and skip other CAN IDs.
	   These #defines originate from the processing of the .sql file
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CMD_CODES_INSERT.sql
	   which generates the file
	     ../../../../../svn_common/trunk/db/gen_db.h */

	uint32_t utmp = (p->id & 0xfffffffc);
	if (utmp != canid_rx)
	{
//printf("utmp %08X:",utmp);printcanmsg(p);			
		return; // CAN ID is not a BMS module function.
	}
	/* Here, CAN msg is from a BMS module. */
	/* Ignore msgs that are not the type requested. */	
/*44,	'[1]-[7] cell readings: responses to timeout heartbeat');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCHB',    45,	'[1]-[7] misc data: timeout heartbeat');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLEMC',   46,	'[1]-[7] cell readings: response to emc sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLPC',    47,	'[1]-[7] cell readings: response to pc sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCEMC',   48,	'[1]-[7] misc data: response to emc sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCPC',    49,	'[1]-[7] misc data: response to pc sent CELLPOLL');
*/
//#define CHECKCODE	
	responder = p->cd.uc[0];
	if ((responder == CMD_CMD_CELLHB)  ||
        (responder == CMD_CMD_CELLEMC1) ||
        (responder == CMD_CMD_CELLEMC2) ||
        (responder == CMD_CMD_CELLPC)  )
	{
		respondcell = 1;
#ifdef CHECKCODE		
printf("CELL : %3d ",responder);
printcanmsg(p);	
#endif	
	}
	else
	{
		if ((responder == CMD_CMD_MISCHB)  ||
            (responder == CMD_CMD_MISCEMC1) ||
            (responder == CMD_CMD_MISCEMC2) ||
            (responder == CMD_CMD_MISCPC)  )
		{
			respondcell = 2;
#ifdef CHECKCODE			
printf("MISC : %3d ",responder);
printcanmsg(p);		
#endif
		}	
		else
		{
			if (responder != CMD_CMD_TYPE2)
			{
				respondcell = 3;
#ifdef CHECKCODE				
printf("TYPE2: %3d ",responder);
printcanmsg(p);	
#endif
			}
			else
			{
#ifdef CHECKCODE				
printf("???? : %3d ",responder);
printcanmsg(p);	
#endif	
			return;
			}
		}
	}

#ifdef CHECKCODE
printf("cmdcode: %d\n",cmdcode);	
#endif

	if ((respondcell == 1) && (cmdcode == CMD_CMD_CELLPOLL))
	{ // Here cell readings
		// Check msg is for the same reading group
		if ((p->cd.uc[1] & 0xf) != (hbseq & 0xf))
		{ // Here sequence number changed. Assume all received
			for (i = 0; i < NCELL; i++)
			{
				if (cellmsg[i].flag != 0)
				{
					if (cellmsg[i].u16 == CELLVNONE)
					{
						printf(" -9999.0");
//						printf("  .omit.");
					}
					else
					{
						if (cellmsg[i].u16 == CELLVOPEN)
						{
							printf("  7000.0");
						}
						else
						{
							dtmp = cellmsg[i].u16; // Convert to float
//printf("%8d",cellmsg[i].u16);							
							printf("%8.1f",dtmp*0.1); 
							cellmsg[i].flag = 0; // Clear was-read flag
						}
					}
				}
				else
				{ // No readings for this cell
					printf("    -1.0");
				}
			}
			printf("%4d\n",responder); // print line
			/* Update to new sequence number. */
			hbseq = p->cd.uc[1] & 0xf;
		}

		/* Add readings in payload into array cell positions. */
		// Check payload size 
		switch(dlc)
		{
//			case 4: n = 1; break; 
//			case 6: n = 2; break; 
			case 8: n = 3; break; 

			default:
				{
					printf("\nDLC not 8 %d",dlc);
					return;
				}
		}
		// Cell number of 1st payload U16
		celln = (p->cd.uc[1] >> 4) & 0xf;
		for (i =0 ; i < n; i++)
			{
				cellmsg[celln+i].u16 = p->cd.us[i+1];
				cellmsg[celln+i].flag = 1;
			}
		return;
	}
	if ((respondcell == 2) && (cmdcode == CMD_CMD_TYPE2))
	{		
	/* Here, TYPE2 msg. */
		if (reqcode != p->cd.uc[1])
			return;
		
		switch (p->cd.uc[1])
		{			
		case MISCQ_CELLV_CAL: // 'a' Cell voltage: calibrated
			miscq_cellv_cal(p);
			break;

		case MISCQ_CELLV_ADC: // 'A' Cell ADC readings
			miscq_cellv_adc(p);
			break;

		case MISCQ_DCDC_V: // 'd' DC-DC converter voltage
			miscq_miscq_dcdc_v(p);
			break;

		case MISCQ_FETBALBITS: // 'f' discharge FET status bits
			miscq_fetbalbits(p);
			break;

		case MISCQ_STATUS: // 's' status 
			miscq_status(p);
			break;

		case MISCQ_CURRENT_CAL: // 'i' Calibrated current sense
			miscq_current_cal(p);
			break;	

		case MISCQ_HALL_CAL: 
			miscq_current_cal(p);
			break;			

		case MISCQ_TEMP_CAL: // 't' Temperature calibrated readings		
			miscq_temp_cal(p);
			break;

		case MISCQ_TEMP_ADC: // 'T' Temperature ADC for calibration
			miscq_temp_adc(p);
			break;

		case MISCQ_TOPOFSTACK: // 'v' BMS measured top-of-stack voltage 	
			miscq_topofstack(p);
			break;

		case MISCQ_PROC_CAL: // 'w' Processor ADC calibrated readings
			miscq_proc_cal(p);
			break;

		case MISCQ_PROC_ADC: // 'W' Processor ADC raw adc counts for making calibrations
			miscq_proc_adc(p);	

		case MISCQ_UNIMPLIMENT:
			printf("Not implemented\n");
			break;

		case MISCQ_READ_AUX:
			miscq_read_aux(p);
			break;

		case MISCQ_PROC_TEMP: // 36 Processor calibrated internal temperature (deg C)
			miscq_proc_temp(p);
			break;	

		case MISCQ_CHG_LIMITS: // 37 Show params: Module V max, Ext chg current max, Ext. chg bal			
			miscq_chg_limits(p);
			break;

		case MISCQ_MORSE_TRAP: // 38  Stored morse_trap code & status codes.
			miscq_morse_trap(p);
			break;			

		default:
			printcanmsg(p); // CAN msg
			printf("TYPE2 command code not in switch statement: %d\n", p->cd.uc[1]);
			break;
		}
	}
	return;
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
#if 1
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
#endif
/******************************************************************************
 * static void miscq_cellv_cal(struct CANRCVBUF* pcan);
 * @brief 	: Request: Cell calibrated voltages
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
#if 1
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
#endif
/******************************************************************************
 * static void miscq_temp_cal(struct CANRCVBUF* pcan);
 * @brief 	: Request: Thermistor temperatures, calibrated
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_temp_cal(struct CANRCVBUF* p)
{
//printcanmsg(p);

	int idx = (p->cd.uc[3]);
	if (idx > 3)
	{ // Here, index is too high.
		printcanmsg(p);
		printf("T1-T3 index should be [1-3] but was too high or low: %d\n",p->cd.uc[3]);
	}
	else
	{  
		// Convert payload to float and scale to deg F
		temperature[idx].d = (extractfloat(&p->cd.uc[4]));
		temperature[idx].flag = 1; // Reset new readings flag
	}
	if (idx == 2)
	{ // Here, last of three readings received
		printf("%6.1f %6.1f %6.1f %3d\n",temperature[0].d,temperature[1].d,temperature[2].d,p->cd.uc[0]);
		temperature[0].d = 0;temperature[1].d = 0;temperature[2].d = 0;
	}
	return;
}
/******************************************************************************
 * static void miscq_read_aux(struct CANRCVBUF* p);
 * @brief 	: Display AUX registers
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static uint16_t aux_tmp[4*3];
static void miscq_read_aux(struct CANRCVBUF* p)
{
	int i;
	uint8_t idx = (p->cd.uc[3]);
		if (idx > 10)
		{ // Here,  Out-of-range
			printcanmsg(p);
			printf("Register index in this msg too high: %d\n",idx);
			return;
		}
		else
		{  
			aux_tmp[idx+0] = p->cd.us[2];
			aux_tmp[idx+1] = p->cd.us[3];
		}
		if (idx == 10)
		{ // Here, last pair received
			for (i = 0; i < 11; i++)
				printf(" %6d",aux_tmp[i]);

			printf("   0x%04X\n",aux_tmp[11]);
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
static double dadc_cal[9];
static void miscq_proc_cal(struct CANRCVBUF* p)
{
	int i;
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
			dadc_cal[idx] = (extractfloat(&p->cd.uc[4]));
		}
		if (idx == 8)
		{
			for (i = 0; i < 9; i++)
			{
				printf(" %8.3f",dadc_cal[i] );
			}
			printf("\n");
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
 * static void miscq_proc_temp(struct CANRCVBUF* p);
 * @brief 	: Request: Processor internal temperature
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_proc_temp(struct CANRCVBUF* p)
{
	double dtmp;
	dtmp = (extractfloat(&p->cd.uc[4]));
	printf("%6.1f\n",dtmp);
	return;
}
/******************************************************************************
 * static void miscq_current_cal(struct CANRCVBUF* p);
 * @brief 	: Request: Calibrated current from current sense resistance
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_current_cal(struct CANRCVBUF* p)
{
	double dtmp = (extractfloat(&p->cd.uc[4]));
	printf("%10.2f\n",dtmp);
	return;
}
/******************************************************************************
 * static void miscq_status(struct CANRCVBUF* p);
 * @brief 	: Display status bits
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
void printf_hdr_status(void)
{ printf("\n"
	"CAN ID: %08X\n"
	"No reading\n"
	"| Openwire (negative or over 4.3v)\n"
	"| | Cell too high\n"
	"| | | Cell too low\n"
	"| | | | Balance in progress\n"
	"| | | | | Onboard charger ON | DUMP2 ON\n"
	"| | | | | | Discharge to target voltage in progress\n"
	"| | | | | | | One or more cells very low\n"
	"| | | | | | | |     DUMP FET ON\n"
	"| | | | | | | |     | HEATER FET ON\n"
	"| | | | | | | |     | | DUMP2 FET ON\n"
	"| | | | | | | |     | | | On board charger normal rate\n"
	"| | | | | | | |     | | | | Charger low rate ON\n"
	"| | | | | | | |     | | | | |   Self-discharge ON\n"
	"| | | | | | | |     | | | | |   | One or more cells tripped\n"	
	"0 1 2 3 4 5 6 7     0 1 2 3 4   0 1 \n",canid_rx);
}
//static uint8_t hdrctr;
static void miscq_status(struct CANRCVBUF* p)
{
	int i;
/*
// Battery status bits: 'battery_status' 
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 4.3v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Low power charger ON
#define BSTATUS_DUMPTOV   (1 << 6)  // Discharge to a voltage in progress
#define BSTATUS_CELLVRYLO (1 << 7)  // One or more cells very low
*/	
char cc;
	if (oto_sw-- < 1)
	{
		oto_sw = 32;
		printf_hdr_status();
	}

	for (i = 0; i < 8; i++)
	{
		if ((p->cd.uc[4] & (1 << i)) == 0)
			cc = '.';
		else
			cc = 'B';
		printf("%c ",cc);
	}
	printf("    ");
/* FET status bits" 'fet_status' 
#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate	
*/
	for (i = 0; i < 5; i++)
	{
		if ((p->cd.uc[5] & (1 << i)) == 0)
			cc = '.';
		else
			cc = 'F';
		printf("%c ",cc);
	}
/* Mode status bits 'mode_status' 
#define MODE_SELFDCHG  (1 << 0) // 1 = Self discharge; 0 = charging
#define MODE_CELLTRIP  (1 << 1) // 1 = One or more cells tripped max	
*/
	printf("  ");
	for (i = 0; i < 2; i++)
	{
		if ((p->cd.uc[6] & (1 << i)) == 0)
			cc = '.';
		else
			cc = 'M';
		printf("%c ",cc);
	}
	printf("\n");
	return;
}
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
			if (yesmodule[i][j] == 0) continue;

			// Start line with string and module number
			printf("%1d:%1d:",i,j);
			for (k = 0; k < ncol; k++)
			{
				if (cellmsg[i].flag == 0)							
				{ // No cell reading
					printf(" .......");
				}
				else
				{   if (cellmsg[i].flag == 1)
					{
						printf(pfmt, cellmsg[i].d);
 					}
 					else
 					{
 						printf(" ...####");
 					}
 					cellmsg[i].flag = 0; // Reset new readings flag
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
static void print_processor_adc_header(void)
{		
//	if (headerctr >= HEADERMAX)
	{
		printf (
			    "     VREF"
				"  INTTEMP"
				"   DC-DC"
				"   HV DIV"
				"   CUR OP"
				"   SPARE1"
				"   SPARE2"
				"  FET CUR"
				"   FET RC"
				" OPA_OUT\n"
			);
	}
	return;
}
/******************************************************************************
 * static void miscq_chg_limits(struct CANRCVBUF* p);
 * @brief 	: Module params: Max chg current, Balance chg current, module max V
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_chg_limits(struct CANRCVBUF* p)
{
	double f1 = p->cd.uc[4] * 0.1;
	double f2 = p->cd.uc[5] * 0.1;
	double f3 = p->cd.us[3] * 0.1;

	if (oto_sw-- < 1)
	{
		oto_sw = 32;
 		printf("\n  Max I  Bal I   Vmax  %08X\n",canid_rx);
	}	
	printf("%7.1f%7.1f%7.1f\n",f1,f2,f3);
	return;
}
/******************************************************************************
 * static void miscq_morse_trap(struct CANRCVBUF* p);
 * @brief 	: Status bytes held in RTC Registers 
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_morse_trap(struct CANRCVBUF* p)
{
	if (p->cd.uc[3] != 0)
	{
		printf("\nrtcregs: morse_code: %d\n",p->cd.ui[1]);
	}
	else
	{
		printf("\nrtcregs: morse_code: registers NG: %0x\n",p->cd.uc[3]);		
	}
	return;
}

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
#endif
