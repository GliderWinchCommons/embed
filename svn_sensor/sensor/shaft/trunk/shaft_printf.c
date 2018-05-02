/******************************************************************************
* File Name          : shaft_printf.c
* Date First Issued  : 04/23/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "shaft_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/*

*/
/* **************************************************************************************
 * void shaft_printf(struct SHAFTLC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void shaft_printf(struct SHAFTLC* p)
{
int i = 0;
printf("SHAFT: values: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%2d	%d	   %s\n\r",i+0, (unsigned int)p->size,            "Shaft:  0 Number of elements in the following list");
printf("%2d	%d	   %s\n\r",i+1, (unsigned int)p->crc,             "Shaft:  1 CRC");
printf("%2d	%d	   %s\n\r",i+2, (unsigned int)p->version,         "Shaft:  2 Version number");
printf("%2d	%d	   %s\n\r",i+3, (unsigned int)p->hbct,            "Shaft:  3 Time (ms) between HB msg (speed/count alternate)");
printf("%2d	%d	   %s\n\r",i+4, (unsigned int)p->adc3_htr_initial,"Shaft:  4 High threshold register setting, ADC3");
printf("%2d	%d	   %s\n\r",i+5, (unsigned int)p->adc3_ltr_initial,"Shaft:  5 Low  threshold register setting, ADC3");
printf("%2d	%d	   %s\n\r",i+6, (unsigned int)p->adc2_htr_initial,"Shaft:  6 High threshold register setting, ADC2");
printf("%2d	%d	   %s\n\r",i+7, (unsigned int)p->adc2_ltr_initial,"Shaft:  7 Low  threshold register setting, ADC2");
printf("%2d	0x%08X %s\n\r",i+8, (unsigned int)p->cid_msg_speed,   "Shaft:  8 CANID speed, calibrated, response to poll");
printf("%2d	0x%08X %s\n\r",i+9, (unsigned int)p->cid_hb_speed,    "Shaft:  9 CANID speed, calibrated, heartbeat");
printf("%2d	0x%08X %s\n\r",i+10,(unsigned int)p->cid_msg_ct,      "Shaft: 10 CANID running count, response to poll");
printf("%2d	0x%08X %s\n\r",i+11,(unsigned int)p->cid_hb_ct,       "Shaft: 11 CANID running count, heartbeat");
printf("%2d	%d	   %s\n\r",i+12,(unsigned int)p->num_seg,         "Shaft: 12 Number of segments on code wheel");

printf("%2d	0x%08X %s\n\r",i+13, (unsigned int)p->code_CAN_filt[0], "Shaft: 13 CANID 1 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+14, (unsigned int)p->code_CAN_filt[1], "Shaft: 14 CANID 2 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+15, (unsigned int)p->code_CAN_filt[2], "Shaft: 15 CANID 3 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+16, (unsigned int)p->code_CAN_filt[3], "Shaft: 16 CANID 4 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+17, (unsigned int)p->code_CAN_filt[4], "Shaft: 17 CANID 5 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+18, (unsigned int)p->code_CAN_filt[5], "Shaft: 18 CANID 6 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+19, (unsigned int)p->code_CAN_filt[6], "Shaft: 19 CANID 7 for setting up CAN hardware filter");
printf("%2d	0x%08X %s\n\r",i+20, (unsigned int)p->code_CAN_filt[7], "Shaft: 20 CANID 8 for setting up CAN hardware filter");

printf("%2d	%d	   %s\n\r",i+21, (unsigned int)p->iir_k_hb,			"Shaft: 21 Filter factor: divisor sets time constant: hb msg");
printf("%2d	%d	   %s\n\r",i+22, (unsigned int)p->iir_scale_hb,	   "Shaft: 22 Filter scale : upscaling: hb msg");

	USART1_txint_send(); 
	return;
}
