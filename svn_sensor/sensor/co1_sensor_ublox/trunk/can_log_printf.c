/******************************************************************************
* File Name          : can_log_printf.c
* Date First Issued  : 04/22/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
/*
07/22/2018 added Alarm msg
*/
#include "can_log_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/* **************************************************************************************
 * void gos_poll_printf(struct GPSLC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void can_log_printf(struct LOGGERLC* p)
{
int i = 0;
printf("LOGGERLC: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%02d	%d	%s\n\r",    i + 0, (unsigned int)p->size,         " 0 Number of items in list");
printf("%02d	%d	%s\n\r",    i + 1, (unsigned int)p->crc,          " 1 Logger: CRC");
printf("%02d	%d	%s\n\r",    i + 2, (unsigned int)p->version,      " 2 Logger: Version number");
printf("%02d	%d	%s\n\r",    i + 3, (unsigned int)p->hbct,         " 3 Logger: Heartbeat count (ms) between msgs");
printf("%02d	0x%08X %s\n\r",i + 4, (unsigned int)p->cid_loghb_ctr," 4 Logger: CANID: Heartbeat running count");
printf("%02d	0x%08X %s\n\r",i + 5, (unsigned int)p->cid_log_alrm, " 5 CANID: Suspend logging alarm msg (receive)");
printf("%02d	%d	%s\n\r",    i + 6, (unsigned int)p->suspend_ct,   " 6 Suspend duration (ms)");

	USART1_txint_send(); 
	return;
}
