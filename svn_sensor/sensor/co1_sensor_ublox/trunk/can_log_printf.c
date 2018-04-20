/******************************************************************************
* File Name          : can_log_printf.c
* Date First Issued  : 04/22/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "can_log_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/*
 struct LOGGERLC
 {
	uint32_t size;		// 0 Number of items in struct
 	uint32_t crc;		// 1 crc-32 placed by loader
	uint32_t version;	// 2 struct version number
	uint32_t hbct;		// 3 Heartbeat count (ms) between msgs
	uint32_t cid_loghb_ctr;	// 4 CANID: Heartbeat running count
};
INSERT INTO PARAM_LIST VALUES ('LOGGER_LIST_CRC'	   	, 1, 'TYP_U32','%08X','LOGGER','Logger: CRC ');
INSERT INTO PARAM_LIST VALUES ('LOGGER_LIST_VERSION'     , 2, 'TYP_S32','%d'  ,'LOGGER','Logger: Version number');
INSERT INTO PARAM_LIST VALUES ('LOGGER_HEARTBEAT1_CT'		, 3, 'TYP_U32','%d'  ,'LOGGER','Logger: Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_LIST VALUES ('LOGGER_HEARTBEAT_MSG'    , 4, 'TYP_CANID','%x','LOGGER','Logger: CANID: Hearbeat sends running count of logged msgs');
*/
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
printf("%02d	%0x08x %s\n\r",i + 4, (unsigned int)p->cid_loghb_ctr," 4 Logger: CANID: Heartbeat running count");

	USART1_txint_send(); 
	return;
}
