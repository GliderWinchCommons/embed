/******************************************************************************
* File Name          : panic_leds_Ard.c
* Date First Issued  : 05/24/2013
* Board              : stm32F103C8T6 Arduino board
* Description        : Panic flash LED's
*******************************************************************************/
/*
Flash LED's fast for the hard fault count, then pause, and repeat

NOTE--THIS IS BOARD-SPECIFIC.
NOTE--THIS IS BOARD-SPECIFIC.
NOTE--THIS IS BOARD-SPECIFIC.

*/ 
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/gpio.h"
#include "libopenstm32/scb.h"
#include "pinconfig_all.h"

#define GRNLED 13		// PC13 is one and only green led on Blue Pill
#define GRNLEDPORT GPIOC

extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

static void allon(void)
{
	GPIO_BRR(GPIOC) = (1 << 13);
	return;
}
static void alloff(void)
{
	GPIO_BSRR(GPIOC) = (1 << 13);
	return;
}	
static void loop(volatile int ct)
{
	while (ct > 0) ct -= 1; 
	return;
}

/******************************************************************************
void panic_leds_Ard(unsigned int count);
 * @param	: Number of fast flashes
 * @brief	: Configure gpio pins
 ******************************************************************************/
const struct PINCONFIGALL pin_led = {(volatile u32 *)GRNLEDPORT, GRNLED, OUT_OD, MHZ_2};

void panic_leds_Ard(unsigned int count)
{
	unsigned int i;
	volatile int x;		// Loop ct for timing pause
	volatile int xon;	// Loop ct for timing LED on duration
	volatile int xoff;	// Loop ct for timing LED off duration

	/* Have clocks been setup? */
	if (sysclk_freq < 1000000)
	{ // Here, no.  We are running on the 8 MHz HSI oscillator
		sysclk_freq = 8000000;	// Set default
	}

	/* Setup timing for pause */
	x = sysclk_freq/10;

	/* Setup timing for fast flash */
	xon =  sysclk_freq/250;
	xoff = sysclk_freq/40;

	/* Limit max count */
	if (count == 0) count = 6; // Bogus count
	if (count > 6)  count = 7; // Bogus count

	/* Be sure we have the ports and pins setup for the LEDs */
	RCC_APB2ENR |= (0x4 << 2) | (0x1); // Enable Ports A,B,C and AFIO
	pinconfig_all( (struct PINCONFIGALL *)&pin_led);	
	
	/* Now flash, flash, flash away, ye' rummies.  Batten the hatches, 'er there be trouble. */
	while (1==1)
	{
		for (i = 0; i < count; i++)
		{
			allon();	loop(xon);
			alloff();	loop(xoff);			
		}
		loop(x);
	}	
}
