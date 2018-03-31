/******************************************************************************
* File Name          : se1.c
* Date First Issued  : 10/15/2014
* Board              : RxT6
* Description        : Sensor board with pressure, rpm, throttle pot, thermistor
*******************************************************************************/
/* 
 ======================
NOTE: 'adcsensor_eng.c' in 'static void adc_cic_filtering(void)' 
is where the CAN messages get setup and sent.
 ======================

04/19/2014 CAN message rev 427
 number/sec canID description
*1) 64 0x30800000 Throttle & thermistor
 2) 64 0x40800000 RPM & Manifold Pressure
 3)  2 0x70800000 Temperature (deg C x100)
 4) 64 0x80800000 Throttle (range 0 - 4095)
* = commented out in 'adcsensor.eng.c' (not needed)

02/01/2013 Rev 115 Changes to add Tim4_pod_common.c for time syncing.
Initial = POD6 routine.

10/15/2014: change to have ldr.c CAN ID retrieved from ldr.c vector, and
            highflash area for app CAN IDs and other parameters.

Open minicom on the PC with 115200 baud and 8N1.

03/23/2018 Major revision to use database and "functions"

*/

/* &&&&&&&&&&&&& Each node on the CAN bus gets a unit number &&&&&&&&&&&&&&&&&&&&&&&&&& */
#define IAMUNITNUMBER	CAN_UNITID_SE1	// Sensor board #1 w pressure, throttle pot, thermistor, rpm
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */


/* Calibrations, parameters, etc., that apply to this application--FLASHP
   The placement in flash is controlledl by the .ld file.
   The following struct must match the order/index in the .txt file the PC uses.
   The PC stores the data as unsigned ints into an array  

*/
/* Example .txt file the PC uses 
//i   0       4     "%f"    -0.5	@ Manifold pressure offset
//i   1       4     "%f"   9.629E-3	@ Manifold pressure scale (inch hg)
//i   2       4     "%f"   0.01         @ Transmission temperature (deg C)
*/

#ifdef thisisoldstuffthatrevisionwillcastoff
struct FLASHP_SE1
{
	unsigned int crc;	// crc-32 placed by loader
	float offset_mp;	// Manifold pressure offset
	float scale_mp;		// Manifold pressure scale (inch hg)
	float scale_temp;	// Transmission temperature (deg C)
	unsigned int jic[3];	// Some spares
};

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
extern const struct FLASHP_SE1 __highflashp;
__attribute__ ((section(".ctbldata")))
const unsigned int flashp_size = sizeof (struct FLASHP_SE1);
__attribute__ ((section(".ctbldata1")))
const struct FLASHP_SE1* flashp_se1 = (struct FLASHP_SE1*)&__highflashp;
#endif


#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "scb.h"

#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
//#include "libmiscstm32/systick1.h"
//#include "libmiscstm32/printf.h"

#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/DTW_counter.h"
#include "canwinch_setup_F103_pod.h"
//#include "../../../sw_f103/trunk/lib/libsensormisc/canwinch_setup_F103_pod.h"

#include "adcsensor_eng.h"
#include "canwinch_pod_common_systick2048.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "SENSORpinconfig.h"
#include "sensor_threshold.h"
//#include "rw_eeprom.h"
#include "se1.h"
//#include "tick_pod6.h"
#include "panic_leds.h"
#include "rpmsensor.h"
#include "temp_calc.h"
//#include "CANascii.h"
#include "canwinch_pod_common_systick2048_printerr.h"

#include "CAN_poll_loop.h"
#include "can_nxp_setRS.h"
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_msg_reset.h"
#include "can_gps_phasing.h"
#include "sensor_threshold.h"
#include "db/gen_db.h"
#include "CAN_poll_loop.h"
#include "../../../../svn_common/trunk/common_highflash.h"

#include "fmtprint.h"
#include "iir_filter_l.h"
#include "can_filter_print.h"

#include "engine_idx_v_struct.h"
#include "engine_function.h"

/* Error counts for monitoring. */
extern struct CANWINCHPODCOMMONERRORS can_errors;	// A group of error counts

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;
struct CAN_CTLBLOCK* pctl0;	// Some routines use 0 for CAN1 others use 1 (sigh)

/* For test with and without XTAL clocking */
#ifdef usepriortomassiveupdateofse1program
//#define NOXTAL 
#ifdef NOXTAL

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */

/* NOTE: APB2 is set 32 MHz and the ADC set for max divide (divide by 8).  The slower ADC helps the 
   accuracy in the presence of source impedance. */

// INTERNAL RC osc parameters -- 64 MHz
const struct CLOCKS clocks = { \
HSOSELECT_HSI,	/* Select high speed osc 			*/ \
PLLMUL_16X,		/* Multiplier PLL: 0 = not used 		*/ \
0,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#else

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_8X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};
#endif

#endif


/* Parameters for setting up CAN */

// Default: based on 72 MHz clock|36 MHz AHB freqs--500,000 bps, normal, port B
//const struct CAN_PARAMS can_params = CAN_PARAMS_DEFAULT;	// See 'canwinch_pod.h'

// Experimental CAN params: Based on 64 MHz clock|32 MHz AHB freqs
struct CAN_PARAMS can_params = { \
0xfffffffc,	// CAN ID for this unit
500000,		// baudrate
0,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
2,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
3,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 4)
4,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 9)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero = freeze during debug
0,		// ttcm: CAN_MCR[7] Time triggered communication mode
1,		// abom: CAN_MCR[6] Automatic bus-off management
0,		// awum: CAN_MCR[5] Auto WakeUp Mode
0		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once)
};

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
 * void prog_checksum_loader(void);
 * @brief	: Do program checksum/prog load until given "GO AHEAD" command
 * ************************************************************************************** */
void prog_checksum_loader(void)
{	
//int sw_startup  = 0;	// while loop lock
/*
	// Endless loop until checksum gets checked, (or power down and restart the whole mess)
	while (sw_startup == 0)
	{
		// [We might want a timeout on this while]
		while ((canbuffptr = can_unitid_getmsg()) == 0); // Wait for "my unitid" msg

		// Msg is for us, so ignore UNITID.  Look at DATAID, other bits, RTR, IDE
		switch ((canbufptr->id) & ~CAN_UNITID_MASK)
		{
		case CAN_CHECKSUMREQ: // Checksum request: contains start|end addresses
			can_send_checksum (canbufptr);	// Respond with what we computed
			break;

		case (CAN_LOADER_ERASE | CAN_RIxR_RTR): // Loader payload: contains erase block address
			can_loader_erase(canbufptr);	// Returns only when finished
			can_send_loader_response();	// Let sender know we are ready
			break;

		case CAN_LOADER_DATA: // Loader payload: contains 8 bytes of program data
			can_loader_data(canbufptr);	// Returns only when finished
			can_send_loader_response();	// Let sender know we are ready
			break;
		
		case (CAN_DATAID_GO | CAN_RIxR_RTR): // Checksumming complete: "Go ahead" command
			sw_startup = 1;	// Break 
			break;

		case CAN_DATAID_RESET: // Software forced RESET command
			system_reset();	// Cause RESET, never return.
			break;
		
	}
	return;
*/
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
	int i = 0; 		// Timing loop variable
	int init_ret = -4;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();

/* --------------------- Begin setting things up -------------------------------------------------- */ 

//	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );

/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins

	/* Use DTW_CYCCNT counter for startup timing */
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
	i = USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers
	if (i != 0) panic_leds(7);	// Init failed: Bomb-out flashing LEDs 7 times

	/* Announce who we are */
	USART1_txint_puts("\n\rSE1: 03-30-2018 v1\n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();


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
	can_nxp_setRS_sys(0,0); // (1st arg) 0 = high speed mode; 1 = standby mode (Sets yellow led on)

	/* Initialize CAN for POD board (F103) and get control block */
	// Set hardware filters for FIFO1 high priority ID & mask, plus FIFO1 ID for this UNIT
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

/* -------------------- Functions init --------------------------------------------------------------- */
	engine_functions_init_all();

	eng_man_printf(&eman_f.lc); // Print parameters copied from highflash
	eng_rpm_printf(&erpm_f.lc);
	eng_thr_printf(&ethr_f.lc);
	eng_t1_printf(&et1_f.lc);

/* --------------------- ADC setup and initialization ------------------------------------------------ */
	adc_init_se_eng_sequence(can_params.iamunitnumber);	// Time delay + calibration, then start conversion
/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

i = 0;

/* CAN tx test */
//struct CANRCVBUF can_msg;
//can_msg.id = IAMUNITNUMBER | 0x05000000;
//can_msg.id = CAN_UNITID_POD6;	// PD board hack sensor ID
//can_msg.cd.ui[0] = 0x76543210;	// 1st 4 bytes of msg
//can_msg.cd.ui[1] = 0xfedcba98;		// 2nd 4 bytes: if zero, then "n" in msg is only 4 bytes
// can_msg.cd.ull = 0xfedcba9876543210;
//   can_msg.cd.ull = 0x00000000abcd0000;

/* CAN rx test */
//struct CANRCVTIMBUF * pFifo1 = 0;	// FIFO 1 buffer receive msg pointer
//struct CANRCVBUF * pFifo0 = 0;		// FIFO 2 buffer receive msg pointer

//int can_put = -1;
//int k = 0;
//int m = 0;

//extern u32 CAN_ticks;
//u32 CAN_ticks_prev = 0;
//extern s32 CAN_dev;
//extern s32 CAN_ave;
//extern s32 CAN_dbg1;
//extern s32 CAN_dbg2;
//extern s32 CAN_dbg3;
//extern s32 CAN_dif;
//extern u32	stk_val;
//extern u32	can_msgovrflow;	
//u32 		stk_64flgctr_prev1;

u32	uiT;
u32	uiT_prev = 0;
u32 	tctr = 0;
//int tx;
//int tx_prev = 0;
int sum = 0;
//int sctr = 0;
struct TIMCAPTRET32 strT = {0,0};

/* ADC testing */
#define ADCCOUNT 168000000/8;
u32	t_adc = *(volatile unsigned int *)0xE0001004 + ADCCOUNT; // Set initial time
extern unsigned int cicdebug0;
unsigned int cicdebug0_prev = 0;

/* Green LED flashing */
static u32 stk_64flgctr_prev;
static u32 throttleLED = 0;
static struct CANRCVBUF *pcan;

	/* Print the header for the CAN driver error counts */
//	canwinch_pod_common_systick2048_printerr_header();

/* --------------- Start TIM3 CH1 and CH2 interrupts ------------------------------------------------- */
	tim3_ten2_init(pclk1_freq/2048);	// 64E6/2048

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Green LED OK flasher */
		if (stk_64flgctr != stk_64flgctr_prev)
		{
			stk_64flgctr_prev = stk_64flgctr;
			throttleLED += 1;
			if (throttleLED >= 64)
			{
				throttleLED = 0;
				TOGGLE_GREEN;	// Slow flash of green means "OK"

				/* Print the counters in 'canwinch_pod_common_systick2048' */
//				canwinch_pod_common_systick2048_printerr(&can_errors);
			}
//			printf("%7d %4d %2d %7d %7d %7d\n\r",CAN_ave,CAN_dbg1, CAN_dbg2,CAN_dbg3,CAN_dif, CAN_dev); USART1_txint_send();
		}

		/* Poll & compute calibrated temperature */
		temp_calc(); // Floating pt computation done at mainline priority

		/* ---------- Trigger a pass through 'CAN_poll' to poll msg handling & sending. ---------- */
		CAN_poll_loop_trigger();

	}
	return 0;	
}

