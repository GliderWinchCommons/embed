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

static float extractfloat(uint8_t* puc);

static void cmd_e_timerthread(void);
static int starttimer(void);


#define CANID_RX_DEFAULT CANID_MSG_BMS_CELLV12R // 0xB0201124 // Default BMS
#define CANID_TX_DEFAULT CANID_UNI_BMS_I        // 0XB0000000 // Default pollster
#define NCELL 18 // Max number of cells 

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

union UF
{
	uint8_t uc[4];
	float f;
};

struct CELLMSG
{
	double     d; // u32 converted to double
	uint32_t u32; // reading
	uint8_t flag; // 0 = no reading; 1 = reading received
	uint8_t  num; // Cell number
};

static struct CELLMSG cellmsg[NCELL];
static struct CANRCVBUF cantx;
static uint32_t canid_rx;
static uint32_t canid_tx;
static uint8_t  whom;  // To whom is this addressed?
static uint8_t  reqtype; // Request type (miscq code)
static uint8_t  canseqnumber;
static uint8_t  cellidx;

uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.
static int flagnow;  // Timer tick counter
static int flagnext; // Next timer count

static uint8_t ncell_prev;
static uint8_t headerctr;

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
		case 'a': // Cell ADC accumlation for calibation
			printf("Poll: CELLVOLTAGE ADC Readings\n");
			for (int i = 0; i < NCELL; i++) 
				cellmsg[i].flag = 0;
			cantx.cd.uc[1] = MISCQ_CELLV_ADC;
			cellidx = 0;
			cantx.cd.uc[3] = cellidx; // Initial cell number
			break;

		default:
			printf("3rd char not recognized: %c\n", *(p+2));
			return -1;
		}
	}

	printf("whom polled: %c",whom);
	
	printf ("\tCANID: BMS %08X  POLLER %08X\n",canid_rx,cantx.id);

	ncell_prev = 0;
	headerctr = 254;
	flagnext = 10;
	kaON = 1;
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
	uint8_t i;
	uint8_t idx;

	if ((p->id & 0xfffffffc) != canid_rx)
	{
		return; // Msg is not for us.
	}
//printf("\n%08X %X", p->cd.uc[1]);

	/* Here, CAN msg is for us. */
	// Is it cell readings or "misc"?
	if ((p->cd.uc[0] != CMD_CMD_TYPE2) ||
	   ((p->cd.uc[0] == CMD_CMD_TYPE2) && (p->cd.uc[1] == MISCQ_STATUS)))
	{ // Here, not what we are looking for.
		return;
	}
	// 
	switch (p->cd.uc[1])
	{
	case MISCQ_CELLV_ADC: // 'a' 
		idx = p->cd.uc[3];
		if (idx == 0)
		{ // Index (cell number) stepped downward
			// Header 
			headerctr += 1;
			if (headerctr >= 32)
			{
				headerctr = 0;	
				for (i = 0; i < NCELL; i++)
					printf("      %2d",i+1);
				printf("\n");
			}
			// Output accumulated readings
			for (i = 0; i < NCELL; i++)
			{
				if (cellmsg[i].flag == 0)							
				{
					printf(" .......");
				}
				else
				{
					printf("%8.1f", cellmsg[cellidx].d);
				}
				cellmsg[i].flag = 0; // Reset new readings flag
			}
			printf("\n");			
		}

		if (idx >= NCELL)
		{ // Here, index is too high.
			printf("Cell index too high: %d\n",p->cd.uc[3]);
			break;
		}
		else
		{
			cellmsg[idx].d = extractfloat(&p->cd.uc[4]);
			cellmsg[idx].flag = 1; // Reset new readings flag
//printf("%08X %1d: ",p->id,p->dlc);
//for (int j = 0; j < p->dlc; j++)printf(" %02X",p->cd.uc[j]);
//printf(" %2d %f\n",p->cd.uc[3],cellmsg[idx].d);
		}
		flagnext = 100; // 2 sec delay
		break;

	default:
		printf("TYPE2 command code error: expected %d (MISCQ_CELLV_ADC); Got %d\n",MISCQ_CELLV_ADC, p->cd.uc[1]);
		break;
	}
	ncell_prev = p->cd.uc[3];
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
	if (flagnow > flagnext)
	{
		flagnow  = 0;		
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

