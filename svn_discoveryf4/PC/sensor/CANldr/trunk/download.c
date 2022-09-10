/******************************************************************************
* File Name          : download.c
* Date First Issued  : 09/09/2022
* Board              : PC
* Description        : CAN download 
*******************************************************************************/

#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "../../../../../svn_common/trunk/db/gen_db.h"

/* CAN node loader CAN ID passed in from command line */
extern uint32_t CANnodeid;

/* File pointer with extended binary to be sent. */
extern FILE *fpXbin;
extern int fdp;	/* port file descriptor */


static struct CANRCVBUF cantx;
static u8 canseqnumber = 0;

/******************************************************************************
 * static void loadpay(uint8_t* po, uint32_t data);
 * @brief 	: Load payload bytes
 * @param   : po = pointer to payload 
 * @param   : data = four byte word to load
*******************************************************************************/
static void loadpay(uint8_t* po, uint32_t data)
{
	*(po+0) = data >>  0;
	*(po+1) = data >>  8;
	*(po+2) = data >> 16;
	*(po+3) = data >> 24;
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
	pctogateway.cmprs.seq = canseqnumber;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
printf("TX:%3d %08X %d: ",canseqnumber,pcan->id, pcan->dlc);
for (int i = 0; i < pcan->dlc; i++)printf(" %02X", pcan->cd.u8[i]);
printf("\n");
	canseqnumber += 1;
	return;
}
/******************************************************************************
 * static void send_squelch(void);
 * @brief 	: 
*******************************************************************************/
static void send_squelch(void)
{
	cantx.id       = CANID_UNI_BMS_PC_I;
	cantx.dlc      = 8;
	cantx.cd.uc[0] = LDR_SQUELCH;
	loadpay(&cantx.cd.uc[4], 10000); // Send ms of time delay
	sendcanmsg(&cantx);
	return;
}
/******************************************************************************
 * void download_init(void);
 * @brief 	: Reset 
*******************************************************************************/
void download_init(void)
{

}
/******************************************************************************
 * void download_canbus_msg(struct CANRCVBUF* p);
 * @brief 	: Incoming CAN msgs
 * param    : p = pointer to CAN msg struct
*******************************************************************************/
void download_canbus_msg(struct CANRCVBUF* p)
{
	union UIF
	{
		uint32_t ui;
		float f;
	};

	if ((p->id & 0xfffffffc) != CANnodeid) return;

printf("0x%08X %d\n",p->id,p->dlc);
	send_squelch();


	return;
}