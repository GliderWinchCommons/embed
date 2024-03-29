/******************************************************************************
* File Name          : shaft_idx_v_struct.h
* Date First Issued  : 04/21/2018
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
*/

#include <stdint.h>
#include "common_can.h"

#ifndef __SHAFT_IDX_V_STRUCT
#define __SHAFT_IDX_V_STRUCT


/* The parameter list supplies the CAN IDs for the hardware filter setup. */
#define CANFILTMAX	8	// Max number of CAN IDs in parameter list

// Naming convention--"cid" - CAN ID
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
	uint32_t num_seg;          // 12 Number of segments on code wheel
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
	uint32_t iir_k_hb;			// 21 Filter factor: divisor sets time constant: hb msg
   uint32_t iir_scale_hb;		// 22 Filter scale : upscaling: hb msg
 };
 
/* **************************************************************************************/
int shaft_idx_v_struct_copy_shaft(struct SHAFTLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */

#endif
