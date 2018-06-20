/******************************************************************************
* File Name          : pwrbox_function.c
* Date First Issued  : 06/11/2018
* Board              : f103
* Description        : Power monitoring
*******************************************************************************/

/*
06/11/2018 Hack of tension_functionS
05/29/2016
*/
#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "pwrbox_idx_v_struct.h"
#include "db/gen_db.h"
#include "tim3_ten2.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "pwrbox_function.h"
#include "adcsensor_pwrbox.h"
#include "temp_calc_param.h"
#include "PODpinconfig.h"
#include "poly_compute.h"
#include "p1_initialization.h"
#include "can_driver_filter.h"
#include "cmd_code_dispatch.h"

#define TENAQUEUESIZE	3	// Queue size for passing values between levels

#define NUMPWRBOXFUNCTIONS 1

/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to flash area with parameters  */
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash2;	// High flash address of 2nd parameter table (.ld defined)

/* Holds parameters and associated computed values and readings for each instance. */
struct PWRBOXFUNCTION pwr_f;

/* Base pointer adjustment for idx->struct table. */
struct PWRBOXLC* plc;

/* Highflash command CAN id table lookup mapping. */
const uint32_t myfunctype = {FUNCTION_TYPE_PWRBOX};
/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct PWRBOXFUNCTION* p); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * @param	: p = pointer to a bunch of things for this function instance
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct PWRBOXFUNCTION* p)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, p->phub_pwrbox);// Send CAN msg to 'can_hub'
	p->hbct_ticks = (p->pwrbox.hbct * tim3_ten2_rate) / 1000; // Convert ms to timer ticks
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	 // Reset heart-beat time duration each time msg sent
	return;
}

/* **************************************************************************************
 * int pwrbox_function_init_all(void);
 * @brief	: Initialize all 'tension_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = "r" command can id for this function was not found
 *		: -997 = Add CANHUB failed
 *		: -996 = Adding CAN IDs to hw filter failed
 *		: -995 = "i" command can id not found for this function
 *
 * static int tension_a_functionS_init(int n, struct PWRBOXFUNCTION* p );
 * @brief	: Initialize all 'tension_a' functions
 * @param	: n = instance index
 * @param	: p = pointer to things needed for this function
 * @return	: Same as above
 * ************************************************************************************** */
//  Declaration
static int pwrbox_function_init(struct PWRBOXFUNCTION* p );
/* *************************************************** */
/* Init all instances of tension_a_function supported. */
/* *************************************************** */
int pwrbox_function_init_all(void)
{
	return pwrbox_function_init(&pwr_f);
}
/* *********************************************************** */
/* Do the initialization mess for a single function. */
/* *********************************************************** */
static int pwrbox_function_init(struct PWRBOXFUNCTION* p )
{
	int ret;
	int ret2;
	u32 i;

	/* Set pointer to table in highflash.  Base address provided by .ld file */
// TODO routine to find latest updated table in this flash section
	p->pparamflash = (uint32_t*)&__paramflash1;

	/* Copy table entries to struct in sram from high flash. */
	ret = pwrbox_idx_v_struct_copy(&p->pwrbox, (uint32_t*)&__paramflash1);
	
	/* Convert parameters stated as floats to uints, scaled */
	for (i = 0; i < NUMADCPARAM; i++)
	{
		p->ical[i] = (p->pwrbox.adc[i].scale * (1<<CALIBSCALEB)); // Float to uinit
	}

	/* Set IIR filters with parameter pointer */
	
	for (i = 0; i < NIIR; i++)
	{
		p->adc_iir[i].pprm = &p->pwrbox.iir[i]; // Parameter pointer
		p->adc_iir[i].sw = 0;	// Init switch
	}

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (recompute for online update)
	p->hbct_ticks = (p->pwrbox.hbct * tim3_ten2_rate) / 1000;
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	

	/* Add this function to the "hub-server" msg distribution. */
	p->phub_pwrbox = can_hub_add_func();	// Set up port/connection to can_hub
	if (p->phub_pwrbox == NULL) return -997;	// Failed

	/* Add CAN IDs to incoming msgs passed by the CAN hardware filter. */ 
	ret2 = can_driver_filter_add_param_tbl(&p->pwrbox.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	
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
		if (p0a->slot[i].func == myfunctype)
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
		if (p0a->slot[i].func == (myfunctype + CMD_IR_OFFSET))
		{
			p->pcanid_cmd_func_r = &p0a->slot[i].canid; // Save pointer
			return ret;
		}
	}

	return -998;	// Argh! Table size reasonable, but didn't find it.
}

/* **************************************************************************************
 * double iir_filtered_calib(struct PWRBOXFUNCTION* p, uint32_t n); 
 * @brief	: Convert integer IIR filtered value to offset & scaled double
 * @param	: p = pointer to struct with "everything" for this AD7799
 * @return	: offset & scaled
 * ************************************************************************************** */
#ifdef IMPLMENTIIRFILTEREDCALIBRATION
double iir_filtered_calib(struct PWRBOXFUNCTION* p, uint32_t n)
{
	double d;
	double s;
	double dcal;

	/* Scale filter Z^-1 accumulator. */
	int32_t tmp = p->iir_filtered[n].z / p->iir_filtered[n].pprm->scale;
	
	/* Apply offset. */
	tmp += p->pwr.ad.offset;
	
	/* Convert to double and scale. */
	d = tmp; 		// Convert scaled, filtered int to double
	s = p->pwr.ad.scale; 	// Convert float to double
	dcal = d * s;		// Calibrated reading
	p->dcalib_lgr = dcal;	// Save calibrated last good reading
	p->fcalib_lgr = p->dcalib_lgr; // Save float version for CAN bus

	/* Set up status byte for reading. */
	p->status_byte = STATUS_TENSION_BIT_NONEW; // Show no new readings
	
	/* New readings flag. */
	if (p->readingsct != p->readingsct_lastpoll)
	{ // Here, there was a new reading since last poll
		p->status_byte = 0;	// Turn (all) flags off
	}

	return dcal;		// Return scaled value
}
#endif
/* ######################################################################################
 * int pwrbox_function_poll(struct CANRCVBUF* pcan, struct PWRBOXFUNCTION* p);
 * @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of tension_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
//extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks
/* *** NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
int pwrbox_function_poll(struct CANRCVBUF* pcan, struct PWRBOXFUNCTION* p)
{
	int ret = 0;
	union FTINT	// Union for sending float as 4 byte uint32_t
	{
		uint32_t ui;
		float ft;
	}ui; ui.ui = 0; // (initialize to get rid of ui.ft warning)

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ten2_ticks - (int)p->hb_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.
//$		iir_filtered_calib(p, 1);	// Slow (long time constant) filter the reading
//$		ui.ft = p->fcalib_lgr;		// Float version of calibrated last good reading
		
		/* Send heartbeat and compute next hearbeat time count. */
		//      Args:  CAN id, status of reading, reading pointer instance pointer
//$		send_can_msg(p->pwrbox.cid_heartbeat, p->status_byte, &ui.ui, p); 
//$		ret = 1;
	}

	/* Check if any incoming msgs. */
	if (pcan == NULL) return ret;

	/* Check for group poll, and send msg if it is for us. */
//$	if (pcan->id == p->pwr.cid_ten_poll) // Correct ID?
//if (pcan->id == p->pwr.cid_gps_sync) // ##### TEST #########
if (pcan->id == 0x00400000) // ##### TEST #########
	{ // Here, group poll msg.  Check if poll and function bits are for us
//$		if ( ((pcan->cd.uc[0] & p->pwr.p_pollbit) != 0) && \
		     ((pcan->cd.uc[1] & p->pwr.f_pollbit) != 0) )
		{ // Here, yes.  Send our precious msg.
			/* Send tension msg and re-compute next hearbeat time count. */
			//      Args:  CAN id, status of reading, reading pointer instance pointer
//			send_can_msg(p->pwrbox.cid_pwr_msg, p->status_byte, &ui.ui, p); 
			return 1;
		}
	}
	
	/* Check for function command. */
	if (pcan->id == *p->pcanid_cmd_func_i)
	{ // Here, we have a command msg for this function instance. 
		cmd_code_dispatch(pcan, p); // Handle and send as necessary
		return 0;	// No msgs passed to other ports
	}
	return ret;
}

