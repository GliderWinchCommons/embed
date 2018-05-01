/******************************************************************************
* File Name          : shaft.c
* Date First Issued  : 04/22/2018
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : sensor: shaft encoder w foto detector histogram collection
*******************************************************************************/
/* 
02-05-2014 Hack of se4.c
04/27/2018 Hack of se4_h Update to database parameter scheme

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
//#include "CAN_filter_list_printf.h"
#include "tim4_shaft.h"
#include "shaft_printf.h"
#include "shaft_function.h"

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;
struct CAN_CTLBLOCK* pctl0;	// Some routines use 0 for CAN1 others use 1 (sigh)

/* Specify msg buffer and max useage for TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
180,	/* Total number of msg blocks. */\
140,	/* TX can use this huge ammount. */\
40,	/* RX0 can use this many. */\
8	/* RX1 can use this piddling amount. */\
};

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
	int ret;

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
	USART1_txint_puts("\n\r#### SHAFT #### 04/25/2018 (../svn_sensor/sensor/shaft/trunk) \n\r");
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
	unsigned int ii;
	unsigned int jj = pcanidtbl->size;
	if (jj > 16) jj = 16;
	for (ii = 0; ii < jj; ii++)
	{
		printf("%2d %4d  %08X\n\r",(int)ii, (int)pcanidtbl->slot[ii].func, (int)pcanidtbl->slot[ii].canid);
	}
	printf("\n\r");
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	struct FUNC_CANID* pfunc = (struct FUNC_CANID*)&__paramflash0a;
	u32 canid_ldr = pfunc[0].func;
	printf("CANID_LDR  : 0x%08X\n\r", (unsigned int)canid_ldr );
	printf("TBL SIZE   : %d\n\r",(unsigned int)pfunc[0].canid);
	USART1_txint_send();

/* --------------------- CAN setup ---------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Initialize CAN for POD board (F103) and get control block */
	pctl1 = canwinch_setup_F103_pod(&msginit, canid_ldr); // ('can_ldr' is fifo1 reset CAN ID)

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
	tim4_shaft_init();	// Timer for miilsecond timing and 1/64th sec timing
	ret = shaft_function_init_all();
	if (ret < 0)
	{
		printf("FAIL: shaft_functions_init_all, return code %d\n\r",ret);USART1_txint_send(); 
		while (1==1);
	}
	if ((int)shaft_f.lc.size != ret)
	{
		printf("### WARNING ### shaft_functions_init_all return size, %d,not equal Number of elements in list %d\n\r",ret,(int)shaft_f.lc.size);
	}
	printf("PASS: shaft_functions_init_all, return is size: %d\n\r",ret);USART1_txint_send(); 

	shaft_printf(&shaft_f.lc);	// Print parameters in local copy
/* ------------------ Set up hardware CAN filter ----------------------------------------------------- */
	can_msg_reset_init(pctl1, pcanidtbl->unit_code);	// Specify CAN ID for this unit for msg caused RESET
	printf("\n\r## pcanidtbl->unit_code: 0x%08X\n\r",(int)pcanidtbl->unit_code);

	/* Go through table and load "command can id" into CAN hardware filter. */
	//                     (CAN1, even, bank 2)
	can_driver_filter_setbanknum(0, 0, 2);

	unsigned int id;
	jj = pcanidtbl->size;
	printf("pcanidtbl->size: %u\n\r",jj);USART1_txint_send(); 
	if (jj > 16) jj = 16;	// Check for bogus size
	for (ii = 0; ii < jj; ii++) 
	{
		id = pcanidtbl->slot[ii].canid;
		//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
		ret = can_driver_filter_add_one_32b_id(0,id,0);
		printf(" %2u 0x%08X added to filter\n\r",ii,id);
		if (ret < 0)
		{
			printf("FLASHH CAN id table load failed: %d\n\r",ret);USART1_txint_send(); 
			while (1==1);
		}
	}
//	ret = CAN_filter_build_list_add(); // Add previously built filter tables to CAN hardware
//	if (ret <= 0)
//	{
//		printf("\n\r### ERROR: CAN_filter_build_list_add 0x%X\n\r\n\r",ret);USART1_txint_send(); 
//	}
//	printf("CAN_filter_build_list_add: %d\n\r",(ret >> 8)); // Number should match list

//	CAN_filter_list_printf();	// List CAN hardware filter list for incoming msgs

	unsigned int filtnum = can_driver_hw_filter_list(1, -1, 0); // Get next available filter bank number
	printf("CAN 1 hardware filter list\n\r");
	printf("filtnum: %u  odd/even: %d\n\r",(filtnum >> 1),(filtnum & 0x1));
	int i;
	for (i = 0; i < (int)(filtnum >> 1); i++) // List the hw filter registers
	{
		id = can_driver_hw_filter_list(0, i, 0);	printf("%2d 0x%08X ",i,id);
		id = can_driver_hw_filter_list(0, i, 1);	printf("0x%08X\n\r",id);
	}
	printf("\n\r");
	USART1_txint_send(); 
/* ----------------------- Misc ---------------------------------------------------------------------- */
	pctl0 = pctl1;	// Save copy for those routines that use 0 instead of 1 (shameless hack)

	adc_init_sequence_foto_h(&shaft_f);	// ADC setup
	
/* --------------------- Start interrupts ------------------------------------------------------------ */
	adcsensor_foto_h_enable_interrupts(); 
//	NVICISER(NVIC_ADC1_2_IRQ);			// Enable interrupt controller for ADC1|ADC2
//	NVICISER(NVIC_ADC3_IRQ);			// Enable interrupt controller for ADC3

	tim4_shaft_enable_interrupts();
//	NVICISER(NVIC_EXTI0_IRQ);			// Enable interrupt controller 
//	NVICISER(NVIC_TIM4_IRQ);			// Enable interrupt controller for TIM4

	CAN_poll_loop_enable_interrupts();
//	NVICISER(NVIC_I2C1_ER_IRQ);			// Enable interrupt controller	

	ret = can_driver_enable_interrupts(); // CAN sending/receiving starts here.
	if (ret < 0)
	{
		printf("### FAIL ###\n\rCAN interrupt enable fail.  buffct in can_driver.c not zero\n\r");
	}
printf("READY FOR WHILE LOOP\n\r");USART1_txint_send(); 

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */
printf("tim4_tim_rate_shaft: %u hb_tdur: %u\n\r",(unsigned int)tim4_tim_rate_shaft,(unsigned int)shaft_f.cf.hb_tdur);

i = 6;

#ifdef THIS_MAY_BE_NEEDED_LATER
extern u32 adc2histo[2][ADCHISTOSIZE];
extern u32 adc3histo[2][ADCHISTOSIZE];
extern int adc2histoctr[2];
extern int adc3histoctr[2];
extern union ADC12VAL adc12valbuff[2][ADCVALBUFFSIZE];
extern unsigned short adc3valbuff[2][ADCVALBUFFSIZE];
#endif

/* Green LED flashing */
#define LEDPRINTFRINC 2000	// One per sec
uint32_t tim4_tim_ticks_next = tim4_tim_ticks + LEDPRINTFRINC;
extern void testfoto(void);

// Char buffer for fmtprint.c
char a[16];	// RPM
char b[32];

int encoder_prev = encoder_ctr2;
int encoder_diff;

extern int adcsensordb[5];
//int adcsensordb_prev[5];


//canwinch_pod_common_systick2048_printerr_header();
/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Green LED OK flasher */
		if ((int)(tim4_tim_ticks - tim4_tim_ticks_next) >= 0)
		{
			tim4_tim_ticks_next += LEDPRINTFRINC;
			TOGGLE_GREEN;	// Slow flash of green means "OK"
#ifdef ZXCVB
			fpformatn(a,shaft_f.drpm,10,1,10); // 
			encoder_diff = encoder_ctr2 - encoder_prev;
			encoder_prev = encoder_ctr2;
						
			printf("%5d %s %5d %6d ",i++,a, encoder_diff, (int)encoder_ctr2);

			for (j = 0; j < 4; j++)
			{
				printf(" %d",adcsensordb[j]);
//				adcsensordb_prev[j] = adcsensordb[j];
			} 
			printf("\n\r");USART1_txint_send();
#endif
		}
		if (adcsensordb[4] != 0)
		{
			adcsensordb[4] = 0;
			printf("%5d %2d %7d %d\n\r",i++,adcsensordb[0],adcsensordb[1],adcsensordb[3]);
		}
	}
	return 0;	
}

