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
#include "engine_function.h"
#include "libusartstm32/nvicdirect.h"
#include "can_driver.h"
#include "tim3_ten2.h"
#include "db/gen_db.h"

extern void (*can_msg_reset_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk);

void CAN_poll_loop_trigger(void);
void CAN_poll(void);

/* hub port buffer pointers. */
// These were made non-static since if they are not included the compiler
//   will issue a warning.
struct CANHUB* phub_app = NULL;		// ?
/* The following are added separately. */
//struct CANHUB* phub_tension_a = NULL;	// AD7799 #1
//struct CANHUB* phub_tension_b = NULL;	// AD7799 #2
//struct CANHUB* phub_cable_angle = NULL;	// Cable angle function

/* **************************************************************************************
 * int CAN_poll_loop_init(void);
 * @brief	: Initialize
 * @return	: 0 = OK, negative = fail
 * ************************************************************************************** */
int CAN_poll_loop_init(void)
{
	/* Runs polling loop */
	tim3_ten2_ll_ptr = &CAN_poll;	// 'tim3_ten.c' timer triggers low level interrupt to come to this function
	NVICIPR (NVIC_I2C1_ER_IRQ, NVIC_I2C1_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Get a buffer for each "port" */
	phub_app = can_hub_add_func();	// Get a hub port for dealing with commands
	if (phub_app == NULL) return -1;

	return 0;
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
/* ###################### UNDER INTERRUPT ###############################################
 * void CAN_poll(void);
 * @brief	: Low priority interrupt polls for CAN msgs, uses can_hub
 * ###################################################################################### */
void CAN_poll(void)
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
		/* eng_manifold: get msgs from eng_manifold buffer */
  		pcan = can_hub_get(eman_f.cf.phub); 	// Get ptr to CAN msg
		{
	 		ret = eng_man_poll(pcan,&eman_f); 	// function poll
			if (ret != 0)	// Did function send & buffer a msg?
			{ // Here yes.  Other functions may need this msg
				sw = 1; // Set switch to cause loop again
			}
		}


	/* Loop until no msgs were handled. */
	} while ((can_hub_end() != 0) || (sw != 0)); // Repeat if msgs waiting or were added
	return;
}

