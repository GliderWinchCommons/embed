/******************************************************************************
* File Name          : ledf4.h
* Date First Issued  : 07/12/2017
* Board              : Discovery F4
* Description        : LED 
*******************************************************************************/

#ifndef __LEDF4
#define __LEDF4

#include "libopencm3/stm32/f4/gpio.h"

/* LED identification
Discovery F4 LEDs: PD 12, 13, 14, 15

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
12 green   
13 orange
14 red
15 blue		
*/
#define GRN 12
#define ORG 13
#define RED 14
#define BLU 15

#define LED_OFF(pin)	GPIO_BSRR(GPIOD)=(1<<pin);	// Set bit
#define LED_ON(pin)	GPIO_BRR (GPIOD)=(1<<pin);	// Reset bit
#define LED_TOGGLE(pin)	if ((GPIO_ODR(GPIOD) & (1<<pin)) == 0) GPIO_BSRR(GPIOD)=(1<<pin);else GPIO_BSRR(GPIOD)=(1<<(pin+16));

/* **************************************************************************************/
void toggle_4leds (void);
/* @brief	: Make leds go around in a circle
 * ************************************************************************************** */


#endif 

