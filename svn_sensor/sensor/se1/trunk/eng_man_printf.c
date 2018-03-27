/******************************************************************************
* File Name          : eng_man_printf.c
* Date First Issued  : 03/23/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "eng_man_printf.h"
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
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CRC'              ,'0',     'ENG_MANIFOLD','Eng1_manifold: CRC');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_VERSION'          ,'1',     'ENG_MANIFOLD','Eng1_manifold: Version number');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_HEARTBEAT_TIME_CT','1000',  'ENG_MANIFOLD','Eng1_manifold: Time (ms) between HB msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_PRESS_OFFSET'     ,'-0.5',  'ENG_MANIFOLD','Eng1_manifold: Manifold pressure offset');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_PRESS_SCALE'      ,'103.6', 'ENG_MANIFOLD','Eng1_manifold: Manifold pressure  scale (inch Hg)');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_POLL_MANFLD_R','CANID_MSG_ENG1_MANFLD','ENG_MANIFOLD','Eng1_manifold: CANID: Poll response: manifold pressure, calibrated');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_HB_MANFLD_R'  ,'CANID_HB_ENG1_MANFLD' ,'ENG_MANIFOLD','Eng1_manifold: CANID: Heartbeat: manifold pressure, calibrated');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT1','CANID_HB_TIMESYNC'     ,'ENG_MANIFOLD','EPS_1: CANID 1: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT2','CANID_MSG_TIME_POLL'   ,'ENG_MANIFOLD','MC: CANID 2:Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT3','CANID_CMD_ENG1_MANFLDI','ENG_MANIFOLD','Eng1_manifold: CANID 3: command');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT4','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 4 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT5','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 5 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT6','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 6 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT7','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 7 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT8','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 8 add CAN hw filter for incoming msg');
*/
/* **************************************************************************************
 * void eng_man_printf(struct ENGMANLC* p);
 * @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */
void eng_man_printf(struct ENGMANLC* p)
{
int i = 0;
printf("ENG_MAINIFOLD: pointer = %08X\n\r",(int)p);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%2d	%d	%s\n\r",   i + 0, (unsigned int)p->size,     "  0 Number of items in list");
printf("%2d	%d	%s\n\r",   i + 1, (unsigned int)p->crc,      "  1 Eng_manifold: CRC");
printf("%2d	%d	%s\n\r",   i + 2, (unsigned int)p->version,  "  2 Eng_manifold: Version number");
printf("%2d	%d	%s\n\r",   i + 3, (unsigned int)p->hbct,     "  3 Eng_manifold: Time (ms) between HB msg");
fmtprint(i+ 4, p->press_offset,                             "  4 Eng_manifold: Manifold pressure offset");
fmtprint(i+ 5, p->press_scale,                              "  5 Eng_manifold: Manifold pressure  scale (inch Hg)");

printf("%2d	0x%08X	%s\n\r",i+6, (unsigned int)p->cid_msg,"  6 Eng_manifold: CANID: Poll response: manifold pressure, calibrated"); 
printf("%2d	0x%08X	%s\n\r",i+7, (unsigned int)p->cid_hb, "  7 Eng_manifold: CANID: Heartbeat: manifold pressure, calibrated");

printf("%2d	0x%08X	%s\n\r",i+8, (unsigned int)p->code_CAN_filt[0], "  8 Eng_man: CANID 1 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+9, (unsigned int)p->code_CAN_filt[1], "  9 Eng_man: CANID 2 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+10, (unsigned int)p->code_CAN_filt[2], " 10 Eng_man: CANID 3 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+11 (unsigned int)p->code_CAN_filt[3], " 11 Eng_man: CANID 4 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+12, (unsigned int)p->code_CAN_filt[4]," 12 Eng_man: CANID 5 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+13, (unsigned int)p->code_CAN_filt[5]," 13 Eng_man: CANID 6 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+14, (unsigned int)p->code_CAN_filt[6]," 14 Eng_man: CANID 7 in CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+15, (unsigned int)p->code_CAN_filt[7]," 15 Eng_man: CANID 8 in CAN hardware filter");

	USART1_txint_send(); 
	return;
}
