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
#include "tim3_ten2.h"
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

#define TENAQUEUESIZE	3	// Queue size for passing values between levels

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
 * static void setup_can_msg(struct CANRCVBUF* pcan, uint32_t canid, uint8_t status, uint32_t* pv); 
 * @brief	: Setup CAN msg 
 * @param	: pcan = pointer to CAN msg
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent (int32_t, uint32_t, or float)
 * @return	: *pcan is setup.
 * ************************************************************************************** */
static void setup_can_msg(struct CANRCVBUF* pcan, uint32_t canid, uint8_t status, uint32_t* pv)
{
	pcan->id = canid;
	pcan->dlc = 5;			// Set return msg payload count
	pcan->cd.uc[0] = status;
	pcan->cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	pcan->cd.uc[2] = (*pv >>  8);
	pcan->cd.uc[3] = (*pv >> 16);
	pcan->cd.uc[4] = (*pv >> 24);
	return;
}
/* **************************************************************************************
 * static void send_can_msg_man(struct ENG_MAN_FUNCTION* p, struct CANRCVBUF* pcan); 
 * @brief	: Send CAN msg and reset heartbeat counter
 * @param	: p = pointer to engine manifold function struct
 * @param	: pcan = pointer to CAN msg that has been set up
 * ************************************************************************************** */
static void send_can_msg_man(struct ENG_MAN_FUNCTION* p, struct CANRCVBUF* pcan)
{
	// Send CAN msg and reset heartbeat timeout 
	can_hub_send(pcan, p->cf.phub);// Send CAN msg to 'can_hub'
	p->cf.hbct_ticks = (p->lc.hbct * tim3_ten2_rate) / 1000; // Convert ms to timer ticks
	p->cf.hb_t = tim3_ten2_ticks + p->cf.hbct_ticks;	 // Reset heart-beat time duration each time msg sent	
}

/* *********************************************************** */
/* Initialization: generic function                            */
/* *********************************************************** */
static int function_init(uint32_t n, struct COMMONFUNCTION* p, uint32_t hbct )
{
	u32 i;

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (recompute for online update)
	p->hbct_ticks = (hbct * tim3_ten2_rate) / 1000;
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	

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
	int ret;
	int ret2;

	extern void* __paramflash1;
	extern void* __paramflash2;
	extern void* __paramflash3;
	extern void* __paramflash4;

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
	if (ret < 0) return ret;
	
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

	return ret;
}


/* **************************************************************************************
 * int eng_t1_temperature_poll(void);
 * @brief	: Function: eng_t1, temperature #1, w  thermistor-to-temperature conversion
 * @return	: 0 = no update; 1 = temperature readings updated
 * ************************************************************************************** */
/* *** NOTE ***
   This routine is called from 'main' since the thermistor formulas use functions 
   with doubles.
*/

/* Thermistor conversion things. */
static int adc_temp_flag_prev = 0; // Thermistor readings ready counter

double dscale = (1.0 / (1 << 18));	// ADC filtering scale factor (reciprocal multiply)
void toggle_1led(int led);	// Routine is in 'tension.c'
#define NUMBERADCTHERMISTER_TEN 1
int eng_t1_temperature_poll(void)
{
#ifdef thisneedsacompleterewrite
	int i,j;	// One would expect FORTRAN, but alas it is only C
	double therm[NUMBERADCTHERMISTER_TEN];	// Floating pt of thermistors readings

	/* Check if 'adcsensor_tension' has a new, filtered, reading for us. */
	if (adc_temp_flag[0] != adc_temp_flag_prev)
	{ // Here, a new set of thermistor readings are ready
		j = (0x1 & adc_temp_flag_prev);		// Get current double buffer index
		adc_temp_flag_prev = adc_temp_flag[0];	// Reset flag

		/* Convert filtered long long to double. */
		for (i = 0; i < NUMBERADCTHERMISTER_TEN; i++)
		{	
			therm[i] = adc_readings_cic[j][i]; // Convert to double
			therm[i] = therm[i] * dscale;	   // Scale to 0-4095
		}

		/* Convert ADC readings into uncalibrated degrees Centigrade. */
		/* Then, apply calibration to temperature. */
		j = 0;	// Index input of readings
		for (i = 0; i < NUMTENSIONFUNCTIONS; i++)
		{ 
			ten_f[i].thrm[0] = therm[j];	// Raw thermistor ADC reading
			ten_f[i].degX[0] = temp_calc_param_dbl(therm[j++], &ten_f[i].ten_a.ad.tp[0]); // Apply formula
			ten_f[i].degC[0] = compensation_dbl(&ten_f[i].ten_a.ad.comp_t1[0], 4, ten_f[i].degX[0]);// Adjustment

			ten_f[i].thrm[1] = therm[j];
			ten_f[i].degX[1] = temp_calc_param_dbl(therm[j++], &ten_f[i].ten_a.ad.tp[1]);
			ten_f[i].degC[1] = compensation_dbl(&ten_f[i].ten_a.ad.comp_t2[0], 4, ten_f[i].degX[1]);
		}

		// TODO compute AD7799 temperature offset/scale factors
		// Timing: End of main's loop trigger will pick these up before this recomputes.

//toggle_1led(LEDGREEN2);	// Let the puzzled programmer know the ADC stuff is alive
		return 1;	// Let 'main' know there was an update
	}
#endif
	return 0;	// No update
}
/* ######################################################################################
 * int eng_man_poll(struct CANRCVBUF* pcan, struct ENG_MAN_FUNCTION* p);
 * @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer (incoming msg)
 * @param	: p = pointer to struct with "everything" for this instance of engine manifold
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
//extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks
/* *** NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
int eng_man_poll(struct CANRCVBUF* pcan, struct ENG_MAN_FUNCTION* p)
{
	int ret = 0;
	struct CANRCVBUF cantmp;	// For setting up outgoing msgs

#ifdef skipthisoldsectionofcodeuntilupdated
	union FTINT	// Union for sending float as 4 byte uint32_t
	{
		uint32_t ui;
		float ft;
	}ui; ui.ui = 0; // (initialize to get rid of ui.ft warning)

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ten2_ticks - (int)p->hb_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.
		iir_filtered_calib(p, 1);	// Slow (long time constant) filter the reading
		ui.ft = p->fcalib_lgr;		// Float version of calibrated last good reading
		
		/* Send heartbeat and compute next hearbeat time count. */
		//      Args:  CAN id, status of reading, reading pointer instance pointer
		setup_can_msg(&cantmp, p->lc.cid_hb, p->status, &ui.ui); // Setup CAN msg
		send_can_msg_man(p, &cantmp); // Send CAN msg, reset heartbeat cts
		ret = 1;
	}

	/* Check if any incoming msgs. */
	if (pcan == NULL) return ret;

	/* Check for group poll, and send msg if it is for us. */
//$	if (pcan->id == p->ten_a.cid_ten_poll) // Correct ID?
//if (pcan->id == p->ten_a.cid_gps_sync) // ##### TEST #########
if (pcan->id == 0x00400000) // ##### TEST #########
	{ // Here, group poll msg.  Check if poll and function bits are for us
//$		if ( ((pcan->cd.uc[0] & p->ten_a.p_pollbit) != 0) && \
		     ((pcan->cd.uc[1] & p->ten_a.f_pollbit) != 0) )
		{ // Here, yes.  Send our precious msg.
			/* Send tension msg and re-compute next hearbeat time count. */
			//      Args:  CAN id, status of reading, reading pointer instance pointer
			setup_can_msg(&cantmp, p->lc.cid_msg, p->status, &ui.ui); // Setup CAN msg
		   send_can_msg_man(p, &cantmp); // Send CAN msg, reset heartbeat cts
 
			return 1;
		}
	}
	
	/* Check for eng_manifold function command. */
	if (pcan->id == *p->cf.pcanid_cmd_func_i)
	{ // Here, we have a command msg for this function instance. 
		cmd_code_dispatch(pcan, p); // Handle and send as necessary
		return 0;	// No msgs passed to other ports
	}
#endif
	return ret;
}

