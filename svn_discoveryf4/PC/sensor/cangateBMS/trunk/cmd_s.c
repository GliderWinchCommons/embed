/******************************************************************************
* File Name          : cmd_s.c
* Date First Issued  : 08/03/2019
* Board              : PC
* Description        : Retrieve command msgs for CONTACTOR function
*******************************************************************************/
/*
03/10/2015 rev 297 revised s command to send burst of one CAN id based on some other CAN msg poll

*/

enum CONTACTOR_CMD_CODES
{
	ADCRAW5V,         // PA0 IN0  - 5V sensor supply
	ADCRAWCUR1,       // PA5 IN5  - Current sensor: total battery current
	ADCRAWCUR2,       // PA6 IN6  - Current sensor: motor
	ADCRAW12V,        // PA7 IN7  - +12 Raw power to board
	ADCINTERNALTEMP,  // IN17     - Internal temperature sensor
	ADCINTERNALVREF,  // IN18     - Internal voltage reference
	UARTWHV1,  // Battery voltage
	UARTWHV2,  // DMOC +
	UARTWHV3,  // DMOC -
/*	CAL5V,     // 5V supply
	CAL12V,    // CAN raw 12v supply */
};
#define NUMCMDREQ 9	// Number of CONTACTOR_CMD_CODES

#include "cmd_s.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

extern int fdp;	/* port file descriptor */

static u8 canseqnumber = 0;

static struct CANRCVBUF cantx;

#define DEFAULTRX 0xE3600000 // CANID_CMD_CNTCTR1R : U8_VAR: Contactor1: R: Command CAN node response
#define DEFAULTTX 0xE360000C // CANID_CMD_CNTCTR1I : U8_VAR: Contactor1: I: Command CANID incoming to CAN node

static u32 idrx; // CAN id that CAN node sends (to "us)
static u32 idtx; // CAN id that "we" send (to CAN node)

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
printf("TX: %08x %d %2d : ",pcan->id, pcan->dlc, pcan->cd.u8[0]);
	cantx.cd.uc[0] += 1;
	return;
}
/******************************************************************************
 * static int32_t ui16pay(u8* pc);
 * @brief 	: move two payload bytes into a uint32_t
 * @param	: pc = pointer to payload
 * @return	: 
*******************************************************************************/
static int32_t ui16pay(u8* pc)
{
	uint32_t tmp;
	tmp  = (*(pc+0) << 0);
   tmp |= (*(pc+1) << 8);
	return tmp;
}
/******************************************************************************
 * static double gpay(u8* pc);
 * @brief 	: move four payload bytes into a float
 * @param	: pc = pointer to payload
 * @return	: float => double
*******************************************************************************/
static double gpay(u8* pc)
{
	union UIF
	{
		u8 uc[4];
		float f;
	}uif;

	uif.uc[0] = *(pc+0);
	uif.uc[1] = *(pc+1);
	uif.uc[2] = *(pc+2);
	uif.uc[3] = *(pc+3);

	return uif.f;
}
/******************************************************************************
 * int cmd_s_init(char* p);
 * @brief 	: Start command interrogation sequence
 * @param	: keyboard input
*******************************************************************************/
int cmd_s_init(char* p)
{
	idrx = DEFAULTRX;
	idtx = DEFAULTTX;
	if (strlen(p) > 17)
	{ // Here, use something typed int
	    	sscanf( (p+1), "%x %x",&idrx, &idtx);
	}
  	printf ("ID: CAN node sends to us: %08X  We send to CAN node: %08X\n",idrx,idtx);
	cantx.cd.uc[0] = 0; // Command request code
	cantx.dlc      = 1;
	cantx.id       = idtx;
	sendcanmsg(&cantx);

	return 0;	// End of file
}
/******************************************************************************
 * void cmd_s_do_msg(struct CANRCVBUF* p);
 * @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
void cmd_s_do_msg(struct CANRCVBUF* p)
{
	u8* pc;

	if ((p->id & 0xfffffffc) != idrx) return;

printf("RX %08X %i ",p->id,p->dlc);
int i;
for (i = 0; i < p->dlc; i++) printf(" %02X",p->cd.uc[i]);
//printf("\n");
	
	pc = &p->cd.uc[0];
	printf("%2i: ",p->cd.uc[0]);
	switch(p->cd.uc[0])
	{
	case 0:
		printf("%6u %10.2f ADCRAW5V,   PA0 IN0  - 5V sensor supply\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 1:
		printf("%6u %10.1f ADCRAWCUR1, PA2 IN2  - Current sensor: total battery current\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 2:
		printf("%6u %10.1f ADCRAWCUR2, PA4 IN4  - Current sensor: motor\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 3:
		printf("%6u %10.1f ADCRAW12V,  PA7 IN7  - +12 Raw power to board\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 4:
		printf("%6u %10.1f ADCINTERNALTEMP, IN17 - Internal temperature sensor\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 5:
		printf("%6u %10.3f ADCINTERNALVREF, IN18 - Internal voltage reference\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 6:
		printf("%6u %10.2f UARTWHV1, Battery voltage\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 7:
		printf("%6u %10.2f UARTWHV2, DMOC +\n",ui16pay(pc+1),gpay(pc+3));
		break;
	case 8:
		printf("%6u %10.2f UARTWHV3, DMOC -\n",ui16pay(pc+1),gpay(pc+3));
		break;
//	case 9:
//		printf("%6u %10.3f CAL5V, 5V supply\n",ui16pay(pc+1),gpay(pc+3));
//		break;
//	case 10:
//		printf("%6u %10.3f CAL12V, CAN raw 12v supply\n",ui16pay(pc+1),gpay(pc+3));
//		break;
	default:
		printf("Response sent code for not in list\n");
		break;
	}
	if (cantx.cd.uc[0] < NUMCMDREQ)
	{
		sendcanmsg(&cantx);
	}
	else
	{
		cantx.cd.uc[0] = 0;
	}
	return;
}



