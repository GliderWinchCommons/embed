/******************************************************************************
* File Name          : tension_a_function.c
* Date First Issued  : 03/26/2015
* Board              : f103
* Description        : Tension function
*******************************************************************************/

#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "ad7799_filter_ten.h"
#include "tension_idx_v_struct.h"
#include "db/gen_db.h"
#include "tension_idx_v_struct.h"
#include "tim3_ten.h"

/* Flash areas reserved outside of APP program flash defined in .ld file. */
extern void* __highflashlayout;	// function CMD CAN IDs table
extern void* __paramflash1;	// parameter table: Tension
extern void* __paramflash2;	// parameter table: TensionB
extern void* __paramflash3;	// parameter table: Cable_Angle

/* ========================================================================
   The following will include the structs with default parameter values and
     default command CAN IDs, as selected from the full list generated by
     the the java program from the database.  The items selected are specified
     in the "select_function.h" which is specific to this application. */
#include "select_function.h"
#include "db/idx_v_val.c"
/* ======================================================================== */

// Tension parameters that go in SRAM
extern struct TENSIONLC ten_a;	// Tension parameters

static uint32_t	canid_cmd = CANID_CMD_TENSION_1a;// Function command can id (default) 

static struct CANHUB* phub_tension;

static u32 hb_t;	// Next DTWTIIME for heart-beat
static struct CANRCVBUF can_hb = {CANID_HB_TENSION_1,0,{0}};

/* Easy way for other routines to access via 'extern'*/
extern struct CAN_CTLBLOCK* pctl1;

/* Last good 24 bit reading, bipolar, zero offset adjusted */
extern volatile int		ad7799_ten_last_good_reading;	

/* **************************************************************************************
 * static void send_can_cmd(struct CANRCVBUF* pcan, uint32_t v); 
 * @brief	: Setup DLC and payload for sending a current reading
 * @param	: Pointer to incoming CAN msg that has "readings" command
 * @param	: v = 4 byte value to be sent
 * ************************************************************************************** */
static void send_can(struct CANRCVBUF* pcan, uint32_t v)
{
	pcan->dlc = 6;			// Set return msg payload count
	pcan->cd.uc[3] = (v >>  0);	// Add 4 byte value to payload
	pcan->cd.uc[4] = (v >>  8);
	pcan->cd.uc[5] = (v >> 16);
	pcan->cd.uc[6] = (v >> 24);
	can_hub_send(pcan, phub_tension);// Send CAN msg to 'can_hub'
	hb_t = tim3_ticks + ten_a.hbct;	// Reset heart-beat time each time msg sent
	return;
}
/* **************************************************************************************
 * int tension_a_function_init_struct(uint32_t const* ptbl);
 * @brief	: Populate the parameter struct with the four byte parameter values
 * @param	: p = pointer to table (in flash) of parameter values
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
static int tension_a_function_init_struct(uint32_t const* ptbl)
{
	uint32_t** pptr = (uint32_t**)&idx_v_ptr[0];	// Point to table of struct element pointers
	uint32_t ctr = 0;	// counter for checking size of struct vs table of values

	hb_t = tim3_ticks + ten_a.hbct;	// First heartbeat time

	while (*pptr != NULL)	// Copy four byte values into struct via pointer table
	{ 
		**pptr++ = *ptbl++;
		ctr += 1;
	}
	/* struct element count versus size of table of values */
	if (ctr == 0) return ctr;
	if (ctr != (paramval[0] + 1)) return -ctr;	// Return for mismatch
	return ctr;

}
/* **************************************************************************************
 * int tension_a_function_init(void);
 * @brief	: Initialize
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
extern void* __highflashlayout;
int tension_a_function_init(void)
{
	int ret;
	/* The following is the DEFAULT values */
	ret = tension_a_function_init_struct(&paramval[0]);

	/* Add this function (tension_a) to the "hub-server" msg distribution. */
	phub_tension    = can_hub_add_func();	// Set up port/connection to can_hub
	
	/* Replace default command canid with one from table for this function instance. */
	// TODO
	
	return ret;
}
/* **************************************************************************************
 * void tension_a_function_poll(struct CANRCVBUF* pcan);
 * @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ************************************************************************************** */
extern unsigned int tim3_ticks;	// Running count of timer ticks
void tension_a_function_poll(struct CANRCVBUF* pcan)
{
	long long lltmp;
	int ntmp;

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ticks - (int)hb_t) > 0  )	// Time to send heart-beat
	{ 
		lltmp = llAD7799_ten_out2;
		ntmp = lltmp/(1<<19);
//$$		send_can(&can_hb, ntmp); // TODO add status byte
	}

	if (pcan == NULL) return;

	/* Check for group poll. */
	if (pcan->id == ten_a.cid_ten_poll)
	{ // Here, group poll msg.  Check if our drum system is included
		if ((pcan->cd.uc[0] & POLL_FUNC_BIT_TENSION) != 0)
		{ // Here, yes.  See if our function is included
			if ((pcan->cd.uc[0] & POLL_FUNC_BIT_TENSION) != 0)
			{ // Here, yes.  Send our precious msg.
				pcan->cd.uc[0] = 0;	// Status byte TODO
//$$				send_can(pcan, ad7799_ten_last_good_reading);
				return;
			}
		}
	}
	
	/* Check for tension function command. */
	if (pcan->id == canid_cmd)
	{ // Here, we have a command msg for this function instance. 
		// TODO handle command code
	}
	return;
}
/* **************************************************************************************
 * int tension_param_default_init(int x);
 * @brief	: Fill sram struct from table with values
 * @param	; x = 0 for use fixed default; not zero for flash table
 * @return	: Nothing for the nonce
 * ************************************************************************************** */
int tension_param_default_init(int x)
{
	uint32_t* pstruct = (uint32_t*)&idx_v_ptr[0];
	uint32_t* ptable = (uint32_t*)&paramval[0];

	if (x == 0)
	{
		while (*pstruct	!= NULL)
		{
			*pstruct++ = *ptable++;
		}
	}

	return 0;
}

