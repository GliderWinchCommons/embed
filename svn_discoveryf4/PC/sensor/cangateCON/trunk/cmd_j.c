/******************************************************************************
* File Name          : cmd_j.c
* Date First Issued  : 11/13/2020
* Board              : PC
* Description        : Retrieve command msgs for LEVELWIND
*******************************************************************************/
/*
11/13/2020 Hack from cmd_s.c

*/

enum CONTACTOR_CMD_CODES
{
	ADCRAW5V,         // PA0 IN0  - 5V sensor supply
	ADCRAW12V,        // PA7 IN7  - +12 Raw power to board
	ADCSTEPV,			// +96v stepper supply
};
#define NUM36SUBCMDREQ 5	// Number of sub-commands for gen cnd 36

#include "cmd_j.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

static void get_reading (struct CANRCVBUF* p);

#define CMD_GET_READING 36

extern int fdp;	/* port file descriptor */

static u8 canseqnumber = 0;

static struct CANRCVBUF cantx;

#define DEFAULTTX 0xB1000014 //CANID_CMD_LEVELWIND_I1','','GENCMD',1,23,'U8_U8_U8_X4','1 incoming: U8:drum bits,U8:command code,X4:four byte value');
#define DEFAULTRX 0xB1000114 // CANID_CMD_LEVELWIND_R1','','LEVELWIND',1,3,'U8_U8_U8_X4','1: U8:drum bits,U8:command code,X4:four byte value');

static u32 idrx; // CAN id that CAN node sends (to "us)
static u32 idtx; // CAN id that "we" send (to CAN node)

union X4
{
	uint8_t u8[4];
	uint8_t uc[4];
	int32_t s32;
	uint32_t u32;
	float ff;
};

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
printf("TX: %08X %d %2d %2d %2d: ",pcan->id, pcan->dlc, pcan->cd.u8[0],pcan->cd.u8[1],pcan->cd.u8[2]);
	return;
}

/******************************************************************************
 * static union X4 x4pay(u8* pc);
 * @brief 	: move four payload bytes into union
 * @param	: pc = pointer to payload
 * @return	: union
*******************************************************************************/
static union X4 x4pay(u8* pc)
{
	union X4 uif;
	uif.uc[0] = *(pc+0);
	uif.uc[1] = *(pc+1);
	uif.uc[2] = *(pc+2);
	uif.uc[3] = *(pc+3);

	return uif;
}

/******************************************************************************
 * int cmd_j_init(char* p);
 * @brief 	: Start command interrogation sequence
 * @param	: keyboard input
*******************************************************************************/
int cmd_j_init(char* p)
{
	idrx = DEFAULTRX;
	idtx = DEFAULTTX;
	if (strlen(p) > 17)
	{ // Here, use something typed int
	    	sscanf( (p+1), "%x %x",&idrx, &idtx);
	}
  	printf ("ID: CAN node sends to us: %08X  We send to CAN node: %08X\n",idrx,idtx);
	cantx.cd.uc[0] = 1; // ====> Drum #1 <=====
	cantx.cd.uc[1] = CMD_GET_READING;
	cantx.cd.uc[2] = 0;
	cantx.dlc      = 7;
	cantx.id       = idtx;
	sendcanmsg(&cantx);

	return 0;	// End of file
}
/******************************************************************************
 * void cmd_j_do_msg(struct CANRCVBUF* p);
 * @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
void cmd_j_do_msg(struct CANRCVBUF* p)
{
	/* Select CAN msg that is the response to our command. */
	if ((p->id & 0xfffffffc) != idrx) return;

// Debugging
printf("RX %08X %i ",p->id,p->dlc);
int i;
for (i = 0; i < p->dlc; i++) printf(" %02X",p->cd.uc[i]);
printf(":");
	
	/* Drum bits, General command code. */
	printf("%2X %2i: ",p->cd.uc[0],p->cd.uc[1]);

	/* The general command can do many things. */
	switch(p->cd.uc[1])
	{
	case CMD_GET_READING: // Get reading
		get_reading(p);
		break;

	default:
		printf(" pay[1] %3i : general command not programmed\n",p->cd.uc[1]);			
	}

	return;
}
/******************************************************************************
 * static void get_reading (struct CANRCVBUF* p);
 * @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
static void get_reading (struct CANRCVBUF* p)
{
	int i;
	int j;
	union X4 x4;
/*
 0 =  Levelwind switches (uint32_t)
     	payload[3-7] = Port E switches, right justified: (PE14-PE7) >> 7

   1 = CAN bus voltage (float)
   2 = Stepper Controller voltage (float)
   3 = Position accumulator at motor-side limit sw closure
   4 = Position accumulator at not-motor-side limit sw closure
   5 = Position accumulator at center

ManualSw_MSN_NO_Pin GPIO_			PIN_7
#define ManualSw_MS_NO_Pin GPIO_	PIN_8
#define ManualSw_NO_Pin GPIO_		PIN_9
#define LimitSw_MSN_NO_Pin GPIO_	PIN_10
#define LimitSw_MSN_NC_Pin GPIO_	PIN_11
#define LimitSw_MS_NO_Pin GPIO_	PIN_12
#define LimitSw_MS_NC_Pin GPIO_	PIN_13
#define OverrunSwes_NO_Pin GPIO_	PIN_14
*/	
	/* Load pay[3]-[7] into union. */
	x4 = x4pay(&p->cd.uc[3]);
printf(":%2i:",p->cd.uc[2]);
	switch (p->cd.uc[2])
	{
	case 0:
		printf("\n    PE7   PE8   PE9  PE10  PE11  PE12  PE13  PE14\n");
		for (i = 0; i < 8; i++)
		{
			j = (x4.u32 & (1 << i)); if (j != 0) j = 1;	printf("%6i",j);
		}
		printf("\n");
		break;
	case 1:
		printf("%10.1f CAN bus volts\n",x4.ff);
		break;
	case 2:
		printf("%10.1f Stepper volts\n",x4.ff);
		break;
	case 3:
		printf("%10u Pos accum motor side\n",x4.u32);
		break;
	case 4:
		printf("%10u Pos accum motor side not\n",x4.u32);
		break;
	case 5:
		printf("%10u Pos accum center\n",x4.u32);
		break;
	case 8:
		printf("%10.1f ADCINTERNALTEMP, IN17 - Internal temperature sensor\n",x4.ff);
		break;
	case 9:
		printf("%10.3f ADCINTERNALVREF, IN18 - Internal voltage reference\n",x4.ff);
		break;
	default:
		printf("Response sent code for not in list\n");
		break;
	}

	cantx.cd.uc[2] += 1;
	if (cantx.cd.uc[2] >= NUM36SUBCMDREQ)
	{
		cantx.cd.uc[2] = 0;
	}
	else
	{
		sendcanmsg(&cantx);
	}
	return;
}



