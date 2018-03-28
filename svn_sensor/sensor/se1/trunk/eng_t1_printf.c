/******************************************************************************
* File Name          : eng_t1_printf.c
* Date First Issued  : 03/23/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "eng_t1_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/*
// Manifold pressure function 
struct ENGMANLC
{
	uint32_t size;			// 0 Number of items in struct
 	uint32_t crc;			// 1 crc-32 placed by loader
	uint32_t version;		// 2 struct version number
	uint32_t hbct;			// 3 Time (ms) between HB msg
	float	press_offset;	// 4 Mainfold pressure, zero offset
	float press_scale;	// 5 Manifold pressure, scale
	uint32_t cid_msg;    // 6 Manifold pressure, calibrated, response to poll
	uint32_t cid_hb;	   // 7 Manifold pressure, calibrated, heartbeat
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's in hw filter
};
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CRC'              , 1,'TYP_U32','%08X', 'ENG_T1',	'Eng_t1: CRC');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_VERSION'          , 2,'TYP_S32',  '%d', 'ENG_T1',	'Eng_t1: Version number');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_HEARTBEAT_TIME_CT', 3,'TYP_S32',  '%d', 'ENG_T1',	'Eng_t1: Time (ms) between HB msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CONST_B'          , 4,'TYP_FLT',  '%f', 'ENG_T1',	'Eng_t1: Thermistor param: constant B');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_R_SERIES'         , 5,'TYP_FLT',  '%f', 'ENG_T1',	'Eng_t1: Thermistor param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_R_ROOMTMP'        , 6,'TYP_FLT',  '%f', 'ENG_T1',	'Eng_t1: Thermistor param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_REF_TEMP'         , 7,'TYP_FLT',  '%f', 'ENG_T1',	'Eng_t1: Thermistor param: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_TEMP_OFFSET'      , 8,'TYP_FLT',  '%f', 'ENG_T1',	'Eng_t1: Thermistor param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_TEMP_SCALE'       , 9,'TYP_FLT',  '%f', 'ENG_T1',	'Eng_t1: Thermistor param: Thermistor temp scale correction');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_POLL_T1_R'        ,10,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID: Poll response: temperature #1 (deg C)');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_HB_T1_R'          ,11,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID: Heartbeat: temperature #1 (deg C)');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT1'   ,12,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 1 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT2'   ,13,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 2 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT3'   ,14,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 3 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT4'   ,15,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 4 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT5'   ,16,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 5 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT6'   ,17,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 6 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT7'   ,18,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 7 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('ENG_T1_CANID_HW_FILT8'   ,19,'TYP_CANID','%x','ENG_T1','Eng_t1: CANID 8 added to CAN hw filter to allow incoming msg');
*/
/* **************************************************************************************
 * void eng_t1_printf(struct ENGMANLC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void eng_t1_printf(struct ENGT1LC* p)
{
int i = 0;
printf("ENG_T1: values: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%2d	%d	%s\n\r",i+0, (unsigned int)p->size,     "  0 Number of items in list");
printf("%2d	%d	%s\n\r",i+1, (unsigned int)p->crc,      "  1 Eng_t1 CRC");
printf("%2d	%d	%s\n\r",i+2, (unsigned int)p->version,  "  2 Eng_t1 Version number");
printf("%2d	%d	%s\n\r",i+3, (unsigned int)p->hbct,     "  3 Eng_t1 Time (ms) between HB msg");

fmtprint(i+ 4,p->therm.B,     "  5 ENG_T1_THERM1_CONST_B,    Eng_t1 Thermistor1 param: constant B");
fmtprint(i+ 5,p->therm.R0,    "  6 ENG_T1_THERM1_R_SERIES,	  Eng_t1 Thermistor1 param: Series resistor, fixed (K ohms)");
fmtprint(i+ 6,p->therm.RS,    "  7 ENG_T1_THERM1_R_ROOMTMP,  Eng_t1 Thermistor1 param: Thermistor room temp resistance (K ohms)");
fmtprint(i+ 7,p->therm.TREF,  "  8 ENG_T1_THERM1_REF_TEMP,	  Eng_t1 Thermistor1 param: Reference temp for thermistor");
fmtprint(i+ 8,p->therm.offset,"  9 ENG_T1_THERM1_TEMP_OFFSET,Eng_t1 Thermistor1 param: Thermistor temp offset correction (deg C)");
fmtprint(i+ 9,p->therm.scale, " 10 ENG_T1_THERM1_TEMP_SCALE, Eng_t1 Thermistor1 param: Thermistor temp scale correction");

printf("%2d	0x%08X	%s\n\r",i+10, (unsigned int)p->cid_msg," 10 Eng_t1 CANID: Poll response: manifold pressure, calibrated"); 
printf("%2d	0x%08X	%s\n\r",i+11, (unsigned int)p->cid_hb, " 11 Eng_t1 CANID: Heartbeat: manifold pressure, calibrated");

printf("%2d	0x%08X	%s\n\r",i+12, (unsigned int)p->code_CAN_filt[0], " 12 Eng_t1 CANID 1 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+13, (unsigned int)p->code_CAN_filt[1], " 13 Eng_t1 CANID 2 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+14, (unsigned int)p->code_CAN_filt[2], " 14 Eng_t1 CANID 3 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+15, (unsigned int)p->code_CAN_filt[3], " 15 Eng_t1 CANID 4 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+16, (unsigned int)p->code_CAN_filt[4], " 16 Eng_t1 CANID 5 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+17, (unsigned int)p->code_CAN_filt[5], " 17 Eng_t1 CANID 6 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+18, (unsigned int)p->code_CAN_filt[6], " 18 Eng_t1 CANID 7 in CAN hw filter");
printf("%2d	0x%08X	%s\n\r",i+19, (unsigned int)p->code_CAN_filt[7], " 19 Eng_t1 CANID 8 in CAN hw filter");

	USART1_txint_send(); 
	return;
}
