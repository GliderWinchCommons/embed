/******************************************************************************
* File Name          : engine_function.h
* Date First Issued  : 03/24/2018
* Board              : f103
* Description        : engine manifold + rpm + throttle + temperature #1
*******************************************************************************/

#ifndef __ENGINE_FUNCTIONS
#define __ENGINE_FUNCTIONS

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "engine_idx_v_struct.h"
#include "iir_filter_l.h"
#include "queue_dbl.h"

#define NUMENGINEFUNCTIONS 4	// Manifold pressure; RPM; Throttle; Temperature #1

#define CMD_IR_OFFSET 1000	// Command CAN function ID table offset for "R" CAN ID

/* Common to all functions */
struct COMMONFUNCTION
{
	void* p_idx_struct;         // Pointer to table of pointers for idx->struct 
	void* pparamflash;          // Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func_i;// Pointer into high flash for command can id (incoming)
	uint32_t* pcanid_cmd_func_r;// Pointer into high flash for command can id (response)
	uint32_t hb_t;              // tim3 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;        // ten_a.hbct (ms) converted to timer ticks
	struct CANHUB* phub;        // Pointer: CAN hub buffer
	uint8_t flag_msg;           // 1 = send polled msg; 0 = skip
};


/* These are the working structs for the functions */
//
struct ENG_MAN_FUNCTION
{
	struct COMMONFUNCTION cf; // Common to all functions
	struct ENGMANLC lc;	// Flash table copied to sram struct
	double manval;			// Filtered reading converted to double
	uint8_t status;		// Reading status
};

struct ENG_RPM_FUNCTION
{
	struct COMMONFUNCTION cf; // Common to all engine Functions
   struct ENGRPMLC lc;		  // Local Copy of parameters
	uint32_t endtime; 	 // Tim4_gettime_ui() Save current 32b time tick count (time)
	uint32_t endic;       // Tim4_inputcapture_ui() Save the last 32b input capture time and ic counter
	uint32_t endtime_prev;// Previous endtime
	uint32_t endic_prev;  // Previous endic
	uint32_t ct;          // Running count of input captures
	uint32_t ct_prev;     // Previous ic
	double   dk1;         // Scale factor
	double   dprm;        // rpm (double)
	double   frpm;        // rpm (float)
};
struct ENG_THR_FUNCTION
{
	struct COMMONFUNCTION cf; // Common to all functions
   struct ENGTHROTTLELC lc;
	double manthr;			// Filtered reading converted to double
};

struct ENG_T1_FUNCTION
{
	struct COMMONFUNCTION cf; // Common to all functions
   struct ENGT1LC lc;
	double thrm;			// Filtered reading converted to double
	double degX;			// Uncalibrated temperature for each thermistor
};


#ifdef ENGINEFUNCTIONISUSEDNOW
struct ENGINEFUNCTION
{
	/* The following is the sram copy of the fixed (upper flash) parameters */
	struct ENGINELC engine;		// Flash table copied to sram struct
	/* The following are working/computed values */
	struct CANHUB* phub_tension;	// Pointer: CAN hub buffer
	struct ACCUMAVE ave;		// Accumulating average
	double thrm[2];			// Filtered reading converted to double
	double degX[2];			// Uncalibrated temperature for each thermistor
	double degC[2];			// Calibrated temperature for each thermistor
	double ten_iircal[NIIR];	// Tension: filtered and calibrated
	void* ptension_idx_struct;	// Pointer to table of pointers for idx->struct 
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func_i;	// Pointer into high flash for command can id (incoming)
	uint32_t* pcanid_cmd_func_r;	// Pointer into high flash for command can id (response)
	uint32_t hb_t;			// tim3 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;		// ten_a.hbct (ms) converted to timer ticks
	int32_t lgr;			// Last Good Reading (of AD7799)
	int32_t cicraw; 		// CIC filtered w/o any adjustments for one AD7799
	long long cicave;		// Averaged cic readings for nice display
	int32_t offset_reg;		// Last reading of AD7799 offset register
	int32_t offset_reg_filt;	// Last filtered AD7799 offset register
	int32_t offset_reg_rdbk;	// Last filtered AD7799 offset register set read-back
	int32_t fullscale_reg;		// Last reading of AD7799 fullscale register
	uint16_t poll_mask; 		// Mask for first two bytes of a poll msg 		(necessary?)
	uint32_t readingsct;		// Running count of readings (conversions completed)
	uint32_t readingsct_lastpoll;	// Reading count the last time a poll msg sent
	struct IIRFILTERL iir_offset; 	// IIR filter for offsets
	uint32_t offset_ct;		// Running ct of offset updates
	uint32_t offset_ct_prev;	// Previous ct of offset updates
	int16_t zero_flag;		// 1 = zero-calibration operation competed
	struct IIRFILTERL iir_z_recal_w;// Working zero recal filter
	uint32_t z_recal_next;		// Conversion count for next zero recalibration
	uint8_t status_byte;		// Reading status byte
	double	dcalib_lgr;		// (double) Calibrated, last good reading
	float	fcalib_lgr;		// (float) calibrated last good reading
};
#endif

/* **************************************************************************************/
int engine_functions_init_all(void);
/* @brief	: Initialize all 'engine' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = "r" command can id for this function was not found
 *		: -997 = Add CANHUB failed
 *		: -996 = Adding CAN IDs to hw filter failed
 *		: -995 = "i" command can id not found for this function
 *
 * static int tension_a_functionS_init(int n, struct TENSIONFUNCTION* p );
 * @brief	: Initialize all 'tension_a' functions
 * @param	: n = instance index
 * @param	: p = pointer to things needed for this function
 * @return	: Same as above
 * ************************************************************************************** */
 int eng_man_poll(struct CANRCVBUF* pcan, struct ENG_MAN_FUNCTION* p);
/* @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of engine manifold
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ************************************************************************************** */
 int eng_t1_temperature_poll(void);
/* @brief	: Function: eng_t1, temperature #1, w  thermistor-to-temperature conversion
 * @return	: 0 = no update; 1 = temperature readings updated
 * ************************************************************************************** */

/* Holds parameters and associated computed values and readings for each instance. */
extern struct ENG_MAN_FUNCTION eman_f;
extern struct ENG_RPM_FUNCTION erpm_f;
extern struct ENG_THR_FUNCTION ethr_f;
extern struct ENG_T1_FUNCTION et1_f;

#endif 

