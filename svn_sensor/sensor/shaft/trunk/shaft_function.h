/******************************************************************************
* File Name          : shaft_function.h
* Date First Issued  : 04/21/2018
* Board              : f103
* Description        : engine manifold + rpm + throttle + temperature #1
*******************************************************************************/

#ifndef __SHAFT_FUNCTION
#define __SHAFT_FUNCTION

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "shaft_idx_v_struct.h"
#include "iir_filter_l.h"
#include "queue_dbl.h"
#include "cic_filter_l_N2_M3.h"

#define NUMENGINEFUNCTIONS 1	// Just one function for now

#define CMD_IR_OFFSET 1000	// Command CAN function ID table offset for "R" CAN ID

#define PAYSIZE_U8_S32 5	// Payload size: status byte, 4 byte int
#define PAYSIZE_U8_FF  5	// Payload size: status byte, 4 byte float

/* Common to all functions */
struct COMMONFUNCTION
{
	void* p_idx_struct;         // Pointer to table of pointers for idx->struct 
	void* pparamflash;          // Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func_i;// Pointer into high flash for command can id (incoming)
	uint32_t* pcanid_cmd_func_r;// Pointer into high flash for command can id (response)
	uint32_t hb_tct;            // tim3 tick counter for next heart-beat CAN msg
	uint32_t hb_tdur;           // Heartbeat duration (ms) converted to timer ticks
	uint32_t canid_poll;        // Incoming CAN id for poll
	uint32_t ilast1;            // Last reading: first cic filtered 
	float    flast1;            // Last reading: converted to float
	uint32_t ilast2;            // Last reading: second cic filtered 
	float    flast2;            // Last reading: second, converted to float
	int32_t  tmp;               // Temporary
	struct CANHUB* phub;        // Pointer: CAN hub buffer
	struct CANRCVBUF can_msg;   // Latest reading CAN msg: polled
	struct CANRCVBUF can_hb;    // Latest reading CAN msg: heartbeat
	struct CICLN2M3 cic2;       // CIC filtering after initial
	uint8_t status;		       // Reading status
	uint8_t flag_msg;           // 1 = send polled msg; 0 = skip
   uint8_t flag_last1;         // Last reading flag: 1 = new data
};

/* These are the working structs for the functions */
//
struct SHAFT_FUNCTION
{
	struct COMMONFUNCTION cf;	// Items common to most everybody
	struct SHAFTLC lc;		// Local sram copy of database parameters
	double dlast1;			// Filtered reading converted to double
	double dcalibrated;	// 
	
/* These were originally in the.c file */
long	speed_filteredA2;	// Most recent computed & filtered rpm
u32 	encoder_ctrA2;		// Most recent encoder count
struct CICLN2M3 rpm_cic;		// Double buffer CIC intermediate storage
struct CANRCVBUF can_msg_speed;	// CAN msg. Drive Shaft--speed
struct CANRCVBUF can_msg_count;	// CAN msg. Drive Shaft--odometer
struct CANRCVBUF can_hb_speed;	// CAN hearbeat. Drive Shaft--speed
struct CANRCVBUF can_hb_count;	// CAN hearbeat. Drive Shaft--odometer
struct CANRCVBUF can_msg_ER1;	// CAN msg. Errors
struct CANRCVBUF can_msg_ER2;	// CAN msg. Errors
struct CANRCVBUF can_msg_histo31;	// CAN msg. Histogram tx: request count, switch buffers. rx: send count
struct CANRCVBUF can_msg_histo32;	// CAN msg. Histogram tx: bin number, rx: send bin count
struct CANRCVBUF can_msg_histo21;	// CAN msg. Histogram tx: request count, switch buffers; rx send count
struct CANRCVBUF can_msg_histo22;	// CAN msg. Histogram tx: bin number, rx: send bin count
struct CANRCVBUF* can_msg_ptr1;	// 1st can msg pointer
struct CANRCVBUF* can_msg_ptr2;	// 2nd can msg pointer
/* ADC3 ADC2 readings readout */
uint32_t adcreadings_ctr;
struct CANRCVBUF can_msg_adc_5;	// CAN msg. Histogram tx: request count, switch buffers. rx: send count


};



/* **************************************************************************************/
int shaft_function_init_all(void);
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
 int shaft_man_poll(struct CANRCVBUF* pcan, struct SHAFT_FUNCTION* p);
/* @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of engine manifold
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ************************************************************************************** */
int shaft_common_poll(struct CANRCVBUF* pcan, struct COMMONFUNCTION* p);
/* @brief	: 'CAN_poll_loop.c' calls this for each of the engine functions
 * @param	; pcan = pointer to CAN msg buffer (incoming msg)
 * @param	: p = pointer to struct with variables and parameters common to all functions
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
/*******************************************************************************/
void shaft_can_msg_poll_init(void);
/* @brief	: Set address for high-priority incoming CAN msg check
 * @param	: pctl = pointer to CAN control block 
********************************************************************************/



/* Holds parameters and associated computed values and readings for each instance. */
extern struct SHAFT_FUNCTION shaft_f;

#endif 

