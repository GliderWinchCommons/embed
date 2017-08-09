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
p->cid_msg_time_pol   	= ptbl[SHEAVE_H_CANID_HW_FILT2];//
p->cid_tst_enc_a   	= ptbl[SHEAVE_H_CANID_HW_FILT3];// 
p->cid_cmd_hi   	= ptbl[SHEAVE_H_CANID_HW_FILT4];// CANID-Command incoming/interrogation
p->hw_CAN_filt[0]   	= ptbl[SHEAVE_H_CANID_HW_FILT5];// Remaining unassigned in list (CANID_DUMMY)
p->hw_CAN_filt[1]   	= ptbl[SHEAVE_H_CANID_HW_FILT6];// Remaining unassigned in list (CANID_DUMMY)
p->hw_CAN_filt[2]   	= ptbl[SHEAVE_H_CANID_HW_FILT7];// Remaining unassigned in list (CANID_DUMMY)
p->hw_CAN_filt[3]   	= ptbl[SHEAVE_H_CANID_HW_FILT8];// Remaining unassigned in list (CANID_DUMMY)






p->ad.offset       = (int)ptbl[TENSION_a_AD7799_1_OFFSET];	  /*  3 Tension: AD7799 offset */
u.ui = ptbl[TENSION_a_AD7799_1_SCALE]; 		p->ad.scale        = u.f;/*  4 Tension: AD7799 #1 Scale (convert to kgf) */
u.ui = ptbl[TENSION_a_THERM1_CONST_B]; 		p->ad.tp[0].B      = u.f;/*  5 Tension: Thermistor1 param: constant B */
u.ui = ptbl[TENSION_a_THERM1_R_SERIES];		p->ad.tp[0].RS     = u.f;/*  6 Tension: Thermistor1 param: Series resistor, fixed (K ohms) */
u.ui = ptbl[TENSION_a_THERM1_R_ROOMTMP];	p->ad.tp[0].R0     = u.f;/*  7 Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */
u.ui = ptbl[TENSION_a_THERM1_REF_TEMP]; 	p->ad.tp[0].TREF   = u.f;/*  8 Tension: Thermistor1 param: Reference temp for thermistor */
u.ui = ptbl[TENSION_a_THERM1_TEMP_OFFSET]; 	p->ad.tp[0].offset = u.f;/*  9 Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */
u.ui = ptbl[TENSION_a_THERM1_TEMP_SCALE];	p->ad.tp[0].scale  = u.f;/* 10 Tension: Thermistor1 param: Thermistor temp scale correction */
u.ui = ptbl[TENSION_a_THERM2_CONST_B]; 		p->ad.tp[1].B      = u.f;/* 11 Tension: Thermistor2 param: constant B */
u.ui = ptbl[TENSION_a_THERM2_R_SERIES]; 	p->ad.tp[1].RS     = u.f;/* 12 Tension: Thermistor2 param: Series resistor, fixed (K ohms) */
u.ui = ptbl[TENSION_a_THERM2_R_ROOMTMP]; 	p->ad.tp[1].R0     = u.f;/* 13 Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */
u.ui = ptbl[TENSION_a_THERM2_REF_TEMP];		p->ad.tp[1].TREF   = u.f;/* 14 Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */
u.ui = ptbl[TENSION_a_THERM2_TEMP_OFFSET]; 	p->ad.tp[1].offset = u.f;/* 15 Tension: Thermistor2 param: Thermistor temp scale correction (deg C) */
u.ui = ptbl[TENSION_a_THERM2_TEMP_SCALE]; 	p->ad.tp[1].scale  = u.f; /* 16 Tension: Thermistor2 param: Thermistor temp scale correction */
u.ui = ptbl[TENSION_a_THERM1_COEF_0]; 	p->ad.comp_t1[0] = u.f;   /* 17 Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
u.ui = ptbl[TENSION_a_THERM1_COEF_1];	p->ad.comp_t1[1] = u.f;	 /* 18 Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
u.ui = ptbl[TENSION_a_THERM1_COEF_2];	p->ad.comp_t1[2] = u.f;  /* 19 Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
u.ui = ptbl[TENSION_a_THERM1_COEF_3];	p->ad.comp_t1[3] = u.f;  /* 20 Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
u.ui = ptbl[TENSION_a_THERM2_COEF_0];	p->ad.comp_t2[0] = u.f;	 /* 21 Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
u.ui = ptbl[TENSION_a_THERM2_COEF_1];	p->ad.comp_t2[1] = u.f;	 /* 22 Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
u.ui = ptbl[TENSION_a_THERM2_COEF_2];	p->ad.comp_t2[2] = u.f;	 /* 23 Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
u.ui = ptbl[TENSION_a_THERM2_COEF_3];	p->ad.comp_t2[3] = u.f;	 /* 24 Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
p->hbct            = ptbl[TENSION_a_HEARTBEAT_CT]; 		 /* 25 Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs */
p->drum            = ptbl[TENSION_a_DRUM_NUMBER]; 		 /* 26 Tension: drum: Drum system number for this function instance */
p->f_pollbit       = ptbl[TENSION_a_DRUM_FUNCTION_BIT]; 	 /* 27 Tension: bit: f_pollbit: Drum system poll 1st byte bit for function instance */
p->p_pollbit       = ptbl[TENSION_a_DRUM_POLL_BIT]; 		 /* 28 Tension: bit: p_pollbit: Drum system poll 2nd byte bit for this type of function */
p->cid_ten_msg     = ptbl[TENSION_a_CANPRM_TENSION]; 		 /* 29 Tension: CANID: cid_ten_msg:  canid msg Tension */
p->cid_ten_poll    = ptbl[TENSION_a_MSG_TIME_POLL]; 		 /* 30 Tension: CANID: cid_ten_msg:  canid MC: Time msg/Group polling */
p->cid_gps_sync    = ptbl[TENSION_a_TIMESYNC]; 			 /* 31 Tension: CANID: cid_gps_sync: canid time: GPS time sync distribution */
p->cid_heartbeat   = ptbl[TENSION_a_HEARTBEAT]; 		 /* 32 Tension: CANID: Heartbeat msg */
p->cid_tst_ten_a   = ptbl[TENSION_a_CANIDTEST]; 		 /* 33 Tension: CANID: Test */
p->iir[0].k        = ptbl[TENSION_a_IIR_POLL_K]; 		 /* 34 Tension: Divisor for IIR filter: polled msg */
p->iir[0].scale    = ptbl[TENSION_a_IIR_POLL_SCALE]; 		 /* 35 Tension: Scale for IIR filter: polled msg */
p->iir[1].k        = ptbl[TENSION_a_IIR_HB_K]; 			 /* 36 Tension: Divisor for IIR filter: heart-beat */
p->iir[1].scale    = ptbl[TENSION_a_IIR_HB_SCALE]; 	 	 /* 37 Tension: Scale for IIR filter: heart-beat */
p->useme           = ptbl[TENSION_a_USEME]; 			 /* 38 Tension: Bits for using this instance */
p->iir_z_recal.k    = ptbl[TENSION_a_IIR_Z_RECAL_K]; 		 /* 39 Tension: IIR Filter factor: divisor sets time constant: zero recalibration */
p->iir_z_recal.scale= ptbl[TENSION_a_IIR_Z_RECAL_SCALE]; 	 /* 40 Tension: IIR Filter scale : upscaling (due to integer math): zero recalibration */
p->z_recal_ct      = ptbl[TENSION_a_Z_RECAL_CT];		 /* 41 Tension: ADC conversion counts between zero recalibrations */
u.ui = ptbl[TENSION_a_LIMIT_HI];	p->limit_hi = u.f;	 /* 42 Tension: Exceeding this limit (+) means invalid reading */
u.ui = ptbl[TENSION_a_LIMIT_LO];	p->limit_lo = u.f;	 /* 43 Tension: Exceeding this limit (-) means invalid reading */
p->code_CAN_filt[0] = ptbl[TENSION_a_CANID_HW_FILT1];		 /* 44 Tension: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[TENSION_a_CANID_HW_FILT2];		 /* 45 Tension: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[TENSION_a_CANID_HW_FILT3];		 /* 46 Tension: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[TENSION_a_CANID_HW_FILT4];		 /* 47 Tension: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[TENSION_a_CANID_HW_FILT5];		 /* 48 Tension: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[TENSION_a_CANID_HW_FILT6];		 /* 49 Tension: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[TENSION_a_CANID_HW_FILT7];		 /* 50 Tension: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[TENSION_a_CANID_HW_FILT8];		 /* 51 Tension: CAN ID 8 for setting up CAN hardware filter */

return PARAM_LIST_CT_ENCODER_a;
}


