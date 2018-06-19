/******************************************************************************
* File Name          : pwrbox_printf.c
* Date First Issued  : 05/24/2015, 06/13/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "pwrbox_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/* **************************************************************************************
 * void tension_a_printf(struct TENSIONLC* ppwr);
 * @brief	: Print the values
 * @param	: ppwr = pointer to struct with the values 
 * ************************************************************************************** */
void pwrbox_printf(struct PWRBOXLC* ppwr)
{
int i = 0;
printf("PWRBOX: values: pointer = %08X\n\r",(int)ppwr);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%2d	%d	%s\n\r",i+ 0, (unsigned int)ppwr->size,     "  0 Number of elements in the following list");
printf("%2d	%d	%s\n\r",i+ 1, (unsigned int)ppwr->crc,      "  1 Pwrbox: CRC");
printf("%2d	%d	%s\n\r",i+ 2, (unsigned int)ppwr->version,  "  2 Pwrbox: Version number");
printf("%2d	%d	%s\n\r",i+ 3, (unsigned int)ppwr->hbct,     "  3 Pwrbox: Heartbeat ct: ticks between sending msgs");

fmtprint(i+ 4, ppwr->adc[0].offset*10000, " 4 Pwrbox: ADC 0 offset*10000");
fmtprint(i+ 5, ppwr->adc[0].scale*10000,  " 5 Pwrbox: ADC 0 scale*10000");
fmtprint(i+ 6, ppwr->adc[1].offset*10000, " 6 Pwrbox: ADC 1 offset*10000");
fmtprint(i+ 7, ppwr->adc[1].scale*10000,  " 7 Pwrbox: ADC 1 scale*10000");
fmtprint(i+ 8, ppwr->adc[2].offset*10000, " 8 Pwrbox: ADC 2 offset*10000");
fmtprint(i+ 9, ppwr->adc[2].scale*10000,  " 9 Pwrbox: ADC 2 scale*10000");
fmtprint(i+10, ppwr->adc[3].offset*10000, "10 Pwrbox: ADC 3 offset*10000");
fmtprint(i+11, ppwr->adc[3].scale*10000,  "11 Pwrbox: ADC 3 scale*10000");
fmtprint(i+12, ppwr->adc[4].offset*10000, "12 Pwrbox: ADC 4 offset*10000");
fmtprint(i+13, ppwr->adc[4].scale*10000,  "13 Pwrbox: ADC 4 scale*10000");
fmtprint(i+14, ppwr->adc[5].offset*10000, "14 Pwrbox: ADC 5 offset*10000");
fmtprint(i+15, ppwr->adc[5].scale*10000,  "15 Pwrbox: ADC 5 scale*10000");
fmtprint(i+16, ppwr->adc[6].offset*10000, "16 Pwrbox: ADC 6 offset*10000");
fmtprint(i+17, ppwr->adc[6].scale*10000,  "17 Pwrbox: ADC 6 scale*10000");
fmtprint(i+18, ppwr->adc[7].offset*10000, "18 Pwrbox: ADC 7 offset*10000");
fmtprint(i+19, ppwr->adc[7].scale*10000,  "19 Pwrbox: ADC 7 scale*10000");

fmtprint(i+20, ppwr->iir[0].k,     "20 Pwrbox: filter 0 factor k");
fmtprint(i+21, ppwr->iir[0].scale, "21 Pwrbox: filter 0 scale");
fmtprint(i+22, ppwr->iir[1].k,     "22 Pwrbox: filter 1 factor k");
fmtprint(i+23, ppwr->iir[1].scale, "23 Pwrbox: filter 1 scale");
fmtprint(i+24, ppwr->iir[2].k,     "24 Pwrbox: filter 2 factor k");
fmtprint(i+25, ppwr->iir[2].scale, "25 Pwrbox: filter 2 scale");
fmtprint(i+26, ppwr->iir[3].k,     "26 Pwrbox: filter 3 factor k");
fmtprint(i+27, ppwr->iir[3].scale, "27 Pwrbox: filter 3 scale");

printf("%2d	0x%08X	%s\n\r",i+28, (unsigned int)ppwr->cid_heartbeat,"28 Pwrbox: CANID: heartbeat");
printf("%2d	0x%08X	%s\n\r",i+29, (unsigned int)ppwr->cid_pwr_msg,  "29 Pwrbox: CANID: poll msg");
printf("%2d	0x%08X	%s\n\r",i+30, (unsigned int)ppwr->cid_pwr_alarm,"30 Pwrbox: CANID: alarm");

printf("%2d	%d	%s\n\r",i+ 3, (unsigned int)ppwr->alarm_rate,  "31 Pwrbox: Time between alarm msgs (ms)");
fmtprint(i+32, ppwr->alarm_thres, "32 Pwrbox: alarm msg threshold voltage");

printf("%2d	0x%08X	%s\n\r",i+44, (unsigned int)ppwr->code_CAN_filt[0], " 33 CANID_HB_TIMESYNC");
printf("%2d	0x%08X	%s\n\r",i+45, (unsigned int)ppwr->code_CAN_filt[1], " 34 CANID_MSG_TIME_POLL");
printf("%2d	0x%08X	%s\n\r",i+46, (unsigned int)ppwr->code_CAN_filt[2], " 35 Dummy");
printf("%2d	0x%08X	%s\n\r",i+47, (unsigned int)ppwr->code_CAN_filt[3], " 36 Dummy");
printf("%2d	0x%08X	%s\n\r",i+48, (unsigned int)ppwr->code_CAN_filt[4], " 37 Dummy");
printf("%2d	0x%08X	%s\n\r",i+49, (unsigned int)ppwr->code_CAN_filt[5], " 38 Dummy");
printf("%2d	0x%08X	%s\n\r",i+50, (unsigned int)ppwr->code_CAN_filt[6], " 39 Dummy");
printf("%2d	0x%08X	%s\n\r",i+51, (unsigned int)ppwr->code_CAN_filt[7], " 40 Dummy");
	USART1_txint_send(); 
	return;
}

