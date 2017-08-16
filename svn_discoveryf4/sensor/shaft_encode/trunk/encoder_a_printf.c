/******************************************************************************
* File Name          : encoder_a_printf.c
* Date First Issued  : 08/08/2017
* Board              : Discovery F4
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "encoder_a_printf.h"
#include "../../../sw_discoveryf4/trunk/lib/libusartstm32f4/bsp_uart.h"

#define UXPRT	3	// Uart number for 'xprintf' messages
#include "xprintf.h"

/* **************************************************************************************
 * void encoder_a_printf(struct ENCODERALC* penc);
 * @brief	: Print the values
 * @param	: penc = pointer to struct with the values 
 * ************************************************************************************** */
/*
 struct ENCODERALC
 {
	uint32_t size;			// 0 Number of items in struct
 	uint32_t crc;			// 1 crc-32 placed by loader
	uint32_t version;		// 2 struct version number
	uint32_t hbct;			// 3 Heartbeat ct: time (ms) between msgs
	uint32_t ctperrev;		// 4 Number of counts per revolution
	float	distperrev;		// 5 Distance per revolution (meters)
	uint32_t cid_heartbeat;		// 6 CANID-Heartbeat msg 
	uint32_t cid_poll_msg;		// 7 CANID-Fully calibrated encoder msg
	uint32_t cid_poll;		// 8 CANID-Response to poll (??)
	uint32_t cid_poll_r;		// 9 CANID-Response to poll (??)
	// CAN ID's for setting up hw filter
	uint32_t cid_hb_timesync;	//10 CANID-GPS time sync msg polls encoders
	uint32_t cid_msg_time_poll;	//11
	uint32_t cid_tst_enc_a;		//12 
	uint32_t cid_cmd_hi;		//13 CANID-Command incoming/interrogation
	uint32_t code_CAN_filt[4];	//14 Remaining unassigned in list (CANID_DUMMY)
 };
*/
void encoder_a_printf(struct ENCODERALC* penc)
{
int i = 0;
xprintf(UXPRT,"ENCODER_a: values: pointer = %08X\n\r",(int)penc);
xprintf(UXPRT,"%2d	%d	%s\n\r",   i++, (unsigned int)penc->size,            "  0 Number of elements in the following list");
xprintf(UXPRT,"%2d	%d	%s\n\r",   i++, (unsigned int)penc->crc,             "  1 Tension_1: CRC for tension list");
xprintf(UXPRT,"%2d	%d	%s\n\r",   i++, (unsigned int)penc->version,         "  2 Version number");
xprintf(UXPRT,"%2d	%d	%s\n\r",   i++, (unsigned int)penc->hbct,            "  3 Heartbeat ct: time (ms) between msgs");
xprintf(UXPRT,"%2d	%d	%s\n\r",   i++, (unsigned int)penc->ctperrev,        "  4 Number of counts per revolution");
xprintf(UXPRT,"%2d	%12.4f	%s\n\r",   i++,       (double)penc->distperrev,      "  5 Distance per revolution (meters)");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_heartbeat,   "  6 CANID-Heartbeat raw (int64_t) running encoder count");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_poll_msg,    "  7 CANID-Fully calibrated encoder msg");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_poll,        "  8 CANID-Response to poll ");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_poll_r,      "  9 CANID-Response to poll ");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_hb_timesync, " 10 CANID-GPS time sync msg polls encoders");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_msg_time_poll," 11 CANID-cid_msg_time_poll");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_tst_enc_a,   " 12 CANID-cid_tst_enc_a");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->cid_cmd_hi,      " 13 CANID-Command incoming/interrogation");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->code_CAN_filt[0]," 14 Remaining unassigned in list (CANID_DUMMY)");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->code_CAN_filt[1]," 15 Remaining unassigned in list (CANID_DUMMY)");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->code_CAN_filt[2]," 16 Remaining unassigned in list (CANID_DUMMY)");
xprintf(UXPRT,"%2d	%08X	%s\n\r",   i++, (unsigned int)penc->code_CAN_filt[3]," 17 Remaining unassigned in list (CANID_DUMMY)");

	return;
}
