/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xbrange_w
* Person             : deh
* Date First Issued  : 04/08/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : WINCH END receives reflected packet from POD end.  Uses API mode
*******************************************************************************/

/* 

Open minicom on the PC with 115200 baud and 8N1.

*/
/*
XBee pins of interest--
PB XB	XB wire	Signal
5   1	red	3.3v reg enable--XBee : gpio out	(TIM3 CH2*)(SPI3_MOSI)(SPI1_MOSI*)(I2C1_SMBAI)
8   9	white	XBee /DTR/SLEEP_RQ: gpio_out 		(CANRX*)(I2C2_SMBAI)
9		XBee /RESET: gpio_out 			(CANTX*)

PD
3  12	white	Xbee /CTS:	USART2_CTS*
4  16	yellow	Xbee /RTS:	USART2_RTS*
5   3	yellow	Xbee DIN/CONFIG:USART2_TX*
6   2	white	Xbee DOUT: 	USART2_RX*


*/


#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"

#include "libusartstm32/usartallproto.h"

#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/systick_delay.h"
#include "libmiscstm32/printf.h"

#include "libxbeestm32/xb_api_rx.h"
#include "libxbeestm32/xb_api_tx.h"
#include "libxbeestm32/xb_at_cmd.h"
#include "libxbeestm32/xb_api_atcmd.h"

#include "PODpinconfig.h"
#include "pinconfig.h"


/* Parameters for setting up the clocking */
/* PLLMUL_6X = 6 * 8 Mhz = 48 MHz clock */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_9X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_2,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};


int	nCode;		// Return code for USART2 remapping
char pp[192];		// USART1 -> USART2 buffer

unsigned int dist[16];
int nV;
extern char api_debug;


#define MAXXBEESIZE	124
char xbframe[MAXXBEESIZE];	// Buffer for received api frame
char xbrcv[MAXXBEESIZE];	// Buffer for received api frame


/* This if for test sprintf */
char vv[180];	// sprintf buffer

/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);
	}

/* LED identification

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
3 green   
4 red
5 green
6 yellow
*/

/* ************************************************************
Turn the LEDs on in sequence, then turn them back off 
***************************************************************/
static int lednum = LED3;	// Lowest port bit numbered LED
void toggle_4leds (void)
{
	if ((GPIO_ODR(GPIOE) & (1<<lednum)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOE) = (1<<lednum);	// Set bits = all four LEDs off
	}
	else
	{ // HEre, LED bit was on
		GPIO_BRR(GPIOE) = (1<<lednum);	// Reset bits = all four LEDs on
	}
	lednum += 1;		// Step through all four LEDs
	if (lednum > LED6) lednum = LED3;
}
/* ************************************************************
 * void xb_api_rx_at(char *p, char *q);
 * @brief	: Setup string for output from 0x88 (AT command response)
 * @param	: p = pointer to output buffer 
 * @parm	: q = pointer to received frame buffer
***************************************************************/
void xb_api_rx_at(char *p, char *q)
{
	/* Two char AT command */
	*p++ = *(q+2);
	*p++ = *(q+3);
	switch (*(q+4))
	{
	case 0:
		strcpy(p," OK") ;  break;
	case 1:
		strcpy(p," ER") ;  break;
	default:
		strcpy(p," oo") ;  break;
	}
	p += 3;	
	sprintf (p," %02x \n\r",*p);
	return;
}

/***************************************************************************************************
And now for the main routine 
****************************************************************************************************/
int main(void)
{
	int j;			// Another (fixed pt FORTRAN!) variable
	char* p;		// Temp getline pointer
	unsigned int uiT;	// Time delay count
	int	nRcv;		// Receive packet builder return code

/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS *)&clocks);	// Get the system clock and bus clocks running

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen on USART1)

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

/* --------------------- Initialize usart --------------------------------------------------------- */
/*	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line)
		number of tx line buffers, (must be > 1)
*/
	USART1_rxinttxint_init(115200,150,2,150,2); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rxbrange_w: XBee range test WINCH-END 04-09-2012 \n\r");
	USART1_txint_send();	// Start the line buffer sending


	
	/* Xbee set up */
	/* USART2* connects the the XBee.  'rxdma' uses circular buffering, needed for receiving api frames. */
	if ( (nCode=USART2_rxdma_init(  9600,256,32) ) != 0) 	// Initialize USART and setup control blocks and pointers
	{
		printf("\n\rUSART2 rxdma initialize failed, %d\n\r",nCode); USART1_txint_send();
	}
	if ( (nCode=USART2_txdma_init(  9600,124,2) ) != 0) 	// Initialize USART and setup control blocks and pointers
	{
		printf("\n\rUSART2 txdma initialize failed, %d\n\r",nCode); USART1_txint_send();
	}


	if ( (nCode = USARTremap (USART2, 1)) != 0)	// Remap pins
	{
		printf("\n\rUSART2 remap failed code = %d\n\r",nCode);	USART1_txint_send();
	}


	/* Inidividual pins */
	XBEE_RESET_hi;		// Be sure /RESET is de-asserted (to use Digi's terminology)
	XBEESLEEPRQ_hi;		// Be sure /SLEEP is de-asserted
	XBEEREG_on;		// Enable regulator to turn on power to XB module
	configure_pin_input_pull_up_dn ( (volatile u32 *)GPIOD, 3, 1); // Configures PD3, /CTS from XBee, for input w pull up
	configure_pin ( (volatile u32 *)GPIOD, 4); // Configures PD4, /RTS to XBee, for pushpull output
	delay_systick(600);	// Allow time for power to come up and XBee to initialize


	/* Configure */
	USART1_txint_puts("--Configurating WINCH END--\n\r");	USART1_txint_send();
	/* DL-dest addr, MY-source addr, BD- baud rate (7 = 115200), CN - exit command mode */
	xb_send_AT_cmd("ATRE,DL 2,MY 1,AP 2,DL,MY,AP,CH D,CA 0,BD 7,CN\r");	// Library routine to do 'send_xb_cmd'
	uiT =  delay_systick_t0(200);			// Get systick time for end of duration
	while ( ((int)(SYSTICK_getcount32() - uiT)) > 0 )// Wait for responses to complete
	{
		/* Send XBee chars to PC */
		if ((p=USART2_rxdma_getline()) != 0)	// Check if we have a completed line XBee USART
		{ // Here, pointer points to line buffer with a zero terminated line
			USART1_txint_puts("USART2: ");	// Identify source of printout
			USART1_txint_puts(p);		// Echo back the line just received
			USART1_txint_puts ("\n");	// Add line feed to make things look nice
			USART1_txint_send();		// Start the line buffer sending			
		}
	}
	unsigned int uiBaud = 115200;
	printf("Change USART baud %i\n\r", uiBaud);	USART1_txint_send();
	USART2_setbaud(uiBaud);

/* -------------------- Send/receive loop -------------------------------------------------------- */
	USART1_txint_puts("\n\r");	USART1_txint_send();
	j = 0;

union XB
{
long l[2];
char c[8];
}xb;
xb.l[0] = 0x00000000;

char sw;

#define	DELAYMS	50	// Milliseconds for loop

	while (1==1)
	{
		toggle_4leds();			// Walk the LED's for the hapless Op

		/* Xmit frame */
		xb.l[1] = ~xb.l[0];		// Send 2nd long bit inverted

delay_systick(4);

		xb_tx16(0xffff,&xb.c[0], 8 );	// Send 16 bit address: dest addr, data pointer, size (@1)


		sw = 0;
		/* Look for a reply for the remainder of the interval */
		while ( ((int)(SYSTICK_getcount32() - uiT)) > 0 )	// Wait for interval to complete
		{
			if ( (nRcv = xb_getframe(xbrcv, MAXXBEESIZE)) > 0)
			{ // Here, we got a packet from the winch end	
				printf ("%u %u",xb.l[0], *(unsigned int*)(&xbrcv[6]) );// Our ct, rec'd count
				printf ("%4u%4u",xbrcv[3],xbrcv[5]);	// Our RSSI, winch RSSI
				USART1_txint_puts ("\r\n");	// Add line feed to make things look nice
				USART1_txint_send();		// Start the line buffer sending
				sw = 1;	// Show we got a good packet		
			}
		}
		if (sw == 0)
		{
			printf ("%4u *\n\r",j);  USART1_txint_send();
		}

		xb.l[0] += 1; 	// Advance data counter
		j += 1;

		/* Set time for the next interval */
		uiT -= (DELAYMS*(sysclk_freq/1000));	// Advance delay ticks
	}




	return 0;	
}


