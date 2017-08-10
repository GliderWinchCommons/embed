/******************************************************************************
* File Name          : encoder_idx_v_struct.c
* Date First Issued  : 07/14/2017
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/* This file is manually generated and must match the idx_v_val.c file that is
generated with the java program and database.

The reference for the sequence of items in this file is the database table--
PARAM_LIST
And for making making changes to *values* manually,
PARAM_VAL_INSERT.sql

If the number of entries in this file differs the initialization of 'encoder.c' will
bomb with a mis-match error....(maybe!)


*/
#include <stdint.h>
#include "encoder_idx_v_struct.h"
#include "encoder_a_functionS.h"
#include "db/gen_db.h"

#define NULL 0

/* **************************************************************************************
 * int encoder_idx_v_struct_copy2(struct ENCODERALC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */\
/*
 struct ENCODERALC
 {
	uint32_t size;			// 0 Number of items in struct
 	uint32_t crc;			// 1 crc-32 placed by loader
	uint32_t version;		// 2 struct version number
	uint32_t hbct;			// 3 Heartbeat ct: time (ms) between msgs
	uint32_t ctperrev;		// 4 Number of counts per revolution
	float	distperrev;		// 5 Distance per revolution (meters)
	uint32_t cid_heartbeat;		// 6 CANID-Heartbeat msg 
	uint32_t cid_poll_msg;		// 7 CANID-Fully calibrated encoder msg
	uint32_t cid_poll;		// 8 CANID-Response to poll (??)
	uint32_t cid_poll_r;		// 9 CANID-Response to poll (??)
	// CAN ID's for setting up hw filter
	uint32_t cid_hb_timesync;	//10 CANID-GPS time sync msg polls encoders
	uint32_t cid_msg_time_poll;	//11
	uint32_t cid_tst_enc_a;		//12 
	uint32_t cid_cmd_hi;		//13 CANID-Command incoming/interrogation
	uint32_t code_CAN_filt[4];	//14 Remaining unassigned in list (CANID_DUMMY)
 };

INSERT INTO PARAM_LIST VALUES ('ENCODER_LIST_CRC'	     	, 1, 'TYP_U32','%08X', 	'SHEAVE_H',	'Encoder_f4_1: crc: CRC for tension list');
INSERT INTO PARAM_LIST VALUES ('ENCODER_LIST_VERSION'  	    	, 2, 'TYP_S32','%d', 	'SHEAVE_H',	'Encoder_f4_1: version: Version number for Tension List');
INSERT INTO PARAM_LIST VALUES ('ENCODER_HEARTBEAT_CT'		, 3, 'TYP_U32','%d',	'SHEAVE_H',	'Encoder_f4_1: Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_LIST VALUES ('ENCODER_CT_PER_REV'		, 4, 'TYP_U32','%d',	'SHEAVE_H',	'Encoder_f4_1: Number of counts per revolution');
INSERT INTO PARAM_LIST VALUES ('ENCODER_DIST_PER_REV'		, 5, 'TYP_FLT','%f',	'SHEAVE_H',	'Encoder_f4_1: Distance per revolution (meters)');
INSERT INTO PARAM_LIST VALUES ('ENCODER_HEARTBEAT_MSG'    	, 6, 'TYP_CANID','%x', 	'SHEAVE_H', 	'Encoder_f4_1: Heartbeat sends raw (long long) running encoder count');
INSERT INTO PARAM_LIST VALUES ('ENCODER_POLL_MSG'	    	, 7, 'TYP_CANID','%x', 	'SHEAVE_H', 	'Encoder_f4_1: CANID: hi-res: msg--upper calibrated--distance and speed');
INSERT INTO PARAM_LIST VALUES ('ENCODER_POLL'	    		, 8, 'TYP_CANID','%x', 	'SHEAVE_H', 	'Encoder_f4_1: CANID: Poll with time sync msg');
INSERT INTO PARAM_LIST VALUES ('ENCODER_POLL_R'	  		, 9, 'TYP_CANID','%x', 	'SHEAVE_H', 	'Encoder_f4_1: CANID: Response to POLL');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT1'    ,10, 'TYP_CANID','%x',	'SHEAVE_H',	'Encoder_f4_1: CANID 1 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT2'    ,11, 'TYP_CANID','%x',	'SHEAVE_H',	'Encoder_f4_1: CANID 2 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT3'    ,12, 'TYP_CANID','%x',	'SHEAVE_H',	'Encoder_f4_1: CANID 3 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT4'    ,13, 'TYP_CANID','%x',	'SHEAVE_H',	'Encoder_f4_1: CANID 4 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT5'    ,14, 'TYP_CANID','%x', 	'SHEAVE_H',	'Encoder_f4_1: CANID 5 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT6'    ,15, 'TYP_CANID','%x',  	'SHEAVE_H',	'Encoder_f4_1: CANID 6 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT7'    ,16, 'TYP_CANID','%x',  	'SHEAVE_H',	'Encoder_f4_1: CANID 7 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_LIST VALUES ('SHEAVE_H_CANID_HW_FILT8'    ,17, 'TYP_CANID','%x',  	'SHEAVE_H',	'Encoder_f4_1: CANID 8 added to CAN hardware filter to allow incoming msg');
*/
int encoder_idx_v_struct_copy(struct ENCODERALC* p, uint32_t* ptbl)
{
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;

/* NOTE: values that are not uint32_t  */
p->size         = ptbl[0];		 		/*  0 Tension: Number of elements in the following list */
p->crc          = ptbl[ENCODER_LIST_CRC];		/*  1 Tension: CRC for tension list */
p->version      = ptbl[ENCODER_LIST_VERSION];		/*  2 Version number */
p->hbct		= ptbl[ENCODER_HEARTBEAT_CT];		/*  3 Heartbeat count of time (ms) between msgs */
p->ctperrev	= ptbl[ENCODER_CT_PER_REV];		/*  4 Number of counts per revolution */
u.ui = ptbl[ENCODER_DIST_PER_REV]; 		p->distperrev = u.f;/*  5 Distance per revolution (meters) */
p->cid_heartbeat= ptbl[ENCODER_HEARTBEAT_MSG];		/*  6 Heartbeat sends raw (long long) running encoder count */

p->cid_poll_msg   	= ptbl[ENCODER_POLL_MSG];		// CANID-Fully calibrated encoder msg
p->cid_poll 	   	= ptbl[ENCODER_POLL];		// CANID-Response to poll (??)
p->cid_poll_r   	= ptbl[ENCODER_POLL_R];		// CANID-Response to poll (??)
	// CAN ID's for setting up hw filter
p->cid_hb_timesync   	= ptbl[SHEAVE_H_CANID_HW_FILT1];// CANID-GPS time sync msg polls encoders
p->cid_msg_time_poll   	= ptbl[SHEAVE_H_CANID_HW_FILT2];//
p->cid_tst_enc_a   	= ptbl[SHEAVE_H_CANID_HW_FILT3];// 
p->cid_cmd_hi   	= ptbl[SHEAVE_H_CANID_HW_FILT4];// CANID-Command incoming/interrogation
p->code_CAN_filt[0]   	= ptbl[SHEAVE_H_CANID_HW_FILT5];// Remaining unassigned in list (CANID_DUMMY)
p->code_CAN_filt[1]   	= ptbl[SHEAVE_H_CANID_HW_FILT6];// Remaining unassigned in list (CANID_DUMMY)
p->code_CAN_filt[2]   	= ptbl[SHEAVE_H_CANID_HW_FILT7];// Remaining unassigned in list (CANID_DUMMY)
p->code_CAN_filt[3]   	= ptbl[SHEAVE_H_CANID_HW_FILT8];// Remaining unassigned in list (CANID_DUMMY)

return PARAM_LIST_CT_SHEAVE_H;
}


