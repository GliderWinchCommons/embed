/******************************************************************************
* File Name          : CAN_poll_loop.c
* Date First Issued  : 04/06/2015
* Board              : f103
* Description        : Poll the functions for the functions in the tension app.
*******************************************************************************/
/* 
NOTE: *ALL* sending of CAN msgs is done under the interrupt in this routine.  Don't
try to send a CAN msg from a different interrupt level.  If that is to be done load a buffer
w flag and have this routine pass that on to the CAN send routine.
*/

#include "can_hub.h"
#include "shaft_function.h"
#include "libusartstm32/nvicdirect.h"
#include "can_driver.h"
#include "db/gen_db.h"
#include "tim4_shaft.h"
#include "adcsensor_foto_h.h"
#include "IRQ_priority_shaft.h"

extern void (*can_msg_reset_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk);

void CAN_poll_loop_trigger(void);
void CAN_poll(void);

/* hub port buffer pointers. */
// These were made non-static since if they are not included the compiler
//   will issue a warning.
struct CANHUB* phub_app = NULL;	

/* **************************************************************************************
 * int CAN_poll_loop_init(void);
 * @brief	: Initialize
 * @return	: 0 = OK, negative = fail
 * ************************************************************************************** */
int CAN_poll_loop_init(void)
{
	/* Runs polling loop */
	tim4_shaft_tim_oc_ptr = &CAN_poll_loop_trigger;	// 'rpmsensor.c' CH2 oc timer triggers poll

	NVICIPR (NVIC_I2C1_ER_IRQ, NVIC_I2C1_ER_IRQ_PRIORITY_FOTO);	// Set interrupt priority

	/* Get a buffer for each "port" */
	phub_app = can_hub_add_func();	// Get a hub port for dealing with commands
	if (phub_app == NULL) return -1;

	/* Each CAN msg not rejected by the hardware filters will trigger
          a pass through CAN_poll.  The trigger comes via a routines that exits to the
          next routine in the chain.  It starts with 
	  'can_driver' RX0,1 interrupt -> 'can_msg_reset' -> 'can_msg_reset_ptr' */
	can_msg_reset_ptr = (void*)&CAN_poll_loop_trigger; // Cast since no arguments are used

	return 0;
}
/* **************************************************************************************
 * void CAN_poll_loop_enable_interrupts(void);
 * @brief	: Enable interrupts of I2C1_ER
 * ************************************************************************************** */
void CAN_poll_loop_enable_interrupts(void)
{
	NVICISER(NVIC_I2C1_ER_IRQ);			// Enable interrupt controller	
	return;
}
/* **************************************************************************************
 * void CAN_poll_loop_trigger(void);
 * @brief	: Trigger low level interrupt to run 'CAN_poll'
 * ************************************************************************************** */
/* The following routine is called by 'main', 'tim3_ten', and eventually 'can_driver' (via
   can_msg_reset') */
void CAN_poll_loop_trigger(void)
{
	/* Trigger a pending interrupt, which will call 'CAN_poll' */
	NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (low priority) interrupt 
	return;
}
/*#######################################################################################
 * ISR routine runs CAN poll loop
 *####################################################################################### */
void I2C1_ER_IRQHandler(void)
{
	struct CANRCVBUF* pcan;
	int sw;	
	int ret;
	do
	{
		sw = 0;
		can_hub_begin(); // Load CAN hardware msgs into tension queue, if any
		/* Get msgs for this APP. */
  		pcan = can_hub_get(phub_app); 	// Get ptr to CAN msg
		if (pcan != NULL)	// Check for no msgs available
		{
			sw = 1;
//$	 		app_function_poll(pcan); 	// function poll
		}
		/* shaft: get msgs from shaft buffer */
  		pcan = can_hub_get(shaft_f.cf.phub);      // Get ptr to CAN msg
	 	ret = shaft_common_poll(pcan,&shaft_f);   // function poll
		if (ret != 0)	// Did function send & buffer a msg?
		{ // Here yes.  Other functions may need this msg
			sw = 1; // Set switch to cause loop again
		}

	} while ((can_hub_end() != 0) || (sw != 0)); // Repeat if msgs waiting or were added
	return;
}

