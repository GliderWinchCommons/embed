/******************************************************************************
* File Name          : shaft.c
* Date First Issued  : 04/22/2018
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : sensor: shaft encoder w foto detector histogram collection
*******************************************************************************/
/* 
02-05-2014
Hack of se4.c routine

Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>


#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "scb.h"

#include "../../../../svn_common/trunk/can_driver.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "can_driver_filter.h"
#include "can_msg_reset.h"
#include "CAN_poll_loop.h"


#include "../../../sw_f103/trunk/lib/libsensormisc/canwinch_setup_F103_pod.h"
#include "SENSORpinconfig.h"
#include "sensor_threshold.h"
#include "panic_leds.h"
#include "adcsensor_foto_h.h"
#include "common_highflash.h"
#include "../../../../svn_common/trunk/can_gps_phasing.h"
#include "../../../../svn_common/trunk/can_msg_reset.h"
#include "libmiscstm32/DTW_counter.h"

#include "canwinch_pod_common_systick2048_printerr.h"
#include "../../../../svn_common/trunk/common_highflash.h"

#include "fmtprint.h"
#include "can_filter_print.h"
#include "CAN_filter_list_printf.h"

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;
struct CAN_CTLBLOCK* pctl0;	// Some routines use 0 for CAN1 others use 1 (sigh)


/* Make block that can be accessed via the "jump" vector (e.g. 0x08004004) that points
   to 'unique_can_block', which then starts Reset_Handler in the normal way.  The loader
   program knows the vector address so it can then get the address of 'unique_can_block'
   and get the crc, size, and data area via a fixed offset from the 'unique_can_block'
   address. 

   The .ld file assures that the 'unique_can_block' function is is followed by the data
   by using the sections.  */
#include "startup_deh.h"
__attribute__ ((section(".ctbltext")))
void unique_can_block(void)
{
	Reset_Handler();
	while(1==1);
}

/* The order of the following is important. */
extern const struct FLASHP_SE4 __highflashp;
__attribute__ ((section(".ctbldata")))
const unsigned int flashp_size = sizeof (struct FLASHP_SE4);
__attribute__ ((section(".ctbldata1")))
const struct FLASHP_SE4* flashp_se3 = (struct FLASHP_SE4*)&__highflashp;

/* Parameters for setting up CAN */

// Default: based on 72 MHz clock|36 MHz AHB freqs--500,000 bps, normal, port B
//const struct CAN_PARAMS can_params = CAN_PARAMS_DEFAULT;	// See 'canwinch_pod.h'

/* Specify msg buffer and max useage for TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
180,	/* Total number of msg blocks. */\
140,	/* TX can use this huge ammount. */\
40,	/* RX0 can use this many. */\
8	/* RX1 can use this piddling amount. */\
};

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;
/*******************************************************************************
 * void can_nxp_setRS_sys(int rs, int board);
 * @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs) (SYSTICK version)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @param	: board: 0 = POD, 1 = sensor RxT6 board
 * @return	: Nothing for now.
*******************************************************************************/
void can_nxp_setRS_sys(int rs, int board)
{
	/* RS (S) control PB7 (on sensor board) PD11 on pod board */
	// Floating input = resistor controls slope
	// Pin HI = standby;
	// Pin LO = high speed;
	if (board == 0)
	{
		configure_pin ((volatile u32 *)GPIOD, 11);	// configured for push-pull output
		if (rs == 0)
			GPIO_BRR(GPIOD)  = (1<<11);	// Set bit LO for SILENT mode
		else
			GPIO_BSRR(GPIOD) = (1<<11);	// Set bit HI for NORMAL mode
	}
	else
	{
		configure_pin ((volatile u32 *)GPIOB,  7);	// configured for push-pull output	
		if (rs == 0)
			GPIO_BRR(GPIOB)  = (1<< 7);	// Set bit LO for SILENT mode
		else
			GPIO_BSRR(GPIOB) = (1<< 7);	// Set bit HI for NORMAL mode
	}
	return;
}

/* **************************************************************************************
 * void system_reset(void);
 * @brief	: Software caused RESET
 * ************************************************************************************** */
void system_reset(void)
{
/* PM 0056 p 134 (April 2010 Doc ID 15491 Rev 3 1/154)
4.4.4 Application interrupt and reset control register (SCB_AIRCR)
      Address offset: 0x0C
      Reset value: 0xFA05 0000
      Required privilege: Privileged
      The AIRCR provides priority grouping control for the exception model, endian status for data
      accesses, and reset control of the system.
      To write to this register, you must write 0x5FA to the VECTKEY field, otherwise the
      processor ignores the write.
*/

/* Bit 2 SYSRESETREQ System reset request
      This is intended to force a large system reset of all major components except for debug.
      This bit reads as 0.
      0: No system reset request
      1: Asserts a signal to the outer system that requests a reset.
*/
	SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	while (1==1);
}

/* **************************************************************************************
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
//void putc ( void* p, char c)
//	{
//		p=p;	// Get rid of the unused variable compiler warning
//		CANascii_putc(c);
//		USART1_txint_putc(c);
//	}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int j;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();

/* --------------------- Begin setting things up -------------------------------------------------- */ 
	// Start system clocks using parameters matching CAN setup parameters for F103 boards
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins
/* ---------------------- Set up 32b DTW system counter ------------------------------------------- */
	DTW_counter_init();
setbuf(stdout, NULL);

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
	j = USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers
//	USART1_rxinttxint_init(  9600,32,2,32,3); // Initialize USART and setup control blocks and pointers

	if (j != 0) panic_leds(7);	// Init failed: Bomb-out flashing LEDs 7 times

	/* Announce who we are */
	USART1_txint_puts("\n\r#### SHAFT #### 04/22/2018 (../svn_sensor/sensor/shaft/trunk) \n\r");
	USART1_txint_send();	// Start the line buffer sending

	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	

	/* ---------------- Some test, monitoring, debug info ----------------------------------------------- */
	extern void* __paramflash0a;
	struct FLASHH2* pcanidtbl = (struct FLASHH2*)&__paramflash0a;
	printf ("FLASHH2 Address: %08X\n\r",(unsigned int)pcanidtbl);
	printf ("  CAN unit code: %08X\n\r",(int)pcanidtbl->unit_code);
	printf ("  Size of table: %d\n\r",(int)pcanidtbl->size);
	printf(" # func  CANID\n\r");
	u32 ii;
	u32 jj = pcanidtbl->size;
	if (jj > 16) jj = 16;
	for (ii = 0; ii < jj; ii++)
	{
		printf("%2d %4d  %08X\n\r",(int)ii, (int)pcanidtbl->slot[ii].func, (int)pcanidtbl->slot[ii].canid);
	}
	printf("\n\r");
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	struct FUNC_CANID* pfunc = (struct FUNC_CANID*)&__paramflash0a;
//	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	u32 canid_ldr = pfunc[0].func;
	printf("CANID_LDR  : 0x%08X\n\r", (unsigned int)canid_ldr );
	printf("TBL SIZE   : %d\n\r",(unsigned int)pfunc[0].canid);
	USART1_txint_send();

/* --------------------- CAN setup ---------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Initialize CAN for POD board (F103) and get control block */
	pctl1 = canwinch_setup_F103_pod(&msginit, canid_ldr); // ('can_ldr' is fifo1 reset CAN ID)
	pctl0 = pctl1;	// Save copy for those routines that use 0 instead of 1

	/* Check if initialization was successful. */
	if (pctl1 == NULL)
	{
		printf("CAN1 init failed: NULL ptr\n\r");USART1_txint_send(); 
		while (1==1);
	}
	if (pctl1->ret < 0)
	{ // Here, an error code was returned.
		printf("CAN init failed: return code = %d\n\r",pctl1->ret);USART1_txint_send(); 
		while (1==1);
	}
/* ------------------ misc --------------------------------------------------------------------------- */

/* -------------------- Functions init --------------------------------------------------------------- */

/* ------------------ Set up hardware CAN filter ----------------------------------------------------- */
	can_msg_reset_init(pctl0, pcanidtbl->unit_code);	// Specify CAN ID for this unit for msg caused RESET
	printf("## pcanidtbl->unit_code: 0x%08X\n\r",pcanidtbl->unit_code);

	/* Go through table and load "command can id" into CAN hardware filter. */
	//                     (CAN1, even, bank 2)
	can_driver_filter_setbanknum(0, 0, 2);

	int ret;
	u32 id;
	jj = pcanidtbl->size;
	if (jj > 16) jj = 16;	// Check for bogus size
	for (ii = 0; ii < jj; ii++) 
	{
		id = pcanidtbl->slot[ii].canid;
		//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
		ret = can_driver_filter_add_one_32b_id(0,id,0);
		if (ret < 0)
		{
			printf("FLASHH CAN id table load failed: %d\n\r",ret);USART1_txint_send(); 
			while (1==1);
		}
	}
	ret = CAN_filter_build_list_add(); // Add previously built filter tables to CAN hardware
	if (ret <= 0)
	{
		printf("\n\r### ERROR: CAN_filter_build_list_add %d\n\r\n\r",ret);USART1_txint_send(); 
	}
	printf("CAN_filter_build_list_add: %d\n\r",(ret >> 8)); // Number should match list

	CAN_filter_list_printf();	// List CAN hardware filter list for incoming msgs

	u32 filtnum = can_driver_hw_filter_list(1, -1, 0); // Get next available filter bank number
	printf("CAN 1 hardware filter list\n\r");
	printf("filtnum: %d  odd/even: %d\n\r",(filtnum >> 1),(filtnum & 0x1));
	u32 i;
	for (i = 0; i < (filtnum >> 1); i++) // List the hw filter registers
	{
		id = can_driver_hw_filter_list(0, i, 0);	printf("%2d 0x%08X ",i,id);
		id = can_driver_hw_filter_list(0, i, 1);	printf("0x%08X\n\r",id);
	}
	printf("\n\r");

	USART1_txint_send(); 
/* --------------------- ADC setup and initialization ------------------------------------------------ */

/* --------------------- Final CAN setup ------------------------------------------------------------- */
	// Set addresses to chain tests of incoming canid 
//	engine_can_msg_poll_init(); // Test for poll of winch function

	can_driver_enable_interrupts(); // CAN sending/receiving starts here.

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */
//printf("tim4_tim_rate: %d\n\r",tim4_tim_rate);
//printf("eman_f.lc.hbct: %d\teman_f.cf.hb_tdur: %d\n\r",eman_f.lc.hbct,eman_f.cf.hb_tdur);
i = 0;
extern s32 CAN_dev;
extern s32 CAN_ave;
extern s32 CAN_dif;

#ifdef THIS_MAY_BE_NEEDED_LATER
extern u32 adc2histo[2][ADCHISTOSIZE];
extern u32 adc3histo[2][ADCHISTOSIZE];
extern int adc2histoctr[2];
extern int adc3histoctr[2];
extern union ADC12VAL adc12valbuff[2][ADCVALBUFFSIZE];
extern unsigned short adc3valbuff[2][ADCVALBUFFSIZE];
#endif

//int k = 0;

struct CANRCVBUF* pcan;
u32 z = 0;
u32 q = 0;

/* Green LED flashing */
static u32 stk_64flgctr_prev;

extern void testfoto(void);
//testfoto();

int xct = 0;
unsigned int ctr = 0;
//canwinch_pod_common_systick2048_printerr_header();
/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Check for a 1/2048th sec tick ('canwinch_pod_common_systick2048.c') */
		if (stk_64flgctr != stk_64flgctr_prev)
		{
			stk_64flgctr_prev = stk_64flgctr;

			/* Flash green LED in sync with time--ON/OFF each 1 sec */
			if ((stk_64flgctr & 0x7ff) == 0) 
			{
				TOGGLE_GREEN;
//				canwinch_pod_common_systick2048_printerr(&pctl1->can_errors); // CAN error counts
			}	// Slow flash of green means "OK"

			/* Throttle CANascii to stay within bandwidth of CAN bus & PC<->gateway link. */			
			if ((stk_64flgctr & 0x7ff) == 0) 
			{
				printf("A %3d %7d %7d %7d %7d %7d %3u%3u%3u%3u", xct++,CAN_ave, CAN_dif, CAN_dev, encoder_ctrA2, speed_filteredA2,\
				adcsensor_foto_err[0],adcsensor_foto_err[1],adcsensor_foto_err[2],adcsensor_foto_err[3]); 

//				printf ("HSTct: %5d %5d %5d %5d : ",adc2histoctr[k],adc3histoctr[k],adc12valbuff[0][47].us[1],adc3valbuff[0][1]);
//				for (j = 40; j < 70; j++)
//				{
//					printf("%5d %5d | ",adc2histo[k][j]/512,adc3histo[k][j]/512);
//				}
				printf ("\n\r");USART1_txint_send(); 
			}
		}
		/* Check incoming CAN msgs for readout commands and send response. */
		pcan = can_driver_peek0(pctl1);	// Check for FIFO 0 msgs
		if (pcan != NULL) 	// Check for PC CAN msg to readout histogram
		{ // Here, we got something!
			adc_histo_cansend(pcan);// Check for commands to readout histogram and other functions
			z += 1;
			switch (pcan->id & ~0x1)
			{
		/* NOTE: The following CAN IDs are hard coded and not .txt file derived. */
			case 0xd1e00014: q = 0; break;
			case 0xd1e00034: q = 0; break;
			case 0xd1e00024:
			case 0xd1e00044:
				printf ("%u %u\n\r",z,q++);USART1_txint_send(); 
				break;
			case 0xFFFF0004: // Test CAN ID sent by cangate:s
				printf ("%u %d\n\r",pcan->cd.ui[0],(int)(pcan->cd.ui[0] - ctr) );USART1_txint_send(); 
				ctr += 1;
				break;
			}
			can_driver_toss0(pctl1);	// Release buffer block
		}
		pcan = can_driver_peek1(pctl1);	// Check for FIFO 0 msgs
		if (pcan != NULL) 	// Check for PC CAN msg to readout histogram
		{ // Here, we got something!
			can_driver_toss1(pctl1);	// Release buffer block
		}
	}
	return 0;	
}

