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
static void miscq_bits_r(struct CANRCVBUF* p);
static void timer_printresults(void);
static void print_cal_adc(char* pfmt, uint8_t ncol);
static void print_processor_adc_header(void);
static void print_bits_r(void);

static void cmd_e_timerthread(void);
static int starttimer(void);
/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds;
INSERT INTO CANID VALUES ('CANID_UNI_BMS_EMC_I','B0000000', 'BMSV1', 1,1,'U8_U8_U8_X4','BMSV1 UNIversal From EMC, Incoming msg to BMS: X4=target CANID');
INSERT INTO CANID VALUES ('CANID_UNI_BMS_PC_I' ,'B0200000', 'BMSV1', 1,1,'U8_U8_U8_X4','BMSV1 UNIversal From PC,  Incoming msg to BMS: X4=target CANID'); */
#define CANID_RX_DEFAULT CANID_MSG_BMS_CELLV12R // 0xB0201134 // Default BMS '1818 board #1 B0A00000'
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
 #define MISCQ_FETBALBITS 12 // FET on/off discharge bits
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

#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

#define NSTRING  2 // Max number of strings in a winch
#define NMODULE  8 // Max number of modules in a string
#define NCELL   18 // Max number of cells   in a module
#define NTHERM   3 // Max number of thermistors in a module
#define TIMERNEXTCOUNT 80 // 10 ms tick count between outputs
#define ADCDIRECTMAX 10   // Number of ADCs read in one scan with DMA

#define HEADERMAX 16 // Number of print groups between placing a header

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

static struct CELLMSG cellmsg[NSTRING][NMODULE][NCELL];
static struct CANRCVBUF cantx;
static uint32_t canid_rx;
static uint32_t canid_tx;
static uint8_t  whom;  // To whom is this addressed?
static uint8_t  reqtype; // Request type (miscq code)
static uint8_t  canseqnumber;

/* Readings returned determines if string and modules are present. */
static uint8_t yesstring[NSTRING]; // 0 = string number (0-NSTRING) not present
static uint8_t yesmodule[NSTRING][NMODULE]; // 0 = module number (0 - NMODULE) not present
static uint8_t nstring; // String number-1: 0-3
static uint8_t nmodule; // Module number-1: 0-15

uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static uint32_t timerctr;
static uint32_t timernext; // Next timer count

static uint8_t ncell_prev;
static uint8_t headerctr;

/******************************************************************************
 * static uint8_t storeandcheckstringandmodule(struct CANRCVBUF* p);
 * @brief 	: CAN msg 
 * @param	: p = pointer to can msg
 * @return  : 0 = OK; -1 = string; -2 = module
*******************************************************************************/
/*	      payload[2] U8: Module identification
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
	yesmodule[nstring][nmodule] = 1;
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
static void printmenu(char* p)
{
	printf(" e x [Minimum]\n");
	return;
}

/******************************************************************************
 * int cmd_e_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
/*
	printf("d - BMS heartbeat\n\t"
				"d  - default (cell readings by cell number: CANID: B0201134)\n\t"
				"dc aaaaaaaa  (cell readings by cell number: CANID: aaaaaaaa\n\t"
				"dz - default (cell readings voltage sorted: CANID: B0201134)\n\t"
				"dz aaaaaaaa  (cell readings voltage sorted: CANID: aaaaaaaa\n\t"
				"ds -         (bms status: CANID: B0201134)\n\t"
				"ds aaaaaaaa  (bms status: CANID: aaaaaaaa)\n");

	printf("e - BMS polling misc TYPE2 msgs\n\t"
				"e x  default  (CANID: BMS B0201124 POLL B0200000)\n\t"
				"emx  aaaaaaaa <pppppppp>(CANID: BMS aaaaaaaa POLL pppppppp) \n\t"
				"eax  <pppppppp> a (all BMS nodes respond)\n\t"
				"esx  <pppppppp> s (String number: 1-n)\n\t"
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
				"  x = r: Bits: fet status, opencell wires, installed cells"
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
	int i,j,k;

	printf("%c%c %i\n",*p,*(p+1),len);

	whom = 'c'; // To whom the request is issued.
	canid_rx = CANID_RX_DEFAULT; // CANID_MSG_BMS_CELLV11R 0xB0201124 // Default BMS

	/* This 'e' command only polls TYPE2 BMS msgs. */
	cantx.cd.ull   = 0; // Clear all payload bytes
	cantx.id       = CANID_TX_DEFAULT; // Default pollster
	cantx.dlc      = 8;
	cantx.cd.uc[0] = CMD_CMD_TYPE2; // (43) Request BMS responses
	cantx.cd.uc[2] = (1 << 6); // Default: only specified unit responds, module #1
	cantx.cd.ui[1] = CANID_RX_DEFAULT; // BMS node CANID

	/* Show no strings|modules|cells have been reported. */
	for (i = 0; i < NSTRING; i++)
	{
		yesstring[i] = 0;
		for (j = 0; j < NMODULE; j++)
		{
			yesmodule[i][j] = 0;
			for (k = 0; k < NCELL; k++)
			{
				cellmsg[i][j][k].flag = 0;
			}
		}
	}

	if (len < 4)
	{
		printmenu("Too few chars\n");
		return -1;
	}
	else if (len > 3)
	{	
		/* POLLER requests BMS node, string, or all. */
		switch ( *(p+1) )
		{ 
		case ' ': // Use default CANID for BMS and POLLER
			cantx.cd.ui[2] = CANID_RX_DEFAULT;
			whom = ' ';
			break;

		case 'm': // Use specified CANID
			if ((len > 13) && (len < 22))
			{ // Command has BMS CANID specified, POLLER is default
				sscanf( (p+3), "%x",&canid_rx);
			}
			else if (len >= 22)
			{ // Command has BMS and POLLER CANIDs specified
				sscanf( (p+3), "%x %x",&canid_rx, &canid_tx);
				cantx.id = canid_tx;
			}
			cantx.cd.ui[1] = canid_rx; // Set POLLER BMS target CANID
			whom = 'm';
			break;

		case 's': // Module String whom
			if (len > 13)
			{
				sscanf( (p+3), "%d %x",&tmp,&canid_tx);
				cantx.id = canid_tx;
			}
			// Strings respond | String number
			cantx.cd.uc[2] = (2 << 6) | ((tmp & 3) << 4); 
			cantx.cd.ui[1] = 0; // NULL target CANID
			whom = 's';
			break;

		case 'a': // All BMS nodes
			if (len > 13)
			{ // Here it looke like POLLER CANID was specified
				sscanf( (p+3), "%x",&canid_tx);
				cantx.id = canid_tx;
			}
			cantx.cd.uc[2] = (3 << 6); // All BMSs respond
			cantx.cd.ui[1] = 0; // NULL target CANID
			whom = 'a';
			break;

		default:
			printf("2nd char not recognized: %c\n", *(p+1));
			canid_rx = 0;
			return -1;
		}
		printf("whom polled: %c",whom);
		printf ("\tCANID: BMS %08X  POLLER %08X\n",canid_rx,cantx.id);

		/* Type of response. */
		reqtype = *(p+2);
		switch (reqtype)
		{
		case 'a': // Cell calibrated readings
			printf("Poll: Cells: calibrated readings\n");
			cantx.cd.uc[1] = MISCQ_CELLV_CAL; //  TYPE2 code
			break;

		case 'A': // Cell ADC raw adc counts for making calibrations
			printf("Poll: Cells: ADC Readings\n");
			cantx.cd.uc[1] = MISCQ_CELLV_ADC; //  TYPE2 code
			break;

		case 't': // Temperature calibrated readings
			printf("Poll: calibrated temperatures\n");
			cantx.cd.uc[1] = MISCQ_TEMP_CAL; //  TYPE2 code			
			break;

		case 'T': // Temperature raw adc counts for making calibrations
			printf("Poll: thermistor ADC readings\n");
			cantx.cd.uc[1] = MISCQ_TEMP_ADC; //  TYPE2 code			
			break;

		case 's': // BMS measured top-of-stack voltage MISCQ_TOPOFSTACK	
			printf("Poll: BMS measured top-of-stack voltage\n");
			cantx.cd.uc[1] = MISCQ_TOPOFSTACK;  //  TYPE2 code
			break;

		case 'd': // Isolated dc-dc converter output voltage
			printf("Poll: Isolated dc-dc converter output voltage\n");
			cantx.cd.uc[1] = MISCQ_DCDC_V;  //  TYPE2 code
			break;

		case 'f': // FET status for discharge
			printf("Poll: discharge FET status bits\n");
			cantx.cd.uc[1] = MISCQ_FETBALBITS;  //  TYPE2 code
			break;

		case 'w': // Processor ADC calibrated readings
			printf("Poll: Processor ADC calibrated readings\n");
			cantx.cd.uc[1] = MISCQ_PROC_CAL;  //  TYPE2 code
			break;

		case 'W': // Processor ADC raw adc counts for making calibrations
			printf("Poll: Processor ADC raw counts for making calibrations\n");
			cantx.cd.uc[1] = MISCQ_PROC_ADC;  //  TYPE2 code
			break;	

		case 'r': // Bits: fets, open cell wires, installed cells
			printf("Poll: Bits: fet status, disicharge fets, open cell wires, installed cells\n");
			printf( "\t  DUMP\n"
					"\t  |HEATER\n"
					"\t  ||DUMP2\n"
					"\t  |||Trickle Chgr\n"
					"\t  ||||Trickle Chgr Low rate\n"
					"\t  ||||||||\n"
				);
			cantx.cd.uc[1] = MISCQ_R_BITS;  //  TYPE2 code
			break;

		default:
			printf("3rd char not recognized: %c\n", *(p+2));
			return -1;
		}
	}

	ncell_prev =   0;
	headerctr  = HEADERMAX;
	kaON       =   1;
	timerctr   = 0;
	timernext  = TIMERNEXTCOUNT;
	starttimer();
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
		uint8_t err;
	/* Expect the BMS node CAN msg format TYPE1, TYPE2, etc
	     and skip other CAN IDs.
	   These #defines originate from the processing of the .sql file
	     ~/GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
	   which generates the file
	     ../../../../../svn_common/trunk/db/gen_db.h */

	uint32_t utmp = (p->id & 0xfffffffc);
	if ((utmp < (uint32_t)CANID_MSG_BMS_CELLV11R) || (utmp > (uint32_t)CANID_CMD_BMS_MISC28R))
		return; // CAN ID is not a BMS module function.

//printf("\n%08X %X", p->cd.uc[1]); // debug

	/* Here, CAN msg is from a BMS module. */

	/* Ignore msgs that are not the type requested. */
	// TYPE1
	// TYPE2 not requested TYPE
	if ( (p->cd.uc[0] != CMD_CMD_TYPE2) ||
		 (p->cd.uc[1] != cantx.cd.uc[1]) )  
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

	case MISCQ_R_BITS: // 'r' Bits: fets, cells open wire, cells installed	
		miscq_bits_r(p);	
		break;		

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
	if (kaON == 0) return; // No timer generated msgs

	timerctr += 1; // 10 ms tick counter
	if ((int)(timerctr - timernext) > 0)
	{ // Time to output accumulated readings
		timernext += TIMERNEXTCOUNT;
		timer_printresults();
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
			cellmsg[nstring][nmodule][idx].d = extractfloat(&p->cd.uc[4]);
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag
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
			cellmsg[nstring][nmodule][idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag		
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
			cellmsg[nstring][nmodule][idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag
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
			cellmsg[nstring][nmodule][idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag
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
	cellmsg[nstring][nmodule][0].d = (extractfloat(&p->cd.uc[4]));
	cellmsg[nstring][nmodule][0].flag = 1; // Reset new readings flag
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
	cellmsg[nstring][nmodule][0].d = (extractfloat(&p->cd.uc[4]));
	cellmsg[nstring][nmodule][0].flag = 1; // Show new reading is present
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
			cellmsg[nstring][nmodule][i].flag = 0;
		}
		else
		{
			cellmsg[nstring][nmodule][i].flag = 2;
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
			cellmsg[nstring][nmodule][idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag
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
			cellmsg[nstring][nmodule][idx].d = (extractfloat(&p->cd.uc[4]));
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag
		}
		return;
}
/******************************************************************************
 * static void miscq_bits_r(struct CANRCVBUF* p);
 * @brief 	: Request: bits for fets, open wire, installed cells
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
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
			cellmsg[nstring][nmodule][idx].u32 = (extractu32(&p->cd.uc[4]));
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag
		}
		return;
}
/******************************************************************************
 * static void timer_printresults(void);
 * @brief 	: Timer triggers output accumulated readings
*******************************************************************************/
static void timer_printresults(void)
{
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
/******************************************************************************
 * static void print_cal_adc(char* pfmt, uint8_t ncol);
 * @brief 	: Output accumulated readings
 * @param   : pfmt = pointer to format string
 * @param   : ncol = number of columns (readings) on a line
*******************************************************************************/
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
/******************************************************************************
 * static void print_cal_adc(char* pfmt, uint8_t ncol);
 * @brief 	: Output accumulated readings
 * @param   : pfmt = pointer to format string
 * @param   : ncol = number of columns (readings) on a line
*******************************************************************************/
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