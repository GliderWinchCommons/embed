/******************************************************************************
* File Name          : pwrbox_function.h
* Date First Issued  : 06/13/2018
* Board              : f103
* Description        : Power monitoring
*******************************************************************************/

#ifndef __PWRBOX_FUNCTION
#define __PWRBOX_FUNCTION

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "pwrbox_idx_v_struct.h"
#include "iir_filter_l.h"
#include "queue_dbl.h"

#define CMD_IR_OFFSET 1000	// Command CAN function ID table offset for "R" CAN ID

/* Accumulating average (useful for determining offset). */
struct ACCUMAVE
{
	int64_t sum;		// Sum readings
	int32_t n;		// Keep track of number of readings
	int32_t a;		// Computed average
	uint8_t run;		// Switch: 0 = skip; not zero = build average
	uint8_t run_prev;	// Previous state of run
};


/* This is the working struct */
struct PWRBOXFUNCTION
{
	/* The following is the sram copy of the fixed (upper flash) parameters */
	struct PWRBOXLC pwrbox;		// Flash table copied to sram struct
	/* The following are working/computed values */
	struct IIRFILTERL iir_filtered[NIIR]; // IIR filters for one AD7799
	struct CANHUB* phub_pwrbox;	// Pointer: CAN hub buffer
	void* ppwrbox_idx_struct;	// Pointer to table of pointers for idx->struct 
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func_i;	// Pointer into high flash for command can id (incoming)
	uint32_t* pcanid_cmd_func_r;	// Pointer into high flash for command can id (response)
	uint32_t hb_t;			// tim3 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;		// ten_a.hbct (ms) converted to timer ticks
};

/* **************************************************************************************/
int pwrbox_function_init_all(void);
/* @brief	: Initialize all 'tension_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = command can id for this function was not found
 * ************************************************************************************** */
int pwrbox_function_poll(struct CANRCVBUF* pcan, struct PWRBOXFUNCTION* p);
/* @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of powrbox function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ************************************************************************************** */
int pwrbox_function_temperature_poll(void);
/* @brief	: Handler for thermistor-to-temperature conversion, and AD7799 temperature correction.
 * @return	: 0 = no update; 1 = temperature readings updated
 * ************************************************************************************** */


/* Holds parameters and associated computed values and readings for each instance. */
extern struct PWRBOXFUNCTION pwr_f;

#endif 

