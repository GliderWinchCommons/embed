/******************************************************************************
* File Name          : eng_thr_printf.c
* Date First Issued  : 03/23/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "eng_thr_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/*
struct ENGTHROTTLELC
{
	uint32_t size;			// 0 Number of items in struct
 	uint32_t crc;			// 1 crc-32 placed by loader
	uint32_t version;		// 2 struct version number
	uint32_t hbct;			// 3 Time (ms) between HB msg
	uint32_t throttle_open;// 4 Throttle ADC reading: full open
	uint32_t throttle_close;// 5 ADC when throttle closed
	uint32_t cid_msg;    // 6 CANID: Poll response: throttle (0.0-100.0)
	uint32_t cid_hb;	   // 7 CANID: Throttle position, calibrated heartbeat
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter

};
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CRC'              , 1,'TYP_U32','%08X','ENG_THROTTLE','Eng_throttle: CRC');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_VERSION'          , 2,'TYP_S32',  '%d','ENG_THROTTLE','Eng_throttle: Version number');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_HEARTBEAT_TIME_CT', 3,'TYP_S32',  '%d','ENG_THROTTLE','Eng_throttle: Time (ms) between HB msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_OPENFULL'         , 4,'TYP_FLT',  '%f','ENG_THROTTLE','Eng_throttle: ADC when throttle full open');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CLOSED'           , 5,'TYP_FLT',  '%f','ENG_THROTTLE','Eng_throttle: ADC when throttle closed');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_POLL_THROTTLE_R'  , 6,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID: Poll response: throttle (0.0-100.0)');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_HB_THROTTLE_R'    , 7,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID: Heartbeat: throttle (0.0-100.0)');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT1'   , 8,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 1 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT2'   , 9,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 2 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT3'   ,10,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 3 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT4'   ,11,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 4 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT5'   ,12,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 5 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT6'   ,13,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 6 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT7'   ,14,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 7 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_THR_CANID_HW_FILT8'   ,15,'TYP_CANID','%x','ENG_THROTTLE','Eng_throttle: CANID 8 added to CAN hw filter to allow incoming msg');

*/
/* **************************************************************************************
 * void eng_thr_printf(struct ENGTHROTTLELC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void eng_thr_printf(struct ENGTHROTTLELC* p)
{

int i = 0;
printf("ENG_THROTTLE: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%02d	%d	%s\n\r",i+0, (unsigned int)p->size,          "  0 Number of items in list");
printf("%02d	%d	%s\n\r",i+1, (unsigned int)p->crc,           "  1 Eng_throttle: CRC");
printf("%02d	%d	%s\n\r",i+2, (unsigned int)p->version,       "  2 Eng_throttle: Version number");
printf("%02d	%d	%s\n\r",i+3, (unsigned int)p->hbct,          "  3 Eng_throttle: Time (ms) between HB msg");
printf("%02d	%d	%s\n\r",i+4, (unsigned int)p->throttle_open, "  4 Eng_throttle: ADC when throttle full open");
printf("%02d	%d	%s\n\r",i+5, (unsigned int)p->throttle_close,"  5 Eng_throttle: ADC when throttle closed");

printf("%02d	0x%08X %s\n\r",i+6, (unsigned int)p->cid_msg,   "  6 Eng_throttle: CANID: Poll response: manifold pressure, calibrated"); 
printf("%02d	0x%08X %s\n\r",i+7, (unsigned int)p->cid_hb,    "  7 Eng_throttle: CANID: Heartbeat: manifold pressure, calibrated");

printf("%02d	0x%08X %s\n\r",i+8, (unsigned int)p->code_CAN_filt[0], "  8 Eng_throttle: CANID 1 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+9, (unsigned int)p->code_CAN_filt[1], "  9 Eng_throttle: CANID 2 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+10,(unsigned int)p->code_CAN_filt[2]," 10 Eng_throttle: CANID 3 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+11,(unsigned int)p->code_CAN_filt[3]," 11 Eng_throttle: CANID 4 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+12,(unsigned int)p->code_CAN_filt[4]," 12 Eng_throttle: CANID 5 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+13,(unsigned int)p->code_CAN_filt[5]," 13 Eng_throttle: CANID 6 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+14,(unsigned int)p->code_CAN_filt[6]," 14 Eng_throttle: CANID 7 in CAN hardware filter");
printf("%02d	0x%08X %s\n\r",i+15,(unsigned int)p->code_CAN_filt[7]," 15 Eng_throttle: CANID 8 in CAN hardware filter");

	USART1_txint_send(); 
	return;
}
