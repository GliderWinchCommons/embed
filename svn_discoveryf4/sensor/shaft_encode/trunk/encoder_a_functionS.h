/******************************************************************************
* File Name          : encoder_a_functionS.h
* Date First Issued  : 07/14/2017
* Board              : Discovery f4
* Description        : Encoder function  Capital "S" for plural!
*******************************************************************************/

#ifndef __ENCODER_A_FUNCTIONS
#define __ENCODER_A_FUNCTIONS

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "encoder_idx_v_struct.h"
#include "queue_dbl.h"
#include "encoder_timers.h"

#define CMD_IR_OFFSET 1000	// Command CAN function ID table offset for "R" CAN ID


/* This is the working struct for *each* ADD7799. */
struct ENCODERAFUNCTION
{
	/* The following is the sram copy of the fixed (upper flash) parameters */
	struct ENCODERALC enc_a;	// Flash table copied to sram struct
	/* The following are working/computed values */
	struct ENCODERCOMPUTE enc;	// Current readings for this encoder
	struct CANHUB* phub_encoder;	// Pointer: CAN hub buffer
	void* pencoder_idx_struct;	// Pointer to table of pointers for idx->struct 
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func_i;	// Pointer into high flash for command can id (incoming)
	uint32_t* pcanid_cmd_func_r;	// Pointer into high flash for command can id (response)
	uint32_t hb_t;			// tim3 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;		// enc_a.hbct (ms) converted to timer tick
	uint8_t unit;			// Encoder unit number: 0 or 1
	uint8_t status_byte;		// Reading status byte
	double	dcalib_lgr;		// (double) Calibrated, last good reading
	float	fcalib_lgr;		// (float) calibrated last good reading
};

/* **************************************************************************************/
int encoder_a_functionS_init_all(void);
/* @brief	: Initialize all 'encoder_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = command can id for this function was not found
 * ************************************************************************************** */
int encoder_a_functionS_poll(struct CANRCVBUF* pcan, struct ENCODERAFUNCTION* p);
/* @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of encoder_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ************************************************************************************** */


/* Holds parameters and associated computed values and readings for each instance. */
extern struct ENCODERAFUNCTION enc_f[NUMENCODERAFUNCTIONS];

#endif 

