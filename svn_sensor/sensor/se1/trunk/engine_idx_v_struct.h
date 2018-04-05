/******************************************************************************
* File Name          : engine_idx_v_struct.h
* Date First Issued  : 03/16/2018
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
structs for handling parameters (loaded from high flash upon boot) for the
engine functions:
  Manifold pressure
  RPM
  Throttle
  Temperature #1 (possible added thermistors later)

Naming convention for CANID--"cid" prefix
*/

#include <stdint.h>
#include "common_can.h"
//#include "iir_filter_l.h"
#include "temp_calc_param.h"

#ifndef __ENGINE_IDX_V_STRUCT
#define __ENGINE_IDX_V_STRUCT

/* The parameter list supplies the CAN IDs for the hardware filter setup. 
   These are CANIDs that are loaded into the CAN hardware filter to enable 
   incoming msgs */
#define CANFILTMAX	8	// Max number of CAN IDs in parameter list



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
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
};

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

// Engine throttle function
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

// Engine temperature #1 sensor
struct ENGT1LC
{
	uint32_t size;			 //  0 Number of items in struct
 	uint32_t crc;			 //  1 crc-32 placed by loader
	uint32_t version;		 //  2 struct version number
	uint32_t hbct;			 //  3 Time (ms) between HB msg
 struct THERMPARAM therm;//  4-9 Thermistor temperature conversion constants
	uint32_t cid_msg;     // 10 CANID: Temperature #1, calibrated, response to poll
	uint32_t cid_hb;      // 11 CANID: Temperature #1, calibrated, heartbeat
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter

};

/* This struct is for implmementing as a 'combined' function */
 struct ENGINELC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	uint32_t hbct;			// Time (ms) between HB msg
	uint32_t seg_ct;		// Counts per revolution on engine
	float	press_offset;	// Manifold pressure offset
	float press_scale;	// Manifold pressure scale (inch Hg)
	struct THERMPARAM therm; // Thermistor temperature conversion constants
	uint32_t throttle_lo;	// Throttle ADC reading: closed
	uint32_t throttle_hi;	// Throttle ADC reading: full open
	uint32_t cid_rpmmanfld_r;	// CANID: RPM:Manifold pressure, calibrated
	uint32_t cid_tempthrtl_r;	// CANID: Temperature:Throttle, calibrated
	uint32_t cid_hb_rpmmanfld;	// CANID: Heartbeat: RPM:Manifold pressure, raw readings
	uint32_t cid_hb_thermthrtl;// CANID: Heartbeat: Thermistor:Throttle, raw readings
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
 };
 
/* **************************************************************************************/
 int engine_idx_v_struct_copy_eng_man(struct ENGMANLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
 int engine_idx_v_struct_copy_eng_rpm(struct ENGRPMLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
 int engine_idx_v_struct_copy_eng_throttle(struct ENGTHROTTLELC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
 int engine_idx_v_struct_copy_eng_t1(struct ENGT1LC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
 int engine_idx_v_struct_copy(struct ENGINELC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 * ************************************************************************************** */

#endif
