/******************************************************************************
* File Name          : shaft_function.c
* Date First Issued  : 04/21/2018
* Board              : f103
* Description        : Multiple shaft functions 
*******************************************************************************/

#include <stdint.h>
#include "can_driver.h"
#include "can_hub.h"
#include "DTW_counter.h"
#include "shaft_idx_v_struct.h"
#include "db/gen_db.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "shaft_function.h"
#include "adcsensor_foto_h.h"
#include "temp_calc_param.h"
#include "poly_compute.h"
#include "can_driver_filter.h"
#include "cmd_code_dispatch.h"

#include "can_msg_reset.h"
#include "CAN_poll_loop.h"
#include "tim4_shaft.h"
#include "iir_filter_l.h"


#define TENAQUEUESIZE	3	// Queue size for passing values between levels

	union FTINT	// Union for sending float as 4 byte uint32_t
	{
		uint32_t ui;
		float ft;
	};
struct IIR_L_PARAM iirprm;
struct IIRFILTERL iirhb;	// IIR filtering of rpm for heartbeat msg


/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to flash area with parameters  */
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash2;	// High flash address of 2nd parameter table (.ld defined)
extern void* __paramflash3;	// High flash address of 2nd parameter table (.ld defined)
extern void* __paramflash4;	// High flash address of 2nd parameter table (.ld defined)

/* Holds parameters and associated computed values and readings for each instance. */
struct SHAFT_FUNCTION   shaft_f;

const uint32_t* pparamflash[NUMSHAFTFUNCTIONS] = { \
 (uint32_t*)&__paramflash1,	/* Shaft */
};


/* Highflash command CAN id table: lookup CAN_CMD ids for this function */
const uint32_t myfunctype[NUMSHAFTFUNCTIONS] = { \
 FUNCTION_TYPE_DRIVE_SHAFT,
};


/* **************************************************************************************
 * static void send_can_msg_reset(struct CANRCVBUF* pcan,struct COMMONFUNCTION* p); // Heartbeat CAN msg
 * @brief	: Send CAN msg and reset heartbeat counter
 * @param	: pcan = pointer to CAN msg
 * @param	: p = pointer to common function struct
 * ************************************************************************************** */
static void send_can_msg_reset(struct CANRCVBUF* pcan,struct COMMONFUNCTION* p)
{
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
	p->hb_tdur = (hbct * tim4_tim_rate_shaft) / 1000;	// Number of timer ticks between heartbeats
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
 * int shaft_functions_init_all(void);
 * @brief	: Initialize all 'drive_shaft' functions
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
int shaft_function_init_all(void)
{
/* NOTE: much of the following maps the 'lc' (local copy) of the parameters into elements
   of a struct common to all the winch functions (i.e. manifold, rpm, throttle, temperature 1)
   so that routines can use these parameters via a pointer to the common struct for a
   the winch function (rather than have separate routines for each winch function. 
*/

	int ret;
	int ret2;

	extern void* __paramflash1;

	ret = CAN_poll_loop_init();
	if (ret != 0) return ret;

	// Set pointer to table in highflash.  Base address provided by .ld file 
	  shaft_f.cf.pparamflash = (uint32_t*)&__paramflash1;

	// Copy table entries to struct in sram from high flash.
	  ret  = shaft_idx_v_struct_copy_shaft(&shaft_f.lc, shaft_f.cf.pparamflash);

	// Copy IIR filter parameters to working filter struct
	iirhb.pprm = & iirprm; // Pointer to fixed parameter pair
	iirhb.sw   = 0;	     // One-time init switch
	// IIR filter fixed parameter pair
	iirprm.k     = shaft_f.lc.iir_k_hb;     // Cut-off factor
	iirprm.scale = shaft_f.lc.iir_scale_hb; // Scale factor for integer math

	// Add CAN IDs to incoming msgs passed by the CAN hardware filter. 
	  ret2 = can_driver_filter_add_param_tbl(&shaft_f.lc.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed

	// Remainder is common to all functions
	  ret |= function_init(0,&shaft_f.cf, shaft_f.lc.hbct);

	if (ret < 0) return ret;

//$	 adc_cic2_init(&shaft_f.cf.cic2);	// Init second cic filtering constants

	shaft_can_msg_poll_init(); // CAN rx msgs come here to check for poll msg

	return ret;
}
/* ######################################################################################
 * int shaft_common_poll(struct CANRCVBUF* pcan, struct SHAFT_FUNCTION* p);
 * @brief	: 'CAN_poll_loop.c' calls this for each of the shaft functions
 * @param	; pcan = pointer to CAN msg buffer (incoming msg)
 * @param	: p = pointer to struct with variables and parameters
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 CALLED from CAN_poll_loop
 * ###################################################################################### */
uint32_t shaftDbghbct;

int shaft_common_poll(struct CANRCVBUF* pcan, struct SHAFT_FUNCTION* p)
{
	int ret = 0;

	/* Heartbeat msg timing */
	 if ( ( (int)(tim4_tim_ticks - p->cf.hb_tct) ) > 0  )	// Time to send heart-beat?
	{ // Here, yes.		
shaftDbghbct += 1;
		/* Send heartbeat and compute next heartbeat time count. */
		send_can_msg_reset(&p->can_hb_count,&p->cf); // Send CAN msg and reset heartbeat cts
		send_can_msg_reset(&p->can_hb_speed,&p->cf); // Send CAN msg and reset heartbeat cts
		ret = 0;
	}

	if (p->cf.flag_msg != 0) // Poll response msg request?
	{ // Here, yes.  Send regular reading response(s)
		p->cf.flag_msg  = 0;	// Reset flag
	   can_hub_send(&p->can_msg_speed,p->cf.phub); //  Send CAN msg
		can_hub_send(&p->can_msg_count,p->cf.phub); //  Send CAN msg
			ret = 1;
	}
	/* Check drive shaft function command. */
	/* Check if any incoming msgs were with the call to this routine */
	if (pcan != NULL)	// No incoming msgs (just heartbeat polling)
	{
		if (pcan->id == *p->cf.pcanid_cmd_func_i)
		{ // Here, we have a command msg for this function instance. 
			cmd_code_dispatch(pcan, p); // Handle and send as necessary
			ret = 0;	// No msgs passed to other ports
		}
	}

	/* Histogram sending */
	// Load can_driver with histogram msgs, if ready & active
	adc_histo_send(&adchisto2); // Load ADC2 histogram 
	adc_histo_send(&adchisto3); // Load ADC3 histogram

	return ret;
}
/*##############################################################################################
 * The following executes under CAN driver interrupt level
 * can_driver.c (CAN RX) -> can_msg_reset.c -> shaft_can_msg_poll (and then trigger CAN_poll_loop)
 *############################################################################################## */
static void shaft_can_msg_poll(struct CAN_CTLBLOCK* pctl, struct CAN_POOLBLOCK* pblk)
{
	 __attribute__((__unused__))struct CAN_CTLBLOCK* ptemp = pctl;
/* Note: the canid for poll is likely the same for all these functions, but it is possible
   to set different poll canids.
      Since the timing for readings filtering & computation is common, the timer sync reset
   is only called once.
*/
	/* Check for poll msgs and set flags for sending msgs (at lower priority) */
	if (shaft_f.lc.code_CAN_filt[0] == pblk->can.id)	// Drive shaft response request msg?
	{ // Here, yes.
		shaft_f.cf.flag_msg = 1;	// Show poll loop that poll response msg was requested
	}

	/* Sync/reset TIM4 timing counters to either poll or time sync canid */
	if ((shaft_f.lc.code_CAN_filt[0] == pblk->can.id) ||
       (shaft_f.lc.code_CAN_filt[1] == pblk->can.id) )
	{
		tim4_shaft_reset_timer();	// ### Sync/Reset timing ###
	}

	CAN_poll_loop_trigger();	// Trigger low level interrupt poll incoming msgs
	return;
}
/*******************************************************************************
 * void shaft_can_msg_poll_init(void);
 * @brief	: Set address for high-priority incoming CAN msg check
 * @param	: pctl = pointer to CAN control block 
********************************************************************************/
void shaft_can_msg_poll_init(void)
{
	can_msg_reset_ptr = (void*)&shaft_can_msg_poll;

	return;
}

