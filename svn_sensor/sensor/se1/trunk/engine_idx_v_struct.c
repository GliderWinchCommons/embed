/******************************************************************************
* File Name          : engine_idx_v_struct.c
* Date First Issued  : 03/16/2018
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
#include "engine_idx_v_struct.h"
#include "engine_function.h"
#include "db/gen_db.h"
#include "db/idx_v_val.c"

#define NULL 0

/* **************************************************************************************
 * int engine_idx_v_struct_copy_eng_man(struct ENGMANLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int engine_idx_v_struct_copy_eng_man(struct ENGMANLC* p, uint32_t* ptbl)
{
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;

p->size            = ptbl[0];		 			               /*  0 Eng_manifold: Number of elements in the following list */
p->crc             = ptbl[ENG_MAN_CRC];		 	         /*  1 Eng_manifold: CRC for tension list */
p->version         = ptbl[ENG_MAN_VERSION];		         /*  2 Eng_manifold: Version number */	
p->hbct            = ptbl[ENG_MAN_HEARTBEAT_TIME_CT];    /*  3 Eng_manifold: Time (ms) between HB msg */
u.ui = ptbl[ENG_MAN_PRESS_OFFSET]; p->press_offset = u.f;/*  4 Eng_manifold: Manifold pressure offset */
u.ui = ptbl[ENG_MAN_PRESS_SCALE];  p->press_scale = u.f; /*  5 Eng_manifold: Manifold pressure scale */
p->cid_msg          = ptbl[ENG_MAN_POLL_MANFLD_R];       /*  6 Eng_manifold: Manifold pressure, calibrated, response to poll */
p->cid_hb           = ptbl[ENG_MAN_HB_MANFLD_R];         /*  7 Eng_manifold: Manifold pressure, calibrated, heartbeat */
p->code_CAN_filt[0] = ptbl[ENG_MAN_CANID_HW_FILT1];		/*  8 Eng_manifold: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[ENG_MAN_CANID_HW_FILT2];		/*  9 Eng_manifold: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[ENG_MAN_CANID_HW_FILT3];		/* 10 Eng_manifold: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[ENG_MAN_CANID_HW_FILT4];		/* 11 Eng_manifold: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[ENG_MAN_CANID_HW_FILT5];		/* 12 Eng_manifold: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[ENG_MAN_CANID_HW_FILT6];		/* 13 Eng_manifold: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[ENG_MAN_CANID_HW_FILT7];		/* 14 Eng_manifold: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[ENG_MAN_CANID_HW_FILT8];		/* 15 Eng_manifold: CAN ID 8 for setting up CAN hardware filter */
return ENG1_MANIFOLD_PARAM_SIZE;
}

/* **************************************************************************************
 * int engine_idx_v_struct_copy_eng_rpm(struct ENGRPMLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int engine_idx_v_struct_copy_eng_rpm(struct ENGRPMLC* p, uint32_t* ptbl)
{

p->size             = ptbl[0];		 			         /*  0 Eng_rpm: Number of elements in the following list */
p->crc              = ptbl[ENG_RPM_CRC];		 	      /*  1 Eng_rpm: CRC for tension list */
p->version          = ptbl[ENG_RPM_VERSION];		      /*  2 Eng_rpm: Version number */	
p->hbct             = ptbl[ENG_RPM_HEARTBEAT_TIME_CT];/*  3 Eng_rpm: Time (ms) between HB msg */
p->seg_ct           = ptbl[ENG_RPM_SEG_CT];           /*  4 Eng_rpm: Counts per revolution on engine */
p->cid_msg          = ptbl[ENG_RPM_POLL_RPM_R];       /*  5 Eng_rpm: CANID: Poll response, rpm, calibrated */
p->cid_hb           = ptbl[ENG_RPM_HB_RPM_R];         /*  6 Eng_rpm: CANID: Heartbeat: rpm, calibrated */
p->code_CAN_filt[0] = ptbl[ENG_RPM_CANID_HW_FILT1];   /*  7 Eng_rpm: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[ENG_RPM_CANID_HW_FILT2];   /*  8 Eng_rpm: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[ENG_RPM_CANID_HW_FILT3];   /*  9 Eng_rpm: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[ENG_RPM_CANID_HW_FILT4];   /* 10 Eng_rpm: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[ENG_RPM_CANID_HW_FILT5];   /* 11 Eng_rpm: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[ENG_RPM_CANID_HW_FILT6];   /* 12 Eng_rpm: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[ENG_RPM_CANID_HW_FILT7];   /* 13 Eng_rpm: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[ENG_RPM_CANID_HW_FILT8];   /* 14 Eng_rpm: CAN ID 8 for setting up CAN hardware filter */
return ENG1_RPM_PARAM_SIZE;
}

/* **************************************************************************************
 * int engine_idx_v_struct_copy_eng_throttle(struct ENGTHROTTLELC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int engine_idx_v_struct_copy_eng_throttle(struct ENGTHROTTLELC* p, uint32_t* ptbl)
{

p->size             = ptbl[0];		 			          /*  0 Eng_throttle: Number of elements in the following list */
p->crc              = ptbl[ENG_THR_CRC];		 	       /*  1 Eng_throttle: CRC for tension list */
p->version          = ptbl[ENG_THR_VERSION];		       /*  2 Eng_throttle: Version number */	
p->hbct             = ptbl[ENG_THR_HEARTBEAT_TIME_CT]; /*  3 Eng_throttle: Time (ms) between HB msg */
p->throttle_open    = ptbl[ENG_THR_OPENFULL];          /*  4 Eng_throttle: ADC when throttle full open */
p->throttle_close   = ptbl[ENG_THR_CLOSED];            /*  5 Eng_throttle: ADC when throttle closed */
p->cid_msg          = ptbl[ENG_THR_POLL_THROTTLE_R];   /*  6 Eng_throttle: CANID: Poll response: throttle (0.0-100.0) */
p->cid_hb           = ptbl[ENG_THR_HB_THROTTLE_R];     /*  7 Eng_throttle: CANID: Heartbeat: throttle (0.0-100.0) */
p->code_CAN_filt[0] = ptbl[ENG_THR_CANID_HW_FILT1];    /*  8 Eng_throttle: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[ENG_THR_CANID_HW_FILT2];    /*  9 Eng_throttle: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[ENG_THR_CANID_HW_FILT3];    /* 10 Eng_throttle: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[ENG_THR_CANID_HW_FILT4];    /* 11 Eng_throttle: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[ENG_THR_CANID_HW_FILT5];    /* 12 Eng_throttle: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[ENG_THR_CANID_HW_FILT6];    /* 13 Eng_throttle: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[ENG_THR_CANID_HW_FILT7];    /* 14 Eng_throttle: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[ENG_THR_CANID_HW_FILT8];    /* 15 Eng_throttle: CAN ID 8 for setting up CAN hardware filter */
return ENG1_THROTTLE_PARAM_SIZE;
}

/* **************************************************************************************
 * int engine_idx_v_struct_copy_eng_t1(struct ENGT1LC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int engine_idx_v_struct_copy_eng_t1(struct ENGT1LC* p, uint32_t* ptbl)
{
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;

p->size            = ptbl[0];		 			             /*  0 Eng_t1: Number of elements in the following list */
p->crc             = ptbl[ENG_T1_CRC];		 	          /*  1 Eng_t1: CRC for tension list */
p->version         = ptbl[ENG_T1_VERSION];		       /*  2 Eng_t1: Version number */	
p->hbct            = ptbl[ENG_T1_HEARTBEAT_TIME_CT];   /*  3 Eng_t1: Time (ms) between HB msg */
u.ui = ptbl[ENG_T1_CONST_B];     p->therm.B = u.f;     /*  4 Eng_t1: Thermistor param: constant B */
u.ui = ptbl[ENG_T1_R_SERIES];    p->therm.RS = u.f;    /*  5 Eng_t1: Thermistor param: Series resistor, fixed (K ohms) */
u.ui = ptbl[ENG_T1_R_ROOMTMP];   p->therm.R0 = u.f;    /*  6 Eng_t1: Thermistor param: Thermistor room temp resistance (K ohms) */
u.ui = ptbl[ENG_T1_REF_TEMP];    p->therm.TREF = u.f;  /*  7 Eng_t1: Thermistor param: Reference temp for thermistor */
u.ui = ptbl[ENG_T1_TEMP_OFFSET]; p->therm.offset = u.f;/*  8 Eng_t1: Thermistor param: Thermistor temp offset correction (deg C) */
u.ui = ptbl[ENG_T1_TEMP_SCALE];  p->therm.scale = u.f; /*  9 Eng_t1: Thermistor param: Thermistor temp scale correction */
p->cid_msg          = ptbl[ENG_T1_POLL_T1_R];          /* 10 Eng_t1: CANID: Poll response: temperature #1 (deg C) */
p->cid_hb           = ptbl[ENG_T1_HB_T1_R];            /* 11 Eng_t1: CANID: Heartbeat: temperature #1 (deg C) */
p->code_CAN_filt[0] = ptbl[ENG_T1_CANID_HW_FILT1];		 /* 12 Eng_t1: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[ENG_T1_CANID_HW_FILT2];		 /* 13 Eng_t1: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[ENG_T1_CANID_HW_FILT3];		 /* 14 Eng_t1: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[ENG_T1_CANID_HW_FILT4];		 /* 15 Eng_t1: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[ENG_T1_CANID_HW_FILT5];		 /* 16 Eng_t1: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[ENG_T1_CANID_HW_FILT6];		 /* 17 Eng_t1: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[ENG_T1_CANID_HW_FILT7];		 /* 18 Eng_t1: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[ENG_T1_CANID_HW_FILT8];		 /* 19 Eng_t1: CAN ID 8 for setting up CAN hardware filter */
return ENG1_T1_PARAM_SIZE;
}

/* **************************************************************************************
 * int engine_idx_v_struct_copy(struct ENGINELC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 * ************************************************************************************** */
int engine_idx_v_struct_copy_engine(struct ENGINELC* p, uint32_t* ptbl)
{
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;



/* NOTE: values that are not uint32_t  */
p->size            = ptbl[0];		 			            /*  0 Tension: Number of elements in the following list */
p->crc             = ptbl[ENGINE__CRC];		 	      /*  1 Engine: CRC for tension list */
p->version         = ptbl[ENGINE__VERSION];		      /*  2 Egnine: Version number */	
p->hbct            = ptbl[ENGINE__HEARTBEAT_TIME_CT]; /*  3 Engine: Time (ms) between HB msg */
p->seg_ct			 = ptbl[ENGINE_SENSOR_SEG_CT];      /*  4 Engine_sensor: Counts per revolution on engine */
u.ui = ptbl[ENGINE_SENSOR_PRESS_OFFSET]; p->press_offset = u.f; /*  5 Engine_sensor: Manifold pressure offset */
u.ui = ptbl[ENGINE_SENSOR_PRESS_SCALE];  p->press_offset = u.f; /*  6 Engine_sensor: Manifold pressure  scale (inch Hg) */
u.ui = ptbl[ENGINE_THERM1_CONST_B]; p->therm.B = u.f;	       /*  7 Thermistor constant B */
u.ui = ptbl[ENGINE_THERM1_R_SERIES];p->therm.RS = u.f;       /*  8 Thermistor Series resistor, fixed (K ohms) */
u.ui = ptbl[ENGINE_THERM1_R_ROOMTMP];p->therm.R0 = u.f;      /*  9 Thermistor room temp resistance (K ohms) */
u.ui = ptbl[ENGINE_THERM1_REF_TEMP];p->therm.TREF = u.f;     /* 10 Thermistor Reference temp for thermistor */
u.ui = ptbl[ENGINE_THERM1_TEMP_OFFSET];p->therm.offset = u.f;/* 11 Thermistor Reference temp for thermistor */
u.ui = ptbl[ENGINE_THERM1_TEMP_SCALE];p->therm.scale = u.f;  /* 12 Thermistor temp scale correction */
p->throttle_lo     = ptbl[ENGINE_THROTTLE_lO];        /* 13 Throttle pot ADC reading: closed */
p->throttle_hi     = ptbl[ENGINE_THROTTLE_HI];        /* 14 Throttle pot ADC reading: full open */
p->cid_rpmmanfld_r = ptbl[ENGINE_POLL_RPMMANFLD_R];   /* 15 CANID: Poll response: rpm:manifold, calibrated */
p->cid_tempthrtl_r = ptbl[ENGINE_POLL_TEMPTHRTL_R];   /* 16 CANID: Poll response:temperature:throttle pot, calibrated */
p->cid_hb_rpmmanfld = ptbl[ENGINE_HB_RPMMANFLD];      /* 17 CANID: Heartbeat: raw readings: rpm:manifold pressure */
p->cid_hb_thermthrtl = ptbl[ENGINE_HB_THRMTHRTL];     /* 18 CANID: Heartbeat: raw readings: thermistor:throttle */
p->code_CAN_filt[0] = ptbl[TENSION_a_CANID_HW_FILT1];		 /* 44 Tension: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[TENSION_a_CANID_HW_FILT2];		 /* 45 Tension: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[TENSION_a_CANID_HW_FILT3];		 /* 46 Tension: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[TENSION_a_CANID_HW_FILT4];		 /* 47 Tension: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[TENSION_a_CANID_HW_FILT5];		 /* 48 Tension: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[TENSION_a_CANID_HW_FILT6];		 /* 49 Tension: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[TENSION_a_CANID_HW_FILT7];		 /* 50 Tension: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[TENSION_a_CANID_HW_FILT8];		 /* 51 Tension: CAN ID 8 for setting up CAN hardware filter */

return PARAM_LIST_CT_ENGINE_SENSOR;
}


