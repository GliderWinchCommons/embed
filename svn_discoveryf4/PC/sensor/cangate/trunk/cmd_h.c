/******************************************************************************
* Date First Issued  : 05/05/2018
* Board              : PC
* Description        : histogram readout from shaft sensor
*******************************************************************************/
/*
02-07-2014 rev 193: Poll repsonse for shaft sensor
05/05/2018 Revise to match revised '../sensor/trunk/shaft.c'

*/
#include "cmd_h.h"
#include "cmd_q.h"
#include "USB_PC_gateway.h"

/* Subroutine prototypes */
static void cmd_h_sendmsg(struct CANRCVBUF* p);

/* From cangate */
extern FILE *fpList;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file

static struct CANRCVBUF canmsg;
static u8 canseqnumber = 0;
static struct PCTOGATEWAY pctogateway; 

#define CMD_REQ_HISTOGRAM 40
#define CMD_THISIS_HISTODATA 41

/* Default values */
#define RESPONSE_ID 0xE2C00000 // 05/05/2018 CANID_UNIT_2:CANID_CMD_SHAFT1I
#define REQUEST_ID  0xA0600000 // 05/05/2018 CANID_UNIT_2:CANID_CMD_SHAFT1R
#define ADCNUM 4			// This must be 2,3, or 4 (both)
#define SWCT 1				// Number of consecutive histograms sent
#define DUR  10000		// Number of DMA buffers used in histogram
static int otosw = 0;	// One time default

/* Keyboard entered */
static u32 response_id;	// CAN id: response with histogram data
static u32 keybrd_id;	// CAN id: request for historgram data
static u32 adcnum;
static u32 swct;
static u32 dur;


static void printerror(char *p)
{
	printf("%s",p);
	printf("h<enter> error, needs to be one of the following\n");
	printf("hr xxxxxxxx [where x is Command CAN id for response\n");
	printf("hx<enter> repeats previous h command\n");
	printf("h zzzzzzzz a b ccccc\n");
	printf("  zzzzzzzz = Command CAN id (suffix I)\n");
	printf("  a = ADC number: 2, 3 or 4 for both\n");
	printf("  b = Number of consecutive histograms sent (e.g. 1)\n");
	printf("  ccccc = Number of DMA buffers accumulated (e.g. 10000 approx 1 sec)\n");
	return;
}
static void printcommand(char *p)
{
	printf("%s",p);
	printf("Command to request histogram to be sent:\n");
	printf("  CAN id  adc sw duration\tCAN id response\n");
	printf("0x%0X %2d %2d %d\t\t0x%08X\n",keybrd_id,adcnum,swct,dur,response_id);
	return;
}

/******************************************************************************
 * static void canmsg_load(struct CANRCVBUF* pcan);
 * @brief 	: One-Time-Only populate CAN msgs with defaults
*******************************************************************************/
static void canmsg_default(void)
{
	if (otosw != 0) return;
	
	keybrd_id = REQUEST_ID;
	adcnum = ADCNUM;
	swct = SWCT;
	dur = DUR;	
	response_id = RESPONSE_ID;
	otosw = 1;
	return;
}
/******************************************************************************
 * static void send_command(void);
 * @brief 	: Send command
*******************************************************************************/
	union U32B
	{
		u32 ui;
		uint8_t b[4];
	}u32b;

static void send_command(struct CANRCVBUF* pcan)
{
	u32b.ui = dur; // Convert int to bytes

	// Load CAN msg that requests histogram 
	pcan->id = keybrd_id;
	pcan->dlc = 7;
	pcan->cd.uc[0] = CMD_REQ_HISTOGRAM;
	pcan->cd.uc[1] = adcnum;
	pcan->cd.uc[2] = swct;
	pcan->cd.uc[3] = u32b.b[0];
	pcan->cd.uc[4] = u32b.b[1];
	pcan->cd.uc[5] = u32b.b[2];
	pcan->cd.uc[6] = u32b.b[3];

	cmd_h_sendmsg(&canmsg);
}
/******************************************************************************
 * int cmd_h_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_h_init(char* p)
{
	canmsg_default(); // oto, set up defaults

	if (strlen(p) < 3)
	{ // Here too few chars
		printerror("Too few chars\n");
		return -1;
	}

	if (strlen(p) > 2) 
	{
		switch (*(p+1))
		{
		case 'r': // Send what was previously entered
			printcommand("Repeat command\n");
			send_command(&canmsg);
			break;

		case 'x': // Enter response CAN ID
			sscanf((p+2),"%x",&response_id);
			printf("Response CAN id: 0x%08X\n",response_id);
			break;

		case ' ': // Request 			
			if (strlen(p) < 9)
			{
				printerror("Not enough chars for a valid command\n");
				return -1;
			}
			/* Here, enough chars for a command */
			sscanf((p+1), "%x %d %d %d",&keybrd_id,&adcnum,&swct,&dur);
			printcommand("Use this command\n");
			send_command(&canmsg);
			break;
		}
	}
	return 0;
}

/******************************************************************************
 * void cmd_h_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'h' command
 * @param	: 
*******************************************************************************/

void cmd_h_do_msg(struct CANRCVBUF* p)
{
	union U32B ub;

		if ((p->id & ~0x3) == (response_id & ~0x3)) 
		{
			ub.b[0] = p->cd.uc[3];
			ub.b[1] = p->cd.uc[4];
			ub.b[2] = p->cd.uc[5];
			ub.b[3] = p->cd.uc[6];

			printf("# %2d %1d %3d %5u %15.0f %8u\n",p->cd.uc[0],p->cd.uc[1],p->cd.uc[2],p->cd.uc[2]*64,(double)ub.ui/15000,ub.ui);
		}
  return;
}
/******************************************************************************
 * static void cmd_h_sendmsg(struct CANRCVBUF* p);
 * @brief 	: Send the msg
 * @param	: p = Pointer struct with msg
*******************************************************************************/
static void cmd_h_sendmsg(struct CANRCVBUF* p)
{
	pctogateway.mode_link = MODE_LINK;		// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;		// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, p); 	// Send to file descriptor (e.g. serial port)

	return;
}

