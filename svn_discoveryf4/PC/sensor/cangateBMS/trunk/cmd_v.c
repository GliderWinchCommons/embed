/******************************************************************************
* File Name          : cmd_v.h
* Date First Issued  : 08/01/2024
* Board              : PC
* Description        : BMS version check using CRC and CHKSUM
*******************************************************************************/
/*
*/

#include "cmd_v.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "timer_thread.h" // Timer thread for sending keep-alive CAN msgs
#include "../../../../../svn_common/trunk/db/gen_db.h"

extern int fdp;	/* port file descriptor */

static void sendcanmsg(struct CANRCVBUF* pcan);
static void cmd_v_timerthread(void);
static int starttimer(void);
static void comparcrcchk(void);

/*  From: GliderWinchCommons/embed/svn_common/trunk/db/CANID_INSERT.sql
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds; */
#define CANID_RX_DEFAULT CANID_UNIT_BMS03 // B0A00000','UNIT_BMS03', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #01
#define CANID_TX_DEFAULT CANID_UNI_BMS_PC_I //CANID_UNI_BMS_PC_I        // AEC00000 //BMSV1 UNIversal From PC,  Incoming msg to BMS: X4=target CANID // Default pollster

/* See GliderWinchItems/BMS/adbms1818/Ourtasks/cancomm_items.h for latest version. */
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
 #define MISCQ_FAN_SET_SPD  40 // Set fan: pct (0 - 100)
 #define MISCQ_PROG_CRC     41 // Retrieve installed program's: CRC
 #define MISCQ_PROG_CHKSUM  42 // Retrieve installed program's: Checksum
 #define MISCQ_PROG_CRCCHK  43 // Retrieve for both 41 and 42 (two msgs)

struct CRCCHK
{
	uint32_t id;     // CAN id
	uint32_t crc;    // CRC
	uint32_t chk;    // CHKSUM
	uint8_t flgcrc;  // 0 = crc not received
	uint8_t flgchk;  // 0 = chk not received
};
#define CRCCHKSZ 16 // Max size of BMS nodes
// Working struct
static struct CRCCHK crcchk[CRCCHKSZ];
static uint8_t idxcrcchk;  // Index of next available chcchk[]
// Saved struct for later 'vc' command to compare to params/*.txt files
static struct CRCCHK crcchkx[CRCCHKSZ];
static uint8_t idxcrcchkx;  // Index of next available chcchk[]
// Sort the CRCCHKSZ struct array
uint32_t bubble_sort_crcchk(struct CRCCHK *a, uint32_t n);




static struct CANRCVBUF cantx;
static uint32_t canid_rx;
//static uint32_t canid_tx; // POLLer CAN ID (is one who "polls")

static uint32_t canid_whoresp = CANID_RX_DEFAULT; // Who responds?
static uint32_t candid_poller = CANID_TX_DEFAULT; // Default pollster

static uint8_t canseqnumber;
static uint8_t cd_uc1 = (0x3 << 6); // Who responds: canid_tx.cd.uc[1] default: all CAN IDs

uint32_t kaON2;  // 0 = keep-alive is off; 1 = keep-alive = on.
static uint32_t timerctr;
static uint32_t timernext; // Next timer count

// zero duration = one-time only poll.
#define DEFAULT_POLLDUR 2000 // Duration in ms
static uint32_t polldur = DEFAULT_POLLDUR; // Duration between polls

static uint8_t cmdswflg; // Subcommand
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
	printf("BMS %08X  POLLer %08X\n\twho responds code: 0x%02X %s\n",canid_whoresp, cantx.id, cd_uc1, puc1[(cd_uc1 >> 6)&0x3] );
	printf("Poll duration: %d (ms)\n",polldur);
	return;
}

/******************************************************************************
 * int cmd_v_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
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

int cmd_v_init(char* p)
{
	uint32_t len = strlen(p);

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

	cantx.cd.uc[2] = MISCQ_PROG_CRCCHK;

	cmdswflg = 0;

	if (len < 2)
	{
		printfsettings();
		printf("Too few chars: %d\n",len);
		return -1;
	}
	else
	{	
		/* POLLER requests BMS node, string, or all. */
/* 2nd char commands that do not have an 'x' return; others 'break' */
		switch ( *(p+1) )
		{ 
		case 'l': // Loader check
			printf("Loader check  not implemented\n");
			cmdswflg = 'l';
			return -1;

		case 'b': // BMS retrieve CRC & CHKSUM (for all nodes)
		{
			printf("BMS CRC CHKSUM check\n");
			idxcrcchk = 0; // Start at beginning
			for (int k = 0; k < CRCCHKSZ; k++)
			{
				crcchk[k].flgcrc = 0;
				crcchk[k].flgchk = 0;
			}
			cmdswflg = 'b';
			break;
		}	

		case 'c': // Compare CRC & CHKSUM on file with results of 'vb' command
			comparcrcchk();
			return 0;

		case 'p': // 'vp' Set POLL'er CAN ID
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
			printf("New value set is: %d (ms)\n",polldur);
			printfsettings();
			return 0;

		case 'h': // 'vh' Help
			printfsettings();
			return 0;					

		default:
			printf("2nd char not recognized: %c\n", *(p+1));
			canid_rx = 0;
			return -1;
		}
		printf ("\tCANID: BMS %08X  POLLER %08X\n",canid_rx,cantx.id);

		/* All but, 'x' == 'a' are CMD_CMD_TYPE2, so load [0] here. */
		cantx.cd.uc[0] = CMD_CMD_TYPE2; //  TYPE2 payload format 

	}
	printf("Poll duration (ms) is: %d\n",polldur);
	kaON2       =   1;
	timerctr   = 0;
	timernext  = polldur/10;

	sendcanmsg(&cantx);	// Send first request
	if (polldur != 0)
		starttimer(); // Start timer for repetitive requests
	return 0;
}

/******************************************************************************
 * void cmd_v_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_v_do_msg(struct CANRCVBUF* p)
{
		int i;
//printf("\n%08X %d: ", p->id,p->dlc);for (i=0; i<p->dlc; i++) printf(" %02X",p->cd.uc[i]);
	uint32_t utmp = (p->id & 0xfffffffc);
	if ((utmp < (uint32_t)CANID_UNIT_BMS01) || (utmp > (uint32_t)CANID_UNIT_BMS18))
		return; // CAN ID is not a BMS module function.

	/* Here, CAN msg is from a BMS module. */
	/* Check if this CAN id is in the table. */
	for (i = 0; i < idxcrcchk; i++)
	{
		if (crcchk[i].id == utmp)
		{ // Found
			break;
		}
	}
	if (i >= idxcrcchk)
	{ // Add to table
		crcchk[i].id = utmp;
		idxcrcchk += 1;
		if (idxcrcchk >= CRCCHKSZ)
		{
			printf("ER: Too many BMS nodes: %d\n",CRCCHKSZ);
			return;
		}
	}

	if (cmdswflg == 'b')
	{
		/* Check if payload has CRC or CHKSUM */
		if (p->cd.uc[0] == CMD_CMD_MISCPC) 
		{ // Here, not what we are looking for.
			if (p->cd.uc[1] == MISCQ_PROG_CRC)
			{
				 crcchk[i].crc = p->cd.ui[1];
				 crcchk[i].flgcrc = 1;
			}
			if (p->cd.uc[1] == MISCQ_PROG_CHKSUM)
			{
				 crcchk[i].chk = p->cd.ui[1];
				 crcchk[i].flgchk = 1;
			}
			return;
		}		
	}
	return;
}
/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
uint32_t www2;
static void sendcanmsg(struct CANRCVBUF* pcan)
{
//int i;printf("%08X %d: ", pcan->id,pcan->dlc);for (i=0; i<pcan->dlc; i++) printf(" %02X",pcan->cd.uc[i]);printf("\n");

	struct PCTOGATEWAY pctogateway; 
//	pcan->id = canid_tx;
//	pcan->dlc = 1;

	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
//printcanmsg(pcan);	printf(" %4d\n",www2++);
	return;
}
/******************************************************************************
 * static void timer_printresults(void);
 * @brief 	: 
*******************************************************************************/	
static void timer_printresults(void)
{
	if (idxcrcchk == 0)
	{
		printf("\nNo BMS CAN msgs received\n");
		return;
	}
	if (cmdswflg =='b')
	{
		if (idxcrcchk > 0)
		{
			bubble_sort_crcchk(crcchk, idxcrcchk);
		}
		for (int k = 0; k < idxcrcchk; k++)
		{
			uint8_t ff = (crcchk[k].flgcrc << 1) | crcchk[k].flgchk;
			switch(ff)
			{
			case 3:
				printf("%08X  %08X  %08X\n",crcchk[k].id, crcchk[k].crc, crcchk[k].chk);
				break;
			case 2:
				printf("%08X  %08X  --------\n",crcchk[k].id, crcchk[k].crc);
				break;
			case 1:
				printf("%08X  --------  %08X\n",crcchk[k].id, crcchk[k].chk);
				break;
			case 0:
				printf("%08X  --------  --------\n",crcchk[k].id);
				break;
			default:
				printf("Ooops\n");
				break;
			}
		}
	}
	return;
}
/******************************************************************************
 * static void cmd_v_timerthread(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/	
static void cmd_v_timerthread(void)
{
	if (polldur == 0)
		return;

	if (kaON2 == 0) return; // No timer generated msgs

	timerctr += 1; // 10 ms tick counter
	if ((int)(timerctr - timernext) > 0)
	{ // Time to output accumulated readings
		timernext += polldur/10;
		timer_printresults();
		printf("\n");

		// Update copy of struct for later comparison if 'vc' commanded
		for (int k = 0; k < idxcrcchk; k++)
			crcchkx[k] = crcchk[k];
		idxcrcchkx = idxcrcchk;

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
	int ret = timer_thread_init(&cmd_v_timerthread, 10000); // 10 ms
	if (ret != 0)
	{
		printf("\ncmd_v: timer thread startup unsucessful\n");
		return -1;
	}
	return 0;
}
/* **************************************************************************************
 * uint32_t bubble_sort_crcchk(struct CRCCHK *a, uint32_t n) ;
 * @brief   : Convert CAN id from ascii/hex to unint32_t
 * @param   : pc = pointer to CRCCHK array
 * @param   " n  = size of array"
 * @return  : number of compares
 * ************************************************************************************** */
uint32_t bubble_sort_crcchk(struct CRCCHK *a, uint32_t n) 
{
	uint32_t cmpct = 0;
    int i, j = n, s = 1;
    struct CRCCHK t;
    while (s) {
        s = 0;
        for (i = 1; i < j; i++) 
        {
            if (a[i].id < a[i - 1].id) 
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
/* **************************************************************************************
 * static void comparcrcchk(void);
 * @brief   : Compare results of previous 'vb' with CRC & CHKSUM on file
 * ************************************************************************************** */
struct CMPX
{
	uint32_t id;     // CAN id
	uint32_t crc;    // CRC
	uint32_t chk;    // CHKSUM
	time_t t;
};
static struct CMPX cmpx;

FILE* fTXT;
static void comparcrcchk(void)
{
	char s[128];
	char d[64];
	char line[256];

	printf("Compare previous vb command results with CRC & CHKSUM on file\n");
	if (idxcrcchkx == 0)
	{
		printf("No entries from a vb command were saved\nRun a vb command\n ");
		return;
	}
printf("Number entries: %d\n",idxcrcchkx);
	for (int k = 0; k < idxcrcchkx; k++)
	{
		strcpy(s,"../../../../../../../GliderWinchItems/BMS/bmsadbms1818/params/");
		sprintf(d,"%08X",crcchkx[k].id);
		strcat(s,d);
		strcat(s, "-crcchk.txt");		
		if ((fTXT = fopen(s,"r")) != NULL)
		{ // Here, file found in: ~/GliderWinchItems/BMS/*1818/params/
			while ((fgets(line,128,fTXT)) > 0)
			{
//printf("%s\n",line);
				sscanf(line,"%x %x %x %lx",&cmpx.id,&cmpx.crc,&cmpx.chk,&cmpx.t);
printf("%08X  %08X  %08X %lX\n",cmpx.id, cmpx.crc, cmpx.chk,cmpx.t);
			}
			printf("\n");
		}
		else
		{
			printf("No crcchk.txt file: %08X\n",crcchkx[k].id);
		}
	}
	return;
}
