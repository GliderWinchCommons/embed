/******************************************************************************
* File Name          : engine_function.c
* Date First Issued  : 03/23/2018
* Board              : f103
* Description        : Multiple engine functions 
*******************************************************************************/

#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "engine_idx_v_struct.h"
#include "db/gen_db.h"
//#include "tim3_ten2.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "engine_function.h"
#include "rpmsensor.h"
#include "adcsensor_eng.h"
#include "temp_calc_param.h"
#include "PODpinconfig.h"
#include "poly_compute.h"
//#include "p1_initialization.h"
#include "can_driver_filter.h"
#include "cmd_code_dispatch.h"

#include "can_msg_reset.h"
#include "CAN_poll_loop.h"

#include "can_driver.h"

#define TENAQUEUESIZE	3	// Queue size for passing values between levels

	union FTINT	// Union for sending float as 4 byte uint32_t
	{
		uint32_t ui;
		float ft;
	};

/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to flash area with parameters  */
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash2;	// High flash address of 2nd parameter table (.ld defined)
extern void* __paramflash3;	// High flash address of 2nd parameter table (.ld defined)
extern void* __paramflash4;	// High flash address of 2nd parameter table (.ld defined)

/* Holds parameters and associated computed values and readings for each instance. */
struct ENG_MAN_FUNCTION eman_f;
struct ENG_RPM_FUNCTION erpm_f;
struct ENG_THR_FUNCTION ethr_f;
struct ENG_T1_FUNCTION et1_f;

const uint32_t* pparamflash[NUMENGINEFUNCTIONS] = { \
 (uint32_t*)&__paramflash1,	/* Manifold pressure */
 (uint32_t*)&__paramflash2,	/* RPM               */
 (uint32_t*)&__paramflash3,	/* Throttle          */
 (uint32_t*)&__paramflash4,	/* Temperature #1    */
};


/* Highflash command CAN id table lookup mapping. */
const uint32_t myfunctype[NUMENGINEFUNCTIONS] = { \
 FUNCTION_TYPE_ENG_MANIFOLD,
 FUNCTION_TYPE_ENG_RPM,
 FUNCTION_TYPE_ENG_THROTTLE,
 FUNCTION_TYPE_ENG_T1,
};

/* **************************************************************************************
 * static void send_can_msg(struct CANRCVBUF* pcan,struct COMMONFUNCTION* p); // Heartbeat CAN msg
 * @brief	: Send CAN msg and reset heartbeat counter
 * @param	: pcan = pointer to CAN msg
 * @param	: p = pointer to common function struct
 * ************************************************************************************** */
static void send_can_msg(struct CANRCVBUF* pcan,struct COMMONFUNCTION* p, float flt)
{
	union FINT
	{
		uint8_t uc[4];
		float f;
	} fint;
	fint.f = flt;

	pcan->dlc = 5;	// Payload size: status + 4 bytes float
	pcan->cd.uc[0] = p->status;
	pcan->cd.uc[1] = fint.uc[0];
	pcan->cd.uc[2] = fint.uc[1];
	pcan->cd.uc[3] = fint.uc[2];
	pcan->cd.uc[4] = fint.uc[3];

	// Send CAN msg and reset heartbeat timeout 
	can_hub_send(pcan, p->phub);// Send CAN msg to 'can_hub'
	p->hb_tct = tim4_tim_ticks + p->hb_tdur;	 // Reset heart-beat time duration each time msg sent	
	return;
}
/* *********************************************************** */
/* Initialization: generic function                            */
/* *********************************************************** */
static int function_init(uint32_t n, struct COMMONFUNCTION* p, uint32_t hbct )
{
	u32 i;

	/* Flag for triggering sending of polled message */
	p->flag_msg         = 0;	// Be sure it is reset
	p->status           = 0;
	p->can_msg.cd.uc[0] = 0;	// CAN msg polled status
	p->can_hb.cd.uc[0]  = 0;	// CAN msg hb status

	/* First heartbeat time */
	p->hb_tdur = (hbct * tim4_tim_rate) / 1000;	// Number of timer ticks between heartbeats
	p->hb_tct  = tim4_tim_ticks + p->hb_tdur;	// Set first counter value

	/* Add this function to the "hub-server" msg distribution. */
	p->phub = can_hub_add_func();	// Set up port/connection to can_hub
	if (p->phub == NULL) return -997;	// Failed
	
	/* Find command CAN id for this function in table. (__paramflash0a supplied by .ld file) */
	struct FLASHH2* p0a = (struct FLASHH2*)&__paramflash0a;

	/* Check for reasonable size value in table */
	if ((p0a->size == 0) || (p0a->size > NUMCANIDS2)) return -999;

// TODO get the CAN ID for the ldr from low flash and compare to the loader
// CAN id in this table.

	/* Look up "I" (interrogation/incoming) command CAN ID */	
	/* Check if function type code in the table matches our function */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == myfunctype[n])
		{
			p->pcanid_cmd_func_i = &p0a->slot[i].canid; // Save pointer
			break;
		}
	}
	if (i >= p0a->size)
		return -995;
	/* Look up "R" (response) command CAN ID */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == (myfunctype[n] + CMD_IR_OFFSET))
		{
			p->pcanid_cmd_func_r = &p0a->slot[i].canid; // Save pointer
			return 0;
		}
	}

	return -998;	// Argh! Table size reasonable, but didn't find it.
}
/* **************************************************************************************
 * int engine_functions_init_all(void);
 * @brief	: Initialize all 'engine' functions
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
int engine_functions_init_all(void)
{
/* NOTE: much of the following maps the 'lc' (local copy) of the parameters into elements
   of a struct common to all the winch functions (i.e. manifold, rpm, throttle, temperature 1)
   so that routines can use these parameters via a pointer to the common struct for a
   the winch function (rather than have separate routines for each winch function. 
*/

	int ret;
	int ret2;

	extern void* __paramflash1;
	extern void* __paramflash2;
	extern void* __paramflash3;
	extern void* __paramflash4;

	CAN_poll_loop_init();

	/* Manifold function */
	// Set pointer to table in highflash.  Base address provided by .ld file 
	eman_f.cf.pparamflash = (uint32_t*)&__paramflash1;	/* Manifold pressure */
	// Copy table entries to struct in sram from high flash.
	  ret  = engine_idx_v_struct_copy_eng_man(&eman_f.lc, eman_f.cf.pparamflash);
	// Add CAN IDs to incoming msgs passed by the CAN hardware filter. 
	  ret2 = can_driver_filter_add_param_tbl(&eman_f.lc.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	// Remainder is common to all functions
	 ret |= function_init(0,&eman_f.cf, eman_f.lc.hbct);
//eman_f.cf.hb_tdur = 1500;
	if (ret < 0) return ret;
	// Convert to parameters to double since computations done in doubles
	 eman_f.dpress_offset = eman_f.lc.press_offset;
	 eman_f.dpress_scale  = eman_f.lc.press_scale;
	 eman_f.cf.can_msg.id = eman_f.lc.cid_msg;	// Outgoing CAN id: polled msg
	 eman_f.cf.can_hb.id  = eman_f.lc.cid_hb;		// Outgoing CAN id: heartbeat
	 eman_f.cf.canid_poll = eman_f.lc.code_CAN_filt[0]; // Incoming poll CAN id
	 adc_cic2_init(&eman_f.cf.cic2);	// Init second cic filtering constants


	/* RPM function */
	// Set pointer to table in highflash.  Base address provided by .ld file 
     erpm_f.cf.pparamflash = (uint32_t*)&__paramflash2;	/* Manifold pressure */
	// Copy table entries to struct in sram from high flash.
	  ret  = engine_idx_v_struct_copy_eng_rpm(&erpm_f.lc, erpm_f.cf.pparamflash);
	// Add CAN IDs to incoming msgs passed by the CAN hardware filter. 
	  ret2 = can_driver_filter_add_param_tbl(&erpm_f.lc.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	// Remainder is common to all functions
	  ret = function_init(1,&erpm_f.cf,erpm_f.lc.hbct);
	if (ret < 0) return ret;
	// Additional rpm specific sensing initialization
  	  rpmsensor_init();
 	  erpm_f.cf.can_msg.id = erpm_f.lc.cid_msg;
	  erpm_f.cf.can_hb.id  = erpm_f.lc.cid_hb;
	  erpm_f.cf.canid_poll = erpm_f.lc.code_CAN_filt[0]; // Incoming poll CAN id


	/* Throttle function */
	// Set pointer to table in highflash.  Base address provided by .ld file 
	  ethr_f.cf.pparamflash = (uint32_t*)&__paramflash3;	/* Manifold pressure */
	// Copy table entries to struct in sram from high flash.
	  ret  = engine_idx_v_struct_copy_eng_throttle(&ethr_f.lc, ethr_f.cf.pparamflash);
	// Add CAN IDs to incoming msgs passed by the CAN hardware filter. 
	  ret2 = can_driver_filter_add_param_tbl(&ethr_f.lc.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	// Remainder is common to all functions
	  ret = function_init(2,&ethr_f.cf,ethr_f.lc.hbct);
	if (ret < 0) return ret;
     adc_cic2_init(&ethr_f.cf.cic2);	// Init second cic filtering constants
 	  ethr_f.cf.can_msg.id = ethr_f.lc.cid_msg;
	  ethr_f.cf.can_hb.id  = ethr_f.lc.cid_hb;
	  ethr_f.cf.canid_poll = ethr_f.lc.code_CAN_filt[0]; // Incoming poll CAN id



	/* Temperature #1 function */
	// Set pointer to table in highflash.  Base address provided by .ld file 
	  et1_f.cf.pparamflash = (uint32_t*)&__paramflash4;	/* Manifold pressure */
	// Copy table entries to struct in sram from high flash.
	  ret  = engine_idx_v_struct_copy_eng_t1(&et1_f.lc, et1_f.cf.pparamflash);
	// Add CAN IDs to incoming msgs passed by the CAN hardware filter. 
	  ret2 = can_driver_filter_add_param_tbl(&et1_f.lc.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	// Remainder is common to all functions
	  ret = function_init(3,&et1_f.cf,et1_f.lc.hbct);
	// Temperature computataions are doubles, so convert parameters in floats to doubles
 	  temp_calc_param_dbl_init(&et1_f.thermdbl, &et1_f.lc.therm);
	// Set initial values since not available at first
	  et1_f.cf.flast2 = -99.0;
	  et1_f.dlast = -88.8;
 	  et1_f.cf.can_msg.id = et1_f.lc.cid_msg;
	  et1_f.cf.can_hb.id  = et1_f.lc.cid_hb;
	  et1_f.cf.canid_poll = et1_f.lc.code_CAN_filt[1]; // Incoming poll CAN id
     adc_cic2_init(&et1_f.cf.cic2);	// Init second cic filtering constants


	return ret;
}
/* ######################################################################################
 * int eng_common_poll(struct CANRCVBUF* pcan, struct COMMONFUNCTION* p);
 * @brief	: 'CAN_poll_loop.c' calls this for each of the engine functions
 * @param	; pcan = pointer to CAN msg buffer (incoming msg)
 * @param	: p = pointer to struct with variables and parameters common to all functions
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
uint32_t engDbghbct;

int eng_common_poll(struct CANRCVBUF* pcan, struct COMMONFUNCTION* p)
{
	int ret = 0;

	/* Heartbeat msg timing */
	 if ( ( (int)(tim4_tim_ticks - p->hb_tct) ) > 0  )	// Time to send heart-beat?
	{ // Here, yes.		
engDbghbct += 1;
		/* Send heartbeat and compute next heartbeat time count. */
		send_can_msg(&p->can_hb,p, p->flast1); // Send CAN msg, reset heartbeat cts
		ret = 1;
	}

	/* Check if any incoming msgs were with the call to this routine */
	if (pcan == NULL) return ret;	// No incoming msgs (just heartbeat polling)

	if (p->flag_msg != 0) // Poll msg request?
	{ // Here, yes.  Send regular reading response
		p->flag_msg  = 0;
	   send_can_msg(&p->can_msg,p,p->flast1); // Send CAN msg, reset heartbeat cts
			return 1;
	}
	
	/* Check for eng_manifold function command. */
	if (pcan->id == *p->pcanid_cmd_func_i)
	{ // Here, we have a command msg for this function instance. 
//$		cmd_code_dispatch(pcan, p); // Handle and send as necessary
		return 0;	// No msgs passed to other ports
	}
	return ret;
}
/*##############################################################################################
 * The following executes under CAN driver interrupt level
 * can_driver.c (CAN RX) -> can_msg_reset.c -> engine_can_msg_poll (and then trigger CAN_poll_loop)
 *############################################################################################## */
static void engine_can_msg_poll(struct CAN_CTLBLOCK* pctl, struct CAN_POOLBLOCK* pblk)
{
	 __attribute__((__unused__))struct CAN_CTLBLOCK* ptemp = pctl;
/* Note: the canid for poll is likely the same for all these functions, but it is possible
   to set different poll canids.
      Since the timing for readings filtering & computation is common, the timer sync reset
   is only called once.
*/
	/* Check for poll msgs and set flags for sending msgs (at lower priority) */
	if (eman_f.cf.canid_poll == pblk->can.id)	// Manifold poll?
	{
		eman_f.cf.flag_msg = 1;	// Show manifold pressure poll response requested
	}
	if (erpm_f.cf.canid_poll == pblk->can.id)	// RPM poll?
	{
		erpm_f.cf.flag_msg = 1; 	// Show rpm poll response requested
	}
	if (ethr_f.cf.canid_poll == pblk->can.id)	// Throttle poll?
	{
		ethr_f.cf.flag_msg = 1;	// Show throttle response requested
	}
	/* Sync/reset TIM4 timing counters to either poll or time sync canid */
	if ((erpm_f.lc.code_CAN_filt[0] == pblk->can.id) ||
       (erpm_f.lc.code_CAN_filt[1] == pblk->can.id) )
	{
		rpmsensor_reset_timer();	// ### Sync/Reset timing ###
	}
	CAN_poll_loop_trigger();	// Trigger low level interrupt poll incoming msgs
	return;
}
/*******************************************************************************
 * void engine_can_msg_poll_init(void);
 * @brief	: Set address for high-priority incoming CAN msg check
 * @param	: pctl = pointer to CAN control block 
********************************************************************************/
void engine_can_msg_poll_init(void)
{
	can_msg_reset_ptr = &engine_can_msg_poll;

	return;
}

