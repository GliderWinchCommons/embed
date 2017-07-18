/******************************************************************************
* File Name          : ledf4.c
* Date First Issued  : 07/12/2017
* Board              : Discovery F4
* Description        : Simple LED handling
*******************************************************************************/

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

#include "ledf4.h"

/* **************************************************************************************
 * void toggle_4leds (void);
 * @brief	: Make leds go around in a circle
 * ************************************************************************************** */
static int lednum = 12;	// Lowest port bit numbered LED
void toggle_4leds (void)
{
	LED_TOGGLE(lednum);

	lednum += 1;		// Step through all four LEDs
	if (lednum > 15) lednum = 12;

}

