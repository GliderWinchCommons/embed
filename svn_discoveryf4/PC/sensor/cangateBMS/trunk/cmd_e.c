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
static void miscq_cellv_cal(struct CANRCVBUF* p);
static void miscq_cellv_adc(struct CANRCVBUF* p);
static void miscq_temp_cal(struct CANRCVBUF* p);
static void miscq_temp_adc(struct CANRCVBUF* p);
static void miscq_topofstack(struct CANRCVBUF* p);
static void miscq_miscq_dcdc_v(struct CANRCVBUF* p);
static void timer_printresults(void);
static void print_cal_adc(char* pfmt, uint8_t ncol);

static void cmd_e_timerthread(void);
static int starttimer(void);


#define CANID_RX_DEFAULT CANID_MSG_BMS_CELLV12R // 0xB0201124 // Default BMS
#define CANID_TX_DEFAULT CANID_UNI_BMS_I        // 0XB0000000 // Default pollster

 #define MISCQ_HEARTBEAT   0 // reserved for heartbeat
 #define MISCQ_STATUS      1 // status
 #define MISCQ_CELLV_CAL   2 // cell voltage: calibrated
 #define MISCQ_CELLV_ADC   3 // cell voltage: adc counts
 #define MISCQ_TEMP_CAL    4 // temperature sensor: calibrated
 #define MISCQ_TEMP_ADC    5 // temperature sensor: adc counts
 #define MISCQ_DCDC_V      6 // isolated dc-dc converter output voltage
 #define MISCQ_CHGR_V      7 // charger hv voltage
 #define MISCQ_HALL_CAL    8 // Hall sensor: calibrated
 #define MISCQ_HALL_ADC    9 // Hall sensor: adc counts
 #define MISCQ_CELLV_HI   10 // Highest cell voltage
 #define MISCQ_CELLV_LO   11 // Lowest cell voltage
 #define MISCQ_FETBALBITS 12 // FET on/off discharge bits
 #define MISCQ_DUMP_ON	  13 // Turn on Dump FET for no more than ‘payload [3]’ secs
 #define MISCQ_DUMP_OFF	  14 // Turn off Dump FET
 #define MISCQ_HEATER_ON  15 // Enable Heater mode to ‘payload [3] temperature
 #define MISCQ_HEATER_OFF 16 // Turn Heater mode off.
 #define MISCQ_TRICKL_OFF 17 // Turn trickle charger off for no more than ‘payload [3]’ secs
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage

#define NSTRING  2 // Max number of strings in a winch
#define NMODULE  8 // Max number of modules in a string
#define NCELL   18 // Max number of cells   in a module
#define NTHERM   3 // Max number of thermistors in a module

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
	uint8_t flag; // 0 = no reading; 1 = reading received
	uint8_t  num; // Cell number
};

static struct CELLMSG cellmsg[NSTRING][NMODULE][NCELL];
static struct CANRCVBUF cantx;
static uint32_t canid_rx;
static uint32_t canid_tx;
static uint8_t  whom;  // To whom is this addressed?
static uint8_t  reqtype; // Request type (miscq code)
static uint8_t  canseqnumber;
static uint8_t  cellidx;

/* Readings returned determines if string and modules are present. */
static uint8_t yesstring[NSTRING]; // 0 = string number (0-NSTRING) not present
static uint8_t yesmodule[NSTRING][NMODULE]; // 0 = module number (0 - NMODULE) not present
static uint8_t nstring;
static uint8_t nmodule;

uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static int flagnow;  // Timer tick counter
static int flagnext; // Next timer count
static int flagnext1; // Next timer count
static uint8_t timerflag1;

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
	printf(" %08X %01d: ",p->id,p->dlc);
	for (i = 0; i < p->dlc; i++)
		printf ("%02X",p->cd.uc[i]);
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
				"e x  default  (CANID: B0201114)\n\t"
				"emx  aaaaaaaa (CANID: aaaaaaaa) \n\t"
				"eax  (all BMS nodes respond)\n\t"
				"esx  ss  (String number: 1-n)\n\t"
				"emx  aaaaaaaa CANID \n\t"
				"  x = a: ADC cell calibration readings\n\t"
				"  x = h: Trickle charger high voltage\n\t"
				"  x = t; temperatures\n");
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
	cantx.id       = CANID_TX_DEFAULT; // CANID_UNI_BMS_I        0XB0000000 // Default pollster
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

		case 's': // String whom
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
		/* Type of response. */
		reqtype = *(p+2);
		switch (reqtype)
		{
		case 'a': // Cell calibrated readings
			printf("Poll: Cells: calibrated readings\n");
			cantx.cd.uc[1] = MISCQ_CELLV_CAL; //  TYPE2 code
			break;

		case 'A': // Cell ADC accumlation for calibation
			printf("Poll: Cells: ADC Readings\n");
			cantx.cd.uc[1] = MISCQ_CELLV_ADC; //  TYPE2 code
			cellidx = 0;
			break;

		case 't': // Temperature calibrated readings
			printf("Poll: calibrated temperatures\n");
			cantx.cd.uc[1] = MISCQ_TEMP_CAL; //  TYPE2 code
			cellidx = 0;
			break;

		case 'T': // Temperature ADC for calibration
			printf("Poll: thermistor ADC readings\n");
			cantx.cd.uc[1] = MISCQ_TEMP_ADC; //  TYPE2 code
			cellidx = 0;
			break;

		case 's': // BMS measured top-of-stack voltage MISCQ_TOPOFSTACK	
			printf("Poll: BMS measured top-of-stack voltage\n");
			cantx.cd.uc[1] = MISCQ_TOPOFSTACK;  //  TYPE2 code
			break;

		case 'd': // Isolated dc-dc converter output voltage
			printf("Poll: Isolated dc-dc converter output voltage\n");
			cantx.cd.uc[1] = MISCQ_DCDC_V;  //  TYPE2 code
			break;

		default:
			printf("3rd char not recognized: %c\n", *(p+2));
			return -1;
		}
	}

	printf("whom polled: %c",whom);
	
	printf ("\tCANID: BMS %08X  POLLER %08X\n",canid_rx,cantx.id);

	ncell_prev =   0;
	headerctr  = 254;
	flagnext   =  10;
	flagnext1  =  11;
	timerflag1 =   1;
	kaON       =   1;
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
	if ((p->id & 0xfffffffc) != canid_rx)
	{
		return; // Msg is not for us.
	}
//printf("\n%08X %X", p->cd.uc[1]);

	/* Here, CAN msg is for us. */

	// Is it cell readings or "misc"?
	if ( (p->cd.uc[0] != CMD_CMD_TYPE2) ||
		((p->cd.uc[0] == CMD_CMD_TYPE2) && (p->cd.uc[1] == MISCQ_STATUS))) 
	{ // Here, not what we are looking for.
		return;
	}

	// Extract string and module numbers and check for out-of-range
	storeandcheckstringandmodule(p);

	// Handle payload based on request code
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

	flagnow += 1;
	if ((flagnow > flagnext1) && (timerflag1 == 0))
	{
		timerflag1 = 1;
		timer_printresults();
	}
	if (flagnow > flagnext)
	{
		flagnow  = 0;		
		timerflag1 = 0;
		sendcanmsg(&cantx);		
//printf("\nSend flagnow\n");
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

/******************************************************************************
 * static void miscq_cellv_adc(struct CANRCVBUF* p);
 * @brief 	: Request: Cell ADC readings
 * @param	: p = pointer to CANRCVBUF with CAN msg
*******************************************************************************/
/* 
The response CAN msgs are one cell per CAN msg (i.e. 16 msgs per module). ADC count is a float.
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

			flagnext1 =  50; // 1 into flagnext delay
			flagnext  = 100; // 2 sec delay
			timerflag1 = 0;
		}
	return;		
}
/******************************************************************************
 * static void miscq_cellv_cal(struct CANRCVBUF* pcan);
 * @brief 	: Request: Cell calibrated voltages
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
/*
Calibrated voltages are sent as 
*/
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
			cellmsg[nstring][nmodule][idx].d = (extractfloat(&p->cd.uc[4]) * 1E-4);
			cellmsg[nstring][nmodule][idx].flag = 1; // Reset new readings flag

			// Keep reseting until CAN responses end
			flagnext1 =  50; // 1 sec until printout
			flagnext  = 100; // 2 sec cycle
			flagnow   =   0; // Timer flag
		}
		return;
}
/******************************************************************************
 * static void miscq_temp_cal(struct CANRCVBUF* pcan);
 * @brief 	: Request: Thermistor temperatures, calibrated
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
/*
Calibrated voltages are sent as 
*/
static void miscq_temp_cal(struct CANRCVBUF* p)
{
	int idx = (p->cd.uc[3] - 16);
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

			// Keep reseting until CAN responses end
			flagnext1 =  50; // 1 sec until printout
			flagnext  = 100; // 2 sec cycle
			flagnow   =   0; // Timer flag
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
	int idx = (p->cd.uc[3] - 16);
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

			// Keep reseting until CAN responses end
			flagnext1 =  50; // 1 sec until printout
			flagnext  = 100; // 2 sec cycle
			flagnow   =   0; // Timer flag
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

			// Keep reseting until CAN responses end
	flagnext1 =  50; // 1 sec until printout
	flagnext  = 100; // 2 sec cycle
	flagnow   =   0; // Timer flag
	return;
}
/******************************************************************************
 * static void miscq_miscq_dcdc_v(struct CANRCVBUF* p);
 * @brief 	: Request: Isolated DC-DC converter voltager
 * @param	: p = pointer to CANRCVBUF with mesg
*******************************************************************************/
static void miscq_miscq_dcdc_v(struct CANRCVBUF* p)
{
	// Convert payload to float and scale to deg F
	cellmsg[nstring][nmodule][0].d = (extractfloat(&p->cd.uc[4]));
	cellmsg[nstring][nmodule][0].flag = 1; // Reset new readings flag

			// Keep reseting until CAN responses end
	flagnext1 =  50; // 1 sec until printout
	flagnext  = 100; // 2 sec cycle
	flagnow   =   0; // Timer flag
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

		case 's': // BMS measured top-of-stack voltage MISCQ_TOPOFSTACK	
			print_cal_adc("%8.2f",1);
 			break;		

		case 'd': // Isolated dc-dc converter voltage
			print_cal_adc("%8.2f",1);
 			break;		


		default:
			printf("Timer oops: not coded: %c\n",reqtype);
			break;
		}
	return;
}
/******************************************************************************
 * static void print_cal_adc(char* pfmt, uint8_t ncol);
 * @brief 	: Timer: flagnext1: Output accumulated readings
 * @param   : pfmt = pointer to format string
 * @param   : ncol = number of columns (readings) on a line
*******************************************************************************/
static void print_cal_adc(char* pfmt, uint8_t ncol)
{
	int i,j,k; // FORTRAN reminder

	for (i = 0; i < NSTRING; i++)
	{
		// Skip if this string not encountered
		if (yesstring[i] == 0 ) continue;

		/* Column header. */
		headerctr += 1;
		if (headerctr > 16)
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
				{   
					printf(pfmt, cellmsg[i][j][k].d);
 					cellmsg[i][j][k].flag = 0; // Reset new readings flag
				}
			}
			printf("\n");	
		}
	}
	return;
}