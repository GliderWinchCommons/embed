/******************************************************************************
* File Name          : p1_initialization.c
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Initialization for winch tension with POD board
*******************************************************************************/
/*
08-30-2011

The initialization takes place in two steps.  First there is a basic
initialization required in all cases.  This main routine then calls
'p1_get_reset_mode' which does some RTC initialization which in turn
determines the type of reset.  For a "normal" reset (where the unit
comes out of STANDBY) additional initialization is required to get
the SD Card and AD7799 working.  For "deepsleep" mode cleaning up the
adc initialization is all that is needed.

*/

/* The following are in 'svn_pod/sw_stm32/trunk/lib' */
#include <stdio.h>
#include "libopenstm32/systick.h"
#include "libmiscstm32/systick1.h"

#include "libmiscstm32/clockspecifysetup.h"

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/bkp.h"	// Used for #defines of backup register addresses
#include "libopenstm32/systick.h"
#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers
#include "libopenstm32/timer.h"	// Used for #defines of backup register addresses
//#include "PODpinconfig.h"
#include "tim3_ten2.h"
#include "adcsensor_pwrbox.h"
#include "DTW_counter.h"
#include "panic_leds_Ard.h"
#include "pwrbox_function.h"

/* TIM3 interrupt/tick rate */
unsigned int tim3_ten2_rate ; // ticks per second


/******************************************************************************
 * static void timedelay_usec (unsigned int usec);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 *******************************************************************************/
static void timedelay_usec (unsigned int usec)
{
	unsigned int ticks = sysclk_freq/1000000; // Sysclk ticks per usec
	u32 t1 = (ticks * usec) + DTWTIME;	// Time to quit looping	
	WAITDTW(t1);				// Loop
}
/******************************************************************************
 * void p1_initialization(void);
 * @brief 	: Initialize everything needed for all operationg modes
*******************************************************************************/
void p1_initialization(void)
{

	/* Get SAR ADC initialized and calibrated */
	adcsensor_pwrbox_sequence();	// Initialization sequence for the adc

	/* Timer interrupt callback -> ad7799_poll (in 'ad7799_filter.c'). */
//	tim3_ten2_ptr = &ad7799_poll_rdy_ten2_cic;

	/* Poll 2048/sec using this timer. ! */
	tim3_ten2_rate = 4096;//2048;

	return;
}


