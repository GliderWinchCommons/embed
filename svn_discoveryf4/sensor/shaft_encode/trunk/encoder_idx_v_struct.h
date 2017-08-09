/******************************************************************************
* File Name          : encoder_idx_v_struct.h
* Date First Issued  : 07/14/2017
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
Struct - matches sql parameter list
These structs serve all 'Encoder_a" function types
*/

#include <stdint.h>
#include "common_can.h"
#include "iir_filter_l.h"

#ifndef __ENCODER_IDX_V_STRUCT
#define __ENCODER_IDX_V_STRUCT


/* Discovery F4 board with two encoders */
#define NUMENCODERAFUNCTIONS	2	// Number of encoder instances

/* The parameter list supplies the CAN IDs for the hardware filter setup. */
#define CANFILTMAX	8	// Number of CAN IDs reserved in parameter list
/*
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_LIST_CRC'		,'0',	   	 	'SHEAVE_H',	'Encoder_f4_1up: 1 CRC');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_LIST_VERSION'	,'1',      		'SHEAVE_H',	'Encoder_f4_1up: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_HEARTBEAT_CT'	,'250',			'SHEAVE_H',	'Encoder_f4_1up: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_CT_PER_REV'		,'720',			'SHEAVE_H',	'Encoder_f4_1up: 4 Number of counts per revolution');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_DIST_PER_REV'	,'0.558575174',		'SHEAVE_H',	'Encoder_f4_1up: 5 Distance per revolution (meters)');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_HEARTBEAT_MSG'	,'CANID_HB_UPPER1_H_RAW','SHEAVE_H', 	'Encoder_f4_1up: 6 CANID: Heartbeat sends raw (long long) running encoder count');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_POLL_MSG'		,'CANID_MSG_UPPER1_H_CAL','SHEAVE_H', 	'Encoder_f4_1up: 7 CANID: hi-res: msg--upper calibrated--distance and speed');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_POLL'	    	,'CANID_POLL_ENCODER',	'SHEAVE_H',  	'Encoder_f4_1up: 8 CANID: Poll with time sync msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_POLL_R'	  	,'CANID_POLLR_ENCODER_1','SHEAVE_H',	'Encoder_f4_1up: 9 CANID: Response to POLL');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',	'SHEAVE_H',	'Encoder_f4_1up: 10 CANID 1 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'SHEAVE_H',	'Encoder_f4_1up: 11 CANID 2 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT3'    ,'CANID_TST_SHEAVE_1UP','SHEAVE_H',	'Encoder_f4_1up: 12 CANID 3 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT4'    ,'CANID_CMD_UPPER1_HI'	'SHEAVE_H',	'Encoder_f4_1up: 13 CANID 4 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT5'    ,'CANID_DUMMY', 	'SHEAVE_H',	'Encoder_f4_1up: 14 CANID 5 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT6'    ,'CANID_DUMMY',  	'SHEAVE_H',	'Encoder_f4_1up: 15 CANID 6 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'SHEAVE_H',	'Encoder_f4_1up: 16 CANID 7 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'SHEAVE_H',	'Encoder_f4_1up: 17 CANID 8 added to CAN hardware filter to allow incoming msg');
*/
// Naming convention--"cid" - CAN ID
 struct ENCODERALC
 {
	uint32_t size;			// 0 Number of items in struct
 	uint32_t crc;			// 1 crc-32 placed by loader
	uint32_t version;		// 2 struct version number
	uint32_t hbct;			// 3 Heartbeat ct: time (ms) between msgs
	uint32_t ctperrev;		// 4 Number of counts per revolution
	float	distperrev;		// 5 Distance per revolution (meters)
	uint32_t cid_heartbeat;		// 6 CANID-Heartbeat raw (int64_t) running encoder count
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
 
/* **************************************************************************************/
int encoder_idx_v_struct_copy(struct ENCODERALC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
#endif
