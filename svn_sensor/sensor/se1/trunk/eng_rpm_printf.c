/******************************************************************************
* File Name          : eng_rpm_printf.c
* Date First Issued  : 03/26/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "eng_rpm_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/*
// Engine RPM function
struct ENGRPMLC
{
	uint32_t size;			// 0 Number of items in struct
 	uint32_t crc;			// 1 crc-32 placed by loader
	uint32_t version;		// 2 struct version number
	uint32_t hbct;			// 3 Time (ms) between HB msg
	uint32_t seg_ct;		// 4 Counts per revolution on engine
	uint32_t cid_msg;    // 5 CANID: RPM, calirated, response to poll
	uint32_t cid_hb;	   // 6 CANID: RPM, calirated, heartbeat
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
};
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CRC'              , 1, 'TYP_U32','%08X','ENG_RPM','Eng_rpm: CRC');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_VERSION'          , 2, 'TYP_S32' ,'%d','ENG_RPM','Eng_rpm: Version number');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_HEARTBEAT_TIME_CT', 3, 'TYP_S32' ,'%d','ENG_RPM','Eng_rpm: Time (ms) between HB msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_SEG_CT'           , 4, 'TYP_U32' ,'%u','ENG_RPM','Eng_rpm: Counts per revolution on engine');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_POLL_RPM_R'       , 5,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID: Poll response, rpm, calibrated');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_HB_RPM_R'         , 6,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID: Heartbeat: rpm, calibrated');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT1'   , 7,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 1 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT2'   , 8,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 2 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT3'   , 9,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 3 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT4'   ,10,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 4 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT5'   ,11,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 5 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT6'   ,12,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 6 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT7'   ,13,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 7 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_RPM_CANID_HW_FILT8'   ,14,'TYP_CANID','%x','ENG_RPM','Eng_rpm: CANID 8 added to CAN hw filter to allow incoming msg');
*/
/* **************************************************************************************
 * void eng_rpm_printf(struct ENGRPMLC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void eng_rpm_printf(struct ENGRPMLC* p)
{
int i = 0;
printf("ENG_RPM: values: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%2d	%d	%s\n\r",i+0, (unsigned int)p->size,   "  0 Number of items in list");
printf("%2d	%d	%s\n\r",i+1, (unsigned int)p->crc,    "  1 Eng_rpm: CRC");
printf("%2d	%d	%s\n\r",i+2, (unsigned int)p->version,"  2 Eng_rpm: Version number");
printf("%2d	%d	%s\n\r",i+3, (unsigned int)p->hbct,   "  3 Eng_rpm: Time (ms) between HB msg");
printf("%2d	%d	%s\n\r",i+4, (unsigned int)p->seg_ct, "  4 Eng_rpm: Counts per revolution on engine");

printf("%2d	0x%08X %s\n\r",i+5, (unsigned int)p->cid_msg,"  5 Eng_rpm: CANID: Poll response, rpm, calibrated"); 
printf("%2d	0x%08X %s\n\r",i+6, (unsigned int)p->cid_hb, "  6 Eng_rpm: CANID: CANID: Heartbeat: rpm, calibrated");

printf("%2d	0x%08X %s\n\r",i+7, (unsigned int)p->code_CAN_filt[0], "  7 Eng_rpm: CANID 1 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+8, (unsigned int)p->code_CAN_filt[1], "  8 Eng_rpm: CANID 2 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+9, (unsigned int)p->code_CAN_filt[2], "  9 Eng_rpm: CANID 3 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+10,(unsigned int)p->code_CAN_filt[3], " 10 Eng_rpm: CANID 4 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+11,(unsigned int)p->code_CAN_filt[4], " 11 Eng_rpm: CANID 5 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+12,(unsigned int)p->code_CAN_filt[5], " 12 Eng_rpm: CANID 6 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+13,(unsigned int)p->code_CAN_filt[6], " 13 Eng_rpm: CANID 7 in CAN hw filter");
printf("%2d	0x%08X %s\n\r",i+14,(unsigned int)p->code_CAN_filt[7], " 14 Eng_rpm: CANID 8 in CAN hw filter");

	USART1_txint_send(); 
	return;
}
