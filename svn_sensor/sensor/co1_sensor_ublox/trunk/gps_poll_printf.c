/******************************************************************************
* File Name          : gps_poll_printf.c
* Date First Issued  : 04/22/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "gps_poll_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/*
 struct GPSLC
 {
	uint32_t size;		// 0 Number of items in struct
 	uint32_t crc;		// 1 crc-32 placed by loader
	uint32_t version;	// 2 struct version number
	uint32_t hbct_tim;	// 3 Time (ms) between unix time msgs
	uint32_t hbct_llh;	// 4 Time (ms) between burst of lat lon height msgs
	uint32_t hbct_llh_delay;// 5 Heartbeat count (ms) between gps lat/lon/ht msgs burst
	uint32_t cid_hb_tim;	// 6 CANID: Heartbeat unix time
	uint32_t cid_hb_llh;	// 7 CANID: Heartbeat fix (3 msgs) lat/lon/ht
	uint32_t disable_sync;	// 8 0 = enable time sync msgs; 1 = disable
	uint32_t cid_timesync;	// 9 CANID: Time Sync msg
};
INSERT INTO PARAM_LIST VALUES ('GPS_LIST_CRC'	   		  , 1, 'TYP_U32'  ,'%08X',	'GPS', 'GPS: CRC ');
INSERT INTO PARAM_LIST VALUES ('GPS_LIST_VERSION'          , 2, 'TYP_S32'  ,'%d', 	'GPS', 'GPS: Version number');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_TIME_CT'     , 3, 'TYP_U32'  ,'%d',	'GPS', 'GPS: Time (ms) between unix time msgs');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_LLH_CT'      , 4, 'TYP_U32'  ,'%d',	'GPS', 'GPS: Time (ms) between burst of lat lon height msgs');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_LLH_DELAY_CT', 5, 'TYP_U32'  ,'%d',	'GPS', 'GPS: Time (ms) between lat/lon and lon/ht msgs');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_TIME'        , 6, 'TYP_CANID','%x', 	'GPS', 'GPS: Heartbeat unix time');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_LLH'         , 7, 'TYP_CANID','%x', 	'GPS', 'GPS: Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO PARAM_LIST VALUES ('GPS_DISABLE_SYNCMSGS'      , 8, 'TYP_U32'  ,'%d',	'GPS', 'GPS: time sync msgs; 0 = enable  1 = disable');
INSERT INTO PARAM_LIST VALUES ('GPS_TIME_SYNC_MSG'         , 9, 'TYP_CANID','%x', 	'GPS', 'GPS: Time sync msg');
*/
/* **************************************************************************************
 * void gps_poll_printf(struct GPSLC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void gps_poll_printf(struct GPSLC* p)
{
int i = 0;
printf("GPSLC: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%02d	%d	%s\n\r",    i + 0, (unsigned int)p->size,          " 0 Number of items in list");
printf("%02d	%d	%s\n\r",    i + 1, (unsigned int)p->crc,           " 1 GPS: CRC");
printf("%02d	%d	%s\n\r",    i + 2, (unsigned int)p->version,       " 2 GPS: Version number");
printf("%02d	%d	%s\n\r",    i + 3, (unsigned int)p->hbct_tim,      " 3 GPS: Time (ms) between unix time msgs");
printf("%02d	%d	%s\n\r",    i + 4, (unsigned int)p->hbct_llh,      " 4 GPS: Time (ms) between burst of lat lon height msgs");
printf("%02d	%d	%s\n\r",    i + 5, (unsigned int)p->hbct_llh_delay," 5 GPS: Heartbeat count (ms) between gps lat/lon/ht msgs burst");
printf("%02d	0X%08X %s\n\r",i + 6, (unsigned int)p->cid_hb_tim,    " 6 GPS: CANID: Heartbeat unix time");
printf("%02d	0X%08X %s\n\r",i + 7, (unsigned int)p->cid_hb_llh,    " 7 GPS: CANID: Heartbeat fix (3 msgs) lat/lon/ht");
printf("%02d	%d	%s\n\r",    i + 8, (unsigned int)p->disable_sync,  " 8 GPS: 0 = enable time sync msgs; 1 = disable");
printf("%02d	0X%08X %s\n\r",i + 9, (unsigned int)p->disable_sync,  " 9 GPS: CANID: Time Sync msg");

#ifdef ADDALLTHESEINCOMINGCANIDS
printf("%02d	0x%08X	%s\n\r",i+10, (unsigned int)p->code_CAN_filt[0]," 10 GPS: CANID 1 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+11, (unsigned int)p->code_CAN_filt[1]," 11 GPS: CANID 2 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+12, (unsigned int)p->code_CAN_filt[2]," 12 GPS: CANID 3 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+13, (unsigned int)p->code_CAN_filt[3]," 13 GPS: CANID 4 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+14, (unsigned int)p->code_CAN_filt[4]," 14 GPS: CANID 5 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+15, (unsigned int)p->code_CAN_filt[5]," 15 GPS: CANID 6 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+16, (unsigned int)p->code_CAN_filt[6]," 16 GPS: CANID 7 in CAN hardware filter");
printf("%02d	0x%08X	%s\n\r",i+17, (unsigned int)p->code_CAN_filt[7]," 17 GPS: CANID 8 in CAN hardware filter");
#endif

	USART1_txint_send(); 
	return;
}
