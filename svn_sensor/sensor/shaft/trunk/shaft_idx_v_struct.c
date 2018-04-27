/******************************************************************************
* File Name          : tension_idx_v_struct.c
* Date First Issued  : 07/15/2015,06/04/2016
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/* This file is manually generated and must match the idx_v_val.c file that is
generated with the java program and database.

The reference for the sequence of items in this file is the database table--
PARAM_LIST
And for making making changes to *values* manually,
PARAM_VAL_INSERT.sql

If the number of entries in this file differs the initialization of 'tension.c' will
bomb with a mis-match error.
*/
#include <stdint.h>
#include "shaft_idx_v_struct.h"
#include "shaft_function.h"
#include "db/gen_db.h"

#define NULL 0

/* **************************************************************************************
 * int shaft_idx_v_struct_copy_shaft(struct SHAFTLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int shaft_idx_v_struct_copy_shaft(struct SHAFTLC* p, uint32_t* ptbl)
{
/* Avoid compiler warning: Skip until a float is added
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;
*/
/*
-- Drive shaft reflective sensor
--
INSERT INTO PARAM_LIST VALUES ('SHAFT_LIST_CRC'       , 1,'TYP_U32','%08X','DRIVE_SHAFT','Shaft: CRC ');
INSERT INTO PARAM_LIST VALUES ('SHAFT_LIST_VERSION'   , 2,'TYP_S32','%d',  'DRIVE_SHAFT','Shaft: Version number');
INSERT INTO PARAM_LIST VALUES ('SHAFT_HEARTBEAT_CT'   , 3,'TYP_U32','%d',  'DRIVE_SHAFT','Shaft: Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_LIST VALUES ('SHAFT_ADC3_HTR'       , 4,'TYP_U32','%d',  'DRIVE_SHAFT','Shaft: High threshold register setting, ADC3');
INSERT INTO PARAM_LIST VALUES ('SHAFT_ADC3_LTR'       , 5,'TYP_U32','%d',  'DRIVE_SHAFT','Shaft: Low  threshold register setting, ADC3');
INSERT INTO PARAM_LIST VALUES ('SHAFT_ADC2_HTR'       , 6,'TYP_U32','%d',  'DRIVE_SHAFT','Shaft: High threshold register setting, ADC2');
INSERT INTO PARAM_LIST VALUES ('SHAFT_ADC2_LTR'       , 7,'TYP_U32','%d',  'DRIVE_SHAFT','Shaft: Low  threshold register setting, ADC2');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CID_MSG_SPEED'  , 8,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID: Shaft speed, calibrated, response to poll');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CID_HB_SPEED'   , 9,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID: Shaft speed, calibrated, heartbeat');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CID_MSG_CT'     ,10,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID: Shaft running count, response to poll');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CID_HB_CT '     ,11,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID: Shaft running count, heartbeat');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT1' ,12,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 1 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT2' ,13,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 2 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT3' ,14,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 3 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT4' ,15,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 4 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT5' ,16,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 5 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT6' ,17,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 6 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT7' ,18,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 7 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHAFT_CANID_HW_FILT8' ,19,'TYP_CANID','%x','DRIVE_SHAFT','Shaft: CANID 8 added to CAN hw filter to allow incoming msg');
 struct SHAFTLC
 {
	uint32_t size;					//  0 Number of items in struct
 	uint32_t crc;					//  1 crc-32 placed by loader
	uint32_t version;				//  2 struct version number
	uint32_t hbct;					//  3 Time (ms) between HB msg (speed/count alternate)
	uint32_t adc3_htr_initial; //  4 High threshold register setting, ADC3
	uint32_t adc3_ltr_initial; //  5 Low  threshold register setting, ADC3
	uint32_t adc2_htr_initial; //  6 High threshold register setting, ADC2
	uint32_t adc2_ltr_initial; //  7 Low  threshold register setting, ADC2
	uint32_t cid_msg_speed;    //  8 Shaft speed, calibrated, response to poll
	uint32_t cid_hb_speed;	   //  9 Shaft speed, calibrated, heartbeat
	uint32_t cid_msg_ct;    	// 10 Shaft running count, response to poll
	uint32_t cid_hb_ct;	   	// 11 Shaft running count, heartbeat
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
 };
*/

/* NOTE: values that are not uint32_t  */
p->size            = ptbl[0];                      /*  0 Shaft: Number of elements in the following list */
p->crc             = ptbl[SHAFT_LIST_CRC];         /*  1 Shaft: CRC for tension list */
p->version         = ptbl[SHAFT_LIST_VERSION];     /*  2 Shaft: Version number */	
p->hbct            = ptbl[SHAFT_HEARTBEAT_CT];     /*  3 Shaft: Heartbeat count of time (ms) between msgs') */
p->adc3_htr_initial= ptbl[SHAFT_ADC3_HTR];         /*  4 Shaft: High threshold register setting, ADC3 */
p->adc3_ltr_initial= ptbl[SHAFT_ADC3_LTR];         /*  5 Shaft: Low  threshold register setting, ADC3 */
p->adc2_htr_initial= ptbl[SHAFT_ADC2_HTR];         /*  6 Shaft: High threshold register setting, ADC2 */
p->adc2_ltr_initial= ptbl[SHAFT_ADC2_LTR];         /*  7 Shaft: Low  threshold register setting, ADC2 */
p->cid_msg_speed   = ptbl[SHAFT_CID_MSG_SPEED];    /*  8 Shaft: CANID: Shaft speed, calibrated, response to poll */
p->cid_hb_speed    = ptbl[SHAFT_CID_HB_SPEED];     /*  9 Shaft: CANID: Shaft speed, calibrated, heartbeat */
p->cid_msg_ct      = ptbl[SHAFT_CID_MSG_CT];       /* 10 Shaft: CANID: Shaft running count, response to poll */
p->cid_hb_ct       = ptbl[SHAFT_CID_HB_CT];        /* 11 Shaft: CANID: Shaft running count, heartbeat */
p->num_seg         = ptbl[SHAFT_NUMBER_SEG];       /* 12,Shaft: Number of segments on code wheel'); */
p->code_CAN_filt[0] = ptbl[SHAFT_CANID_HW_FILT1];	/* 13 Shaft: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[SHAFT_CANID_HW_FILT2];	/* 14 Shaft: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[SHAFT_CANID_HW_FILT3];	/* 15 Shaft: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[SHAFT_CANID_HW_FILT4];	/* 16 Shaft: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[SHAFT_CANID_HW_FILT5];	/* 17 Shaft: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[SHAFT_CANID_HW_FILT6];	/* 18 Shaft: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[SHAFT_CANID_HW_FILT7];	/* 19 Shaft: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[SHAFT_CANID_HW_FILT8];	/* 20 Shaft: CAN ID 8 for setting up CAN hardware filter */


return PARAM_LIST_CT_DRIVE_SHAFT;
}


