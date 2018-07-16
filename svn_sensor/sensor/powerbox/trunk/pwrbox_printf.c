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
printf("%2d	%d	%s\n\r",i+ 1, (unsigned int)ppwr->crc,      "  1 CRC");
printf("%2d	%d	%s\n\r",i+ 2, (unsigned int)ppwr->version,  "  2 Version number");
printf("%2d	%d	%s\n\r",i+ 3, (unsigned int)ppwr->hbct,     "  3 Heartbeat ct: ticks between sending msgs");

fmtprint(i+ 4, ppwr->adc[0].offset*10000, " 4 ADC 0 offset*10000");
fmtprint(i+ 5, ppwr->adc[0].scale*10000,  " 5 ADC 0 scale*10000");
fmtprint(i+ 6, ppwr->adc[1].offset*10000, " 6 ADC 1 offset*10000");
fmtprint(i+ 7, ppwr->adc[1].scale*10000,  " 7 ADC 1 scale*10000");
fmtprint(i+ 8, ppwr->adc[2].offset*10000, " 8 ADC 2 offset*10000");
fmtprint(i+ 9, ppwr->adc[2].scale*10000,  " 9 ADC 2 scale*10000");
fmtprint(i+10, ppwr->adc[3].offset*10000, "10 ADC 3 offset*10000");
fmtprint(i+11, ppwr->adc[3].scale*10000,  "11 ADC 3 scale*10000");
fmtprint(i+12, ppwr->adc[4].offset*10000, "12 ADC 4 offset*10000");
fmtprint(i+13, ppwr->adc[4].scale*10000,  "13 ADC 4 scale*10000");
fmtprint(i+14, ppwr->adc[5].offset, "14 ADC 5 Hall-effect offset: steps");
fmtprint(i+15, ppwr->adc[5].scale,  "15 ADC 5 Hall-effect: volts/step");
fmtprint(i+16, ppwr->adc[6].offset*10000, "16 ADC 6 offset*10000");
fmtprint(i+17, ppwr->adc[6].scale*10000,  "17 ADC 6 scale*10000");
fmtprint(i+18, ppwr->adc[7].offset*10000, "18 ADC 7 offset*10000");
fmtprint(i+19, ppwr->adc[7].scale*10000,  "19 ADC 7 scale*10000");

fmtprint(i+20, ppwr->iir[0].k,     "20 filter 0 factor k: internal temp");
fmtprint(i+21, ppwr->iir[0].scale, "21 filter 0 scale   : internal temp");
fmtprint(i+22, ppwr->iir[1].k,     "22 filter 1 factor k: internal Vref");
fmtprint(i+23, ppwr->iir[1].scale, "23 filter 1 scale   : internal Vref");
fmtprint(i+24, ppwr->iir[2].k,     "24 filter 2 factor k: CAN bus V");
fmtprint(i+25, ppwr->iir[2].scale, "25 filter 2 scale   : CAN bus V");
fmtprint(i+26, ppwr->iir[3].k,     "26 filter 3 factor k: Power input V");
fmtprint(i+27, ppwr->iir[3].scale, "27 filter 3 scale   : Power input V");

printf("%2d	0x%08X	%s\n\r",i+28, (unsigned int)ppwr->cid_heartbeat,"28 CANID: heartbeat");
printf("%2d	0x%08X	%s\n\r",i+29, (unsigned int)ppwr->cid_pwr_msg,  "29 CANID: poll msg");
printf("%2d	0x%08X	%s\n\r",i+30, (unsigned int)ppwr->cid_pwr_alarm,"30 CANID: alarm");

printf("%2d	%d	%s\n\r",i+ 3, (unsigned int)ppwr->alarmct,  "31 Time between alarm msgs (ms)");
fmtprint(i+32, ppwr->alarm_thres, "32 alarm msg threshold voltage");

printf("%2d	0x%08X	%s\n\r",i+33, (unsigned int)ppwr->code_CAN_filt[0], " 33 CANID_HB_TIMESYNC");
printf("%2d	0x%08X	%s\n\r",i+34, (unsigned int)ppwr->code_CAN_filt[1], " 34 CANID_MSG_TIME_POLLs");
printf("%2d	0x%08X	%s\n\r",i+35, (unsigned int)ppwr->code_CAN_filt[2], " 35 Dummy");
printf("%2d	0x%08X	%s\n\r",i+36, (unsigned int)ppwr->code_CAN_filt[3], " 36 Dummy");
printf("%2d	0x%08X	%s\n\r",i+37, (unsigned int)ppwr->code_CAN_filt[4], " 37 Dummy");
printf("%2d	0x%08X	%s\n\r",i+38, (unsigned int)ppwr->code_CAN_filt[5], " 38 Dummy");
printf("%2d	0x%08X	%s\n\r",i+39, (unsigned int)ppwr->code_CAN_filt[6], " 39 Dummy");
printf("%2d	0x%08X	%s\n\r",i+40, (unsigned int)ppwr->code_CAN_filt[7], " 40 Dummy");

printf("%2d	0x%08X	%s\n\r",i+41, (unsigned int)ppwr->cid_lvl_msg,"41 CANID: Hall-effect msg");
printf("%2d	0x%08X	%s\n\r",i+42, (unsigned int)ppwr->cid_lvl_hb, "42 CANID: Hall-effect hb");

	USART1_txint_send(); 
	return;
}

