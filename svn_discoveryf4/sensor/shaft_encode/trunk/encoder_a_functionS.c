/******************************************************************************
* File Name          : encoder_a_functionS.c
* Date First Issued  : 07/14/2017
* Board              : Discovery f4
* Description        : Encoder function  Capital "S" for plural!
*******************************************************************************/

#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "encoder_idx_v_struct.h"
#include "db/gen_db.h"
//#include "tim3_ten2.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "encoder_a_functionS.h"
#include "temp_calc_param.h"
#include "p1_initialization.h"
#include "can_driver_filter.h"
#include "cmd_code_dispatch.h"

#define ENCAQUEUESIZE	3	// Queue size for passing values between levels

/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to flash area with parameters  */
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash2;	// High flash address of 2nd parameter table (.ld defined)

/* Holds parameters and associated computed values and readings for each instance. */
struct ENCODERAFUNCTION enc_f[NUMTENSIONFUNCTIONS];

const uint32_t* pparamflash[NUMENCODERAFUNCTIONS] = { \
 (uint32_t*)&__paramflash1,
 (uint32_t*)&__paramflash2,
};

/* Base pointer adjustment for idx->struct table. */
struct TENSIONLC* plc[NUMENCODERAFUNCTIONS];

/* Highflash command CAN id table lookup mapping. */
const uint32_t myfunctype[NUMENCODERAFUNCTIONS] = { \
 FUNCTION_TYPE_ENCODERA_1,
 FUNCTION_TYPE_ENCODERA_2 ,
};
/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct TENSIONFUNCTION* p); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * @param	: p = pointer to a bunch of things for this function instance
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct TENSIONFUNCTION* p)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, p->phub_encoder);// Send CAN msg to 'can_hub'
	p->hbct_ticks = (p->enc_a.hbct * tim3_ten2_rate) / 1000; // Convert ms to timer ticks
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	 // Reset heart-beat time duration each time msg sent
	return;
}

/* **************************************************************************************
 * int encoder_a_functionS_init_all(void);
 * @brief	: Initialize all 'encoder_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = "r" command can id for this function was not found
 *		: -997 = Add CANHUB failed
 *		: -996 = Adding CAN IDs to hw filter failed
 *		: -995 = "i" command can id not found for this function
 *
 * static int encoder_a_functionS_init(int n, struct TENSIONFUNCTION* p );
 * @brief	: Initialize all 'encoder_a' functions
 * @param	: n = instance index
 * @param	: p = pointer to things needed for this function
 * @return	: Same as above
 * ************************************************************************************** */
//  Declaration
static int encoder_a_functionS_init(int n, struct TENSIONFUNCTION* p );

/* *************************************************** */
/* Init all instances of encoder_a_function supported. */
/* *************************************************** */
int encoder_a_functionS_init_all(void)
{
	int i;
	int ret;

	/* Do the initialization mess for each instance possible with this program/board */
	for (i = 0; i < NUMTENSIONFUNCTIONS; i++)
	{
		ret = encoder_a_functionS_init(i, &enc_f[i]);
		if (ret < 0) return ret;
	}

	/* Setup incoming CAN msg hardware filters for each function instance */
	

	return ret;
}
/* *********************************************************** */
/* Do the initialization mess for a single encoder_a function. */
/* *********************************************************** */
static int encoder_a_functionS_init(int n, struct ENCODERAFUNCTION* p )
{
	int ret;
	int ret2;
	u32 i;

	/* Set pointer to table in highflash.  Base address provided by .ld file */
// TODO routine to find latest updated table in this flash section
	p->pparamflash = (uint32_t*)pparamflash[n];

	/* Copy table entries to struct in sram from high flash. */
	ret = encoder_idx_v_struct_copy(&p->enc_a, p->pparamflash);

	/* Setup mask for checking if this function responds to a poll msg. */
	// Combine so the polling doesn't have to do two tests
	p->poll_mask = (p->enc_a.p_pollbit << 8) || (p->enc_a.f_pollbit & 0xff);

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (recompute for online update)
	p->hbct_ticks = (p->enc_a.hbct * tim3_ten2_rate) / 1000;
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	

	/* Add this function (encoder_a) to the "hub-server" msg distribution. */
	p->phub_encoder = can_hub_add_func();	// Set up port/connection to can_hub
	if (p->phub_encoder == NULL) return -997;	// Failed

	/* Add CAN IDs to incoming msgs passed by the CAN hardware filter. */ 
	ret2 = can_driver_filter_add_param_tbl(&p->enc_a.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
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
			return ret;
		}
	}

	return -998;	// Argh! Table size reasonable, but didn't find it.
}
/* **************************************************************************************
 * static float encoder_a_scalereading(void);
 * @brief	: Apply offset, scale and corrections to encoder_a, last reading
 * return	: float with value
 * ************************************************************************************** */
#ifdef USECICCALIBRATIONSEQUENCE
static float encoder_a_scalereading(struct TENSIONFUNCTION* p)
{
	int ntmp1;
	long long lltmp;
	float scaled1;

	lltmp = cic[0][0].llout_save;
	ntmp1 = lltmp/(1<<22);
	ntmp1 += p->enc_a.ad.offset * 4;
	scaled1 = ntmp1;
	scaled1 *= p->enc_a.ad.scale;
	return scaled1;
}
#endif

/* ######################################################################################
 * int encoder_a_functionS_poll(struct CANRCVBUF* pcan, struct ENCODERAFUNCTION* p);
 * @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of encoder_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
//extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks
/* *** NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
int encoder_a_functionS_poll(struct CANRCVBUF* pcan, struct ENCODERAFUNCTION* p)
{
	int ret = 0;

	union FTINT	// Union for sending float as 4 byte uint32_t
	{
		uint32_t ui;
		float ft;
	}ui; ui.ui = 0; // (initialize to get rid of ui.ft warning)

	/* Check if this instance is used. */
	if ((p->enc_a.useme & 1) != 1) return 0;

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ten2_ticks - (int)p->hb_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.
		iir_filtered_calib(p, 1);	// Slow (long time constant) filter the reading
		ui.ft = p->fcalib_lgr;		// Float version of calibrated last good reading
		
		/* Send heartbeat and compute next hearbeat time count. */
		//      Args:  CAN id, status of reading, reading pointer instance pointer
		send_can_msg(p->enc_a.cid_heartbeat, p->status_byte, &ui.ui, p); 
		ret = 1;
	}

	/* Check if any incoming msgs. */
	if (pcan == NULL) return ret;

	/* Check for group poll, and send msg if it is for us. */
//$	if (pcan->id == p->enc_a.cid_ten_poll) // Correct ID?
//if (pcan->id == p->enc_a.cid_gps_sync) // ##### TEST #########
if (pcan->id == 0x00400000) // ##### TEST #########
	{ // Here, group poll msg.  Check if poll and function bits are for us
//$		if ( ((pcan->cd.uc[0] & p->enc_a.p_pollbit) != 0) && \
		     ((pcan->cd.uc[1] & p->enc_a.f_pollbit) != 0) )
		{ // Here, yes.  Send our precious msg.
			/* Send encoder msg and re-compute next hearbeat time count. */
			//      Args:  CAN id, status of reading, reading pointer instance pointer
			send_can_msg(p->enc_a.cid_ten_msg, p->status_byte, &ui.ui, p); 
			return 1;
		}
	}
	
	/* Check for encodera function command. */
	if (pcan->id == *p->pcanid_cmd_func_i)
	{ // Here, we have a command msg for this function instance. 
		cmd_code_dispatch(pcan, p); // Handle and send as necessary
		return 0;	// No msgs passed to other ports
	}
	return ret;
}

