/******************************************************************************
* File Name          : pwrbox_idx_v_struct.c
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
#include "pwrbox_idx_v_struct.h"
#include "pwrbox_function.h"
#include "db/gen_db.h"

#define NULL 0
/*
('PWRBOX1','PWRBOX_CRC'              ,'0',     'PWRBOX',' 1 Pwrbox1: CRC');
('PWRBOX1','PWRBOX_VERSION'          ,'1',     'PWRBOX',' 2 Pwrbox1: Version number');
('PWRBOX1','PWRBOX_HEARTBEAT_TIME_CT','1000',  'PWRBOX',' 3 Pwrbox1: Time (ms) between HB msg');
-- ADC calibrated = (adc count - offset)*scale
('PWRBOX1','PWRBOX_CAL_OFFSET1'     ,'0.0'        ,'PWRBOX',' 4 Pwrbox1: ADC reading 1 offset');
('PWRBOX1','PWRBOX_CAL_SCALE1'      ,'0.200000000','PWRBOX',' 5 Pwrbox1: ADC reading 1  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET2'     ,'0.0'        ,'PWRBOX',' 6 Pwrbox1: ADC reading 2 offset');
('PWRBOX1','PWRBOX_CAL_SCALE2'      ,'0.002119649','PWRBOX',' 7 Pwrbox1: ADC reading 2  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET3'     ,'0.0'        ,'PWRBOX',' 8 Pwrbox1: ADC reading 3 offset');
('PWRBOX1','PWRBOX_CAL_SCALE3'      ,'0.002272421','PWRBOX',' 9 Pwrbox1: ADC reading 3  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET4'     ,'0.0'        ,'PWRBOX','10 Pwrbox1: ADC reading 4 offset');
('PWRBOX1','PWRBOX_CAL_SCALE4'      ,'0.003932551','PWRBOX','11 Pwrbox1: ADC reading 4  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET5'     ,'0.0'        ,'PWRBOX','12 Pwrbox1: ADC reading 5 offset');
('PWRBOX1','PWRBOX_CAL_SCALE5'      ,'0.004029455','PWRBOX','13 Pwrbox1: ADC reading 5  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET6'     ,'0.0'        ,'PWRBOX','14 Pwrbox1: ADC reading 6 offset');
('PWRBOX1','PWRBOX_CAL_SCALE6'      ,'0.003977732','PWRBOX','15 Pwrbox1: ADC reading 6  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET7'     ,'0.0'        ,'PWRBOX','16 Pwrbox1: ADC reading 7 offset');
('PWRBOX1','PWRBOX_CAL_SCALE7'      ,'0.200000000','PWRBOX','17 Pwrbox1: ADC reading 7  scale');
('PWRBOX1','PWRBOX_CAL_OFFSET8'     ,'0.0'        ,'PWRBOX','18 Pwrbox1: ADC reading 8 offset');
('PWRBOX1','PWRBOX_CAL_SCALE8'      ,'0.200000000','PWRBOX','19 Pwrbox1: ADC reading 8  scale');

('PWRBOX1','PWRBOX_IIR1_K'     ,'10'  ,'PWRBOX','20 Pwrbox1: IIR1 Filter factor');
('PWRBOX1','PWRBOX_IIR1_SCALE' ,'128' ,'PWRBOX','21 Pwrbox1: IIR1 Filter scale ');
('PWRBOX1','PWRBOX_IIR2_K'     ,'10'  ,'PWRBOX','22 Pwrbox1: IIR2 Filter factor');
('PWRBOX1','PWRBOX_IIR2_SCALE' ,'128' ,'PWRBOX','23 Pwrbox1: IIR2 Filter scale ');
('PWRBOX1','PWRBOX_IIR3_K'     ,'10'  ,'PWRBOX','24 Pwrbox1: IIR3 Filter factor');
('PWRBOX1','PWRBOX_IIR3_SCALE' ,'128' ,'PWRBOX','25 Pwrbox1: IIR3 Filter scale ');
('PWRBOX1','PWRBOX_IIR4_K'     ,'10'  ,'PWRBOX','26 Pwrbox1: IIR4 Filter factor');
('PWRBOX1','PWRBOX_IIR4_SCALE' ,'128' ,'PWRBOX','27 Pwrbox1: IIR4 Filter scale ');

('PWRBOX1','PWRBOX_HB_R'    ,'CANID_HB_PWRBOX1' ,'PWRBOX','28 Pwrbox: CANID: Heartbeat: input voltage, bus voltage');
('PWRBOX1','PWRBOX_MSG_R'   ,'CANID_MSG_PWRBOX1','PWRBOX','29 Pwrbox: CANID: Msg: input voltage, bus voltage ');
('PWRBOX1','PWRBOX_ALARM_R' ,'CANID_ALM_PWRBOX1','PWRBOX','30 Pwrbox: CANID: Alarm: input voltage, bus voltage');
('PWRBOX1','PWRBOX_ALARM_RATE'  ,'50'           ,'PWRBOX','31 Pwrbox: Time (ms) between alarm msgs, when below threshold');
('PWRBOX1','PWRBOX_ALARM_THRES' ,'10.5'         ,'PWRBOX','32 Pwrbox: Voltage threshold for alarm msgs');

-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
('PWRBOX1','PWRBOX_CANID_HW_FILT1','CANID_HB_TIMESYNC'  ,'PWRBOX','33 EPS_1: CANID 1: GPS time sync distribution msg');
('PWRBOX1','PWRBOX_CANID_HW_FILT2','CANID_MSG_TIME_POLL','PWRBOX','34 MC: CANID 2:Time msg/Group polling');
('PWRBOX1','PWRBOX_CANID_HW_FILT3','CANID_DUMMY'        ,'PWRBOX','35 Pwrbox1: CANID 3: command');
('PWRBOX1','PWRBOX_CANID_HW_FILT4','CANID_DUMMY'        ,'PWRBOX','36 Pwrbox1: CANID 4 add CAN hw filter for incoming msg');
('PWRBOX1','PWRBOX_CANID_HW_FILT5','CANID_DUMMY'        ,'PWRBOX','37 Pwrbox1: CANID 5 add CAN hw filter for incoming msg');
('PWRBOX1','PWRBOX_CANID_HW_FILT6','CANID_DUMMY'        ,'PWRBOX','38 Pwrbox1: CANID 6 add CAN hw filter for incoming msg');
('PWRBOX1','PWRBOX_CANID_HW_FILT7','CANID_DUMMY'        ,'PWRBOX','39 Pwrbox1: CANID 7 add CAN hw filter for incoming msg');
('PWRBOX1','PWRBOX_CANID_HW_FILT8','CANID_DUMMY'        ,'PWRBOX','40 Pwrbox1: CANID 8 add CAN hw filter for incoming msg');
*/
/*
 struct PWRBOXLC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	uint32_t hbct;			// Heartbeat ct: ticks between sending msgs
	struct ADCCALPWRBOX adc[NUMADCPARAM];	// ADC measurements
	struct IIR_L_PARAM iir[NIIR];	   // IIR Filter for IIR filters 
	uint32_t cid_heartbeat; // CANID-Heartbeat msg
	uint32_t cid_pwr_msg;   // CANID-voltage msg
	uint32_t cid_pwr_alarm; // CANID-low voltage alarm msg
   uint32_t alarm_rate;    // Time between alarm msgs (ms)
   float  alarm_thres;     // Voltage threshold for low volt alarm msgs    
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
 };
 
*/

/* **************************************************************************************
 * int pwrbox_idx_v_struct_copy2(struct PWRBOXLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int pwrbox_idx_v_struct_copy(struct PWRBOXLC* p, uint32_t* ptbl)
{
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;

p->size            = ptbl[0];		 			           /*  0 Pwrbox: Number of elements in the following list */
p->crc             = ptbl[PWRBOX_CRC];		 	        /*  1 Pwrbox: CRC for tension list */
p->version         = ptbl[PWRBOX_VERSION];		     /*  2 Pwrbox: Version number */	
p->hbct            = ptbl[PWRBOX_HEARTBEAT_TIME_CT]; /* 3 Pwrbox1: Time (ms) between HB msg */

u.ui = ptbl[PWRBOX_CAL_OFFSET1];  p->adc[0].offset = u.f; /* 4 Pwrbox: ADC reading 1 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE1];   p->adc[0].scale  = u.f; /* 5 Pwrbox: ADC reading 1  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET2];  p->adc[1].offset = u.f; /* 6 Pwrbox: ADC reading 2 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE2];   p->adc[1].scale  = u.f; /* 7 Pwrbox: ADC reading 2  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET3];  p->adc[2].offset = u.f; /* 8 Pwrbox: ADC reading 3 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE3];   p->adc[2].scale  = u.f; /* 9 Pwrbox: ADC reading 3  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET4];  p->adc[3].offset = u.f; /*10 Pwrbox: ADC reading 4 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE4];   p->adc[3].scale  = u.f; /*11 Pwrbox: ADC reading 4  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET5];  p->adc[4].offset = u.f; /*12 Pwrbox: ADC reading 5 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE5];   p->adc[4].scale  = u.f; /*13 Pwrbox: ADC reading 5  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET6];  p->adc[5].offset = u.f; /*14 Pwrbox: ADC reading 6 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE6];   p->adc[5].scale  = u.f; /*15 Pwrbox: ADC reading 6  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET7];  p->adc[6].offset = u.f; /*16 Pwrbox: ADC reading 7 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE7];   p->adc[6].scale  = u.f; /*17 Pwrbox: ADC reading 7  scale */
u.ui = ptbl[PWRBOX_CAL_OFFSET8];  p->adc[7].offset = u.f; /*18 Pwrbox: ADC reading 8 offset */
u.ui = ptbl[PWRBOX_CAL_SCALE8];   p->adc[7].scale  = u.f; /*19 Pwrbox: ADC reading 8  scale */

p->iir[0].k     = ptbl[PWRBOX_IIR0_K];     /* 20 Pwrbox: IIR1 Filter factor */
p->iir[0].scale = ptbl[PWRBOX_IIR0_SCALE]; /* 21 Pwrbox: IIR1 Filter scale  */
p->iir[1].k     = ptbl[PWRBOX_IIR1_K];     /* 22 Pwrbox: IIR2 Filter factor */
p->iir[1].scale = ptbl[PWRBOX_IIR1_SCALE]; /* 23 Pwrbox: IIR2 Filter scale  */
p->iir[2].k     = ptbl[PWRBOX_IIR2_K];     /* 24 Pwrbox: IIR3 Filter factor */
p->iir[2].scale = ptbl[PWRBOX_IIR2_SCALE]; /* 25 Pwrbox: IIR3 Filter scale  */
p->iir[3].k     = ptbl[PWRBOX_IIR3_K];     /* 26 Pwrbox: IIR4 Filter factor */
p->iir[3].scale = ptbl[PWRBOX_IIR3_SCALE]; /* 27 Pwrbox: IIR4 Filter scale  */

p->cid_heartbeat = ptbl[PWRBOX_HB_R];		 /* 28 CANID-Heartbeat msg     */
p->cid_pwr_msg   = ptbl[PWRBOX_MSG_R];		 /* 29 CANID-voltage msg       */
p->cid_pwr_alarm = ptbl[PWRBOX_ALARM_R];   /* 30 CANID-low voltage alarm msg */
p->alarmct       = ptbl[PWRBOX_ALARM_RATE]; /* 31 Pwrbox: Time (ms) between alarm msgs, when below threshold */
u.ui = ptbl[PWRBOX_ALARM_THRES];  p->alarm_thres = u.f;/* 32 Voltage threshold for low volt alarm msgs  */

p->code_CAN_filt[0] = ptbl[PWRBOX_CANID_HW_FILT1];		 /* 44 Tension: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[PWRBOX_CANID_HW_FILT2];		 /* 45 Tension: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[PWRBOX_CANID_HW_FILT3];		 /* 46 Tension: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[PWRBOX_CANID_HW_FILT4];		 /* 47 Tension: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[PWRBOX_CANID_HW_FILT5];		 /* 48 Tension: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[PWRBOX_CANID_HW_FILT6];		 /* 49 Tension: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[PWRBOX_CANID_HW_FILT7];		 /* 50 Tension: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[PWRBOX_CANID_HW_FILT8];		 /* 51 Tension: CAN ID 8 for setting up CAN hardware filter */

return PARAM_LIST_CT_PWRBOX;
}


