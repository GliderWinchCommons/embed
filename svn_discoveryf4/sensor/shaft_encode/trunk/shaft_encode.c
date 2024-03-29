/******************************************************************************
* File Name          : shaft_encode.c
* Date               : 07/02/2017
* Board              : F4-Discovery
* Description        : Dual shaft encoder, CAN
*******************************************************************************/
/* 
TODO 
10-21-2013 Move 'Default_Handler' out of this routine
           Pass 'sysclk' to usb routine and delete 'SysInit' in startup.s
	   vector.c rather than .s and the problem with .weak
11-24-2013 Hack of 'gateway/gate.c' to change from usb to ftdi
12-01-2013 rev 132 Changed compression scheme to work with extended addresses
12-16-2013 rev 148 - bin/ascii switch, and better compression.  Works with mode 0 & 1 (bin & asc)
12-17-2013 rev 149 - moved CANuncompress, CANcompress calls into USB_PC_gateway.c, add CAN_error_msgs.c to count errors
12-17-2013 rev 150 - added mode 2 (Gonzaga ascii/hex format)
02-11-2014 rev 202 - fixed bug where PC->gatef msgs occasionally were garbled gatef->PC msgs
06-15-2015 rev 313 - last of 'canwinch_ldr', and begin switch to new linked list can_driver
06-17-2015 rev 317 - Switch to new linked list can_driver "appears to work"

*/
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


#include <math.h>
#include <string.h>
#include <stdio.h>
#include "xprintf.h"

#include <malloc.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/scb.h"

#include "../../../sw_discoveryf4/trunk/lib/libusartstm32f4/bsp_uart.h"
#include "systick1.h"
#include "clockspecifysetup.h"

#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board
#include "can_driver.h"
#include "common_can.h"
#include "panic_leds.h"
#include "PC_gateway_comm.h"
#include "USB_PC_gateway.h"
#include "libopencm3/stm32/systick.h"
//#include "CAN_test_msgs.h"
#include "CAN_error_msgs.h"
#include "canwinch_setup_F4_discovery.h"
#include "can_msg_reset.h"
#include "DTW_counter.h"
#include "db/gen_db.h"
#include "encoder_timers.h"
#include "running_average.h"
#include "ledf4.h"
#include "lltoflt.h"
#include "CAN_poll_loop.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "encoder_a_functionS.h"
#include "encoder_a_printf.h"
#include "can_filter_print_f4.h"
#include "can_driver_filter.h"


#ifndef NULL 
#define NULL	0
#endif

#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS // defined in encoder_timers.h file
#define ENCTEST64SIZE 128
/* Add standard deviation to line */
static int idxr;
static int idxr2;
static float enc_test64_sum;
static float reading64buf[ENCTEST64SIZE];
static int enc_test64_n;
#endif

/* Circular buffer for passing CAN BUS msgs to PC */
#define CANBUSBUFSIZE	64	// Number of incoming CAN msgs to buffer


/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit1 = { \
180,	/* Total number of msg blocks. */
80,	/* TX can use this huge ammount. */
80,	/* RX0 can use this many. */
16	/* RX1 can use this piddling amount. */
};

/* Specify msg buffer and max useage for CAN2: TX, RX0, and RX1. */
const struct CAN_INIT msginit2 = { \
180,	/* Total number of msg blocks. */
140,	/* TX can use this huge ammount. */
32,	/* RX0 can use this many. */
8	/* RX1 can use this piddling amount. */
};


/* USART|UART assignment for xprintf and read/write */
#define UXPRT	3	// Uart number for 'xprintf' messages
#define USTDO	2	// Uart number for gateway (STDOUT_FILE, STDIIN_FILE)

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
/* ------------- Each node on the CAN bus gets a unit number --------------------- */
//#define IAMUNITNUMBER	CAN_UNITID_GATE2	// PC<->CAN bus gateway
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

/* %%%%%%%%%%%%% Board hardware setup %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
#define SETUP_CAN1	// Include code for setup of CAN1
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%encoder_timers.h%%%%%%%%%%% */

/* The following values provide --
External 8 MHz xtal
sysclk  =  168 MHz
PLL48CK =   48 MHz
PLLCLK  =  168 MHz
AHB     =  168 MHz
APB1    =   42 MHz
APB2    =   84 MHz

NOTE: PLL48CK must be 48 MHz for the USB
*/

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl0;	// CAN1 control block pointer
struct CAN_CTLBLOCK* pctl1;	// CAN2 control block pointer

//static struct CANRCVBUF canrcvbuf;
//
struct CANRCVBUF* 	pc1r0;	// Pointer to CAN1 buffer for incoming CAN msgs, low priority
struct CANRCVBUF*	pc1r1;	// Pointer to CAN1 buffer for incoming CAN msgs, high priority

//static struct CANRCVBUF* ptest_pc;	// Test gateway->PC
//static struct CANRCVBUF* ptest_can;	// Test gateway->CAN bus

/* Gateway heart-beat msg */
#define DELAYHEARTBEAT	128	// Number of 1/64th sec ticks between HBs
static u32 t_hb;	// DTW ticks

/* Put sequence number on incoming CAN messages that will be sent to the PC */
u8 canmsgctr = 0;	// Count incoming CAN msgs

/* file descriptor */
int fd;

char vv[128];	// sprintf buf

static u32	t_cmdn;

/* **************************************************************************************
 * static void printmsg(struct CANRCVBUF* p);
 * @brief	: Simple hex print
 * @param	: p = pointer to msg
 * ************************************************************************************** */
#ifdef PRINTMSG
static void printmsg(struct CANRCVBUF* p, int sw)
{
	int i;
	int ct = (p->dlc & 0xf); if (ct > 8) ct = 8;
	xprintf (UXPRT, "F%1d %08X %d ",sw,p->id,ct);
	for (i = 0; i < ct; i++)
	  xprintf(UXPRT, "%02X ",p->cd.uc[i]);
	xprintf(UXPRT, "\n\r");
	return;
}
#endif

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
//	int tmp;
	int ret;
//u32 xctr = 1;
//u32 yctr = 0;
//u32 zctr = 0;

//	int temp;


/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup(canwinch_setup_F4_discovery_clocks()); // Get the system clock and bus clocks running
/* ---------------------- Set up pins ------------------------------------------------------------- */
	/* Configure pins */
	DISCgpiopins_Config();	// Configure pins
/* ---------------------- Set usb ----------------------------------------------------------------- */
//	usb1_init();	// Initialization for USB (STM32F4_USB_CDC demo package)
	setbuf(stdout, NULL);
/* --------------------- Initialize USART/UARTs ---------------------------------------------------- */
/* Regarding 'fprintf' and 'fopen'--(11-21-2013) this does not work.  'fprintf' (in 'newlib.c') does not call 
   '_write' in newlib_support.c'.  In the meantime the function of 'fprintf' is accomplished by using 'sprintf'
   followed by a 'puts' to send the string to the uart. 

   The strategy is to setup the USART/UART so that it will handle STDOUT, and STDIN, making 'printf' etc. work
   directly.  Bulk calls are made to _write, _read in subroutines, and these routines will work with the correct
   usart/uart via the 'fd' that relates the fd to uart control block during the call to _open.  Normally one would
   use 'fprintf' etc., but that isn't working, and this shameful sequence is an interim solution that allows easily
   changing the STDOUT, STDIN uart.

   USART2 and USART6 are shown below.  Select one, or make up one for the USART/UART this will be used.  
   Either DMA or CHAR-BY-CHAR interrupt driven can be used.  DMA for faster high volume loads.

*/
/*	DMA DRIVEN  (note: non-blocking, i.e. discard chars if port buff is full.) */
// int bsp_uart_dma_init_number(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority
//	u8 block, u8 nstop);
	bsp_uart_dma_init_number(USTDO,2000000, 2048, 1024, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 921600, 2048, 1024, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 460800, 256, 256, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 230400, 1024, 256, 5, 6, 0x10),1,0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 115200, 256, 256, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(UXPRT, 115200, 256, 256, 1, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.

/*	CHAR-BY-CHAR INTERRUPT DRIVEN  (note: non-blocking, i.e. discard chars if port buff is full.) */
// int bsp_uart_init_number(u32 iuart, u32 baud, u32 txbuffsize, u32 rxbuffsize,  u32 uart_int_priority	
//	u8 block, u8 nstop);
//	bsp_uart_int_init_number(USTDO, 460800,  256,  256, 0x30, 1, 0);
//	bsp_uart_int_init_number(USTDO, 230400, 4096, 1024, 0x40, 1, 0);
//	bsp_uart_int_init_number(USTDO, 921600, 4092, 1024, 0x40, 1, 0);
//	bsp_uart_int_init_number(USTDO, 115200,  256,  256, 0x10, 1, 0);

	bsp_uart_int_init_number(UXPRT, 115200, 256,  256, 0x30, 0, 0);

/* Setup STDOUT, STDIN (a shameful sequence until we sort out 'newlib' and 'fopen'.)  The following 'open' sets up 
   the USART/UART that will be used as STDOUT_FILENO, and STDIN_FILENO.  Don't call 'open' again!  */
	fd = open("tty2", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
//	fd = open("tty6", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
/* ---------------------- DTW sys counter -------------------------------------------------------- */
	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();
/* ---------------------- Let the hapless Op know it is alive ------------------------------------ */
	int i;
	/* Do this several times because it takes the PC a while to recognize and start 'ttyACM0' and some of
           the chars are missed.  No such problem with ttyUSBx, however. */
	for (i = 0; i < 1; i++) 
	{
		/* Announce who we are. ('xprintf' uses uart number to deliver the output.) */
		xprintf(UXPRT,  " \n\rF4 DISCOVERY SHAFT ENCODER FTDI w LINKED LIST CAN1 DRIVER: 07/16/2017 \n\r");
		/* Make sure we have the correct bus frequencies */
		xprintf (UXPRT, "   hclk_freq (MHz) : %9u...............................\n\r",  hclk_freq/1000000);	
		xprintf (UXPRT, "  pclk1_freq (MHz) : %9u...............................\n\r", pclk1_freq/1000000);	
		xprintf (UXPRT, "  pclk2_freq (MHz) : %9u...............................\n\r", pclk2_freq/1000000);	
		xprintf (UXPRT, " sysclk_freq (MHz) : %9u...............................\n\r",sysclk_freq/1000000);
	}

/* Unique device ID--12 bytes */
unsigned int uid[3];
uid[0] = *(unsigned int *)0x1fff7a10;
uid[1] = *(unsigned int *)0x1fff7a14;
uid[2] = *(unsigned int *)0x1fff7a18;
xprintf(UXPRT,"Unique device ID: 0x%08X 0x%08X 0x%08X\n\r",uid[0],uid[1],uid[2]);

/* Byte-by-byte unique device ID display */
xprintf(UXPRT,"Unique device ID: 0x%08X ",uid[0]);
unsigned char* puid = (unsigned char *)0x1fff7a14;
for (i = 0; i < 8; i++)
{
	if (*puid < (unsigned char)0x20)
		xprintf(UXPRT,"0x%02X ", *puid++);	
	else
		xprintf(UXPRT,"%c ", *puid++);
}
xprintf (UXPRT,"\n\r");

xprintf (UXPRT,"Flash size: %dK\n\r",(*(unsigned int*)0x1FFF7A22)&0xffff);

/* ---------------- Some test, monitoring, debug info ----------------------------------------------- */
	extern void* __paramflash0a;
	struct FLASHH2* pcanidtbl = (struct FLASHH2*)&__paramflash0a;
	xprintf (UXPRT,"FLASHH2 Address: %08X\n\r",(unsigned int)pcanidtbl);
	xprintf (UXPRT,"  CAN unit code: %08X\n\r",(int)pcanidtbl->unit_code);
	xprintf (UXPRT,"  Size of table: %d\n\r",(int)pcanidtbl->size);
	xprintf (UXPRT," # func  CMD CANID\n\r");
	u32 ii;
	u32 jj = pcanidtbl->size;
	if (jj > 16) jj = 16;
	for (ii = 0; ii < jj; ii++)
	{
		xprintf (UXPRT,"%2d %4d  %08X\n\r",(int)ii, (int)pcanidtbl->slot[ii].func, (int)pcanidtbl->slot[ii].canid);
	}
	printf("\n\r");
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	struct FUNC_CANID* pfunc = (struct FUNC_CANID*)&__paramflash0a;
//	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	u32 canid_ldr = pfunc[0].func;
	xprintf (UXPRT,"CANID_LDR  : 0x%08X\n\r", (unsigned int)canid_ldr );
	xprintf (UXPRT,"TBL SIZE   : %d\n\r",(unsigned int)pfunc[0].canid);
/* ------------------------------------------------------------------------------------------------------ */

/* Test double to float using inline asm */
#ifdef TEST_LLTOFLT_W_INLINE_ASM
extern float lltoflt(long long x);
extern void encoder_speed(struct ENCODERCOMPUTE *p);
long long llw = 5000000000000000;
struct ENCODERCOMPUTE enc_w;
enc_w.enr_prev.t.ll = 0;
enc_w.enr_prev.n = 0;
enc_w.enr.t.ll = llw;
enc_w.enr.n = 0;

xprintf(UXPRT,"xxx %016llX\n\r",llw);

encoder_speed(&enc_w);
double dtmp3 = enc_w.ft;
xprintf(UXPRT,"@@@ %15.8e\n\r",dtmp3);

double dtmp2 = lltoflt(llw);
xprintf(UXPRT,"### %15.8e\n\r", dtmp2 );
#endif

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/*  Pin usage for CAN1 on DiscoveryF4--
	PD01 CAN1  Tx LQFP 82 Header P2|33 WHT
	PD00 CAN1  Rx LQFP 81 Header P2|36 BLU
	GRN  (MCP2551 RS pin) Grounded for high speed
	*/
	/* Setup CAN registers and initialize routine */
	pctl0 =  canwinch_setup_F4_discovery(&msginit1, 1);	// Number msg bufferblocks, CAN1

	/* Check if initialization was successful. */
	if (pctl0 == NULL)
	{
		xprintf(UXPRT,"CAN1 init failed: NULL ptr\n\r");
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	if (pctl0->ret < 0)
	{
		xprintf(UXPRT,"CAN1 init failed: return code = %d\n\r",pctl0->ret);
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	xprintf(UXPRT,"CAN1 initialized OK@ baudrate %d\n\r",CANWINCH_BAUDRATE);
	can_driver_enable_interrupts();	// Enable CAN interrupts
	xprintf(UXPRT,"CAN interrupts enabled\n\r");

/* --------------------- Monitoring incoming CAN ids  ------------------------------------------------- */
	can_msg_reset_init(pctl0, 0xffe00000);	// Specify CAN ID for this unit for msg caused RESET

/* ----------- Go through table and load "command can id" into CAN hardware filter. ------------------- */
	//                     (CAN1, even, bank 2)
	can_driver_filter_setbanknum(0, 0, 2);
	jj = pcanidtbl->size;
	if (jj > 16) jj = 16;	// Check for bogus size
	u32 id;
	for (ii = 0; ii < jj; ii++) 
	{
		id = pcanidtbl->slot[ii].canid;
		//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
		ret = can_driver_filter_add_one_32b_id(0,id,0);
		if (ret < 0)
		{
			xprintf(UXPRT,"FLASHH CAN id table load failed: %d\n\r",ret);
			while (1==1);
		}
	}

/* ---------------- Encoder functions ---------------------------------------------------------------- */
	ret = encoder_a_functionS_init_all();
	if (ret <= 0)
	{
		printf("encoder_a_functionS: table size mismatch count: %d\n\r", ret); 
		while(1==1);
	}
	xprintf(UXPRT,"encoder_a_functionS: table size : %d\n\r", ret);
/* ----------------- Debug parameters ----------------------------------------------------------------- */
for (i = 0; i < NUMENCODERAFUNCTIONS; i++)
{
	xprintf(UXPRT,"\n\rENCODER #%1d values\n\r",i+1);
	encoder_a_printf(&enc_f[i].enc_a);	// Print parameters
}
/* ----------------- CAN filter registers ------------------------------------------------------------- */
	can_filter_print_f4(17);	// Print the CAN filter registers
/* ------------------------ CAN msg loop (runs under interrupt) --------------------------------------- */
	ret = CAN_poll_loop_init();
	if (ret != 0)
	{ // Here the init failed (e.g. malloc)
		xprintf(UXPRT,"CAN_poll_loop_init: failed %d\n\r",ret);
		while (1==1);		
	}
/* --------------------- Timer and Encoder setup ----------------------------------------------------------------- */
	encoder_timers_init(0x00200000); // Pass reset CAN id to routine

/* ---------------- When CAN interrupts are enabled reception of msgs begins! ------------------------ */
	can_driver_enable_interrupts();	// Enable CAN interrupts

/* Test disable/enable global interrupts */
//__asm__ volatile ("CPSID I");
//__asm__ volatile ("CPSIE I");

t_cmdn = DTWTIME + 168000000; // Set initial time
u32 hb_inc = DELAYHEARTBEAT * (sysclk_freq/64);
t_hb = DTWTIME + hb_inc;

uint32_t encode_oc_ticks_prev = 3;  // Check TIM3 1/64sec ticking

struct ENCODERCOMPUTE enc_main[2];	// Encoder count and time

/* Calibration for test & debug */
//float enc_cal[2] = {(84E6/720), (84E6/720)};	   // Rev per sec
float enc_cal[2] = {84E6/(720/60), (84E6/(720/60))}; // Rev per min

/* Distance per pulse based on 7 inch dia pulley. */
#define FEETPERPULSE	2.54527182E-03
#define METERSPERPULSE	7.75798852E-04

double dtmp;

/* --------------------- Endless Polling Loop ----------------------------------------------- */
	while (1==1)
	{
		/* Flash the LED's to amuse the hapless Op or signal the wizard programmer that the loop is running. */
/*		if (((int)(DTWTIME - t_led)) > 0) // Has the time expired?
		{ // Here, yes.
			toggle_4leds(); 	// Advance some LED pattern
			t_led += FLASHCOUNT; 	// Set next toggle time
		}
*/
		/* Have LEDs follow encoder phase signals */
		encoder_leds();

		/* Check that the TIM3 OC is timing correctly (yes, and it didn't!) */
		if (encode_oc_ticks != encode_oc_ticks_prev)
		{ // Here, 1/64th sec tick incremented the flag
			encode_oc_ticks_prev = encode_oc_ticks;
			if ((encode_oc_ticks & 0x01f) == 0)
			{ // Here, end of one second 
				xprintf(UXPRT,"\n\rTIM3_OC: %5d ",(int)encode_oc_ticks>>6);
				for (i = 0; i < 2; i++) // Do both encoders
				{
					encoder_get_all(&enc_main[i],i);     // Readings and rate computation
					dtmp = (enc_main[i].r * enc_cal[i]); // Apply temporary test calibration
					xprintf(UXPRT," %8d %10d %10lld %10.2f",enc_main[i].enr.n,enc_main[i].dn,enc_main[i].dt,dtmp);
//					enc_main[i].enr_prev = enc_main[i].enr; // Update '_prev' for next time
				}


// Amount of cable-out converted to meters
double dtmp3 = (lltoflt(enc_main[1].enr.n))*METERSPERPULSE;
xprintf(UXPRT," %10.2f",dtmp3 );

//xprintf(UXPRT," #%4d",encoder_get_reading_loop_cnt);

#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS // defined in encoder_timers.h file

float vsum2 = 0;
float vmean = enc_test64_sum/enc_test64_n;
float vartmp;
float stdfin = -999;
int vi = 0;

while (idxr2 != idxr) // Go through all that was buffered
{
   vartmp = (reading64buf[idxr2] - vmean); //(x(i) - xbar)
   vsum2 += vartmp * vartmp;
   idxr2 += 1; if (idxr2 >= ENCTEST64SIZE) idxr2 = 0;
   vi += 1;
}
vartmp = vsum2 / (vi - 1);
if (vartmp > 0)
{
   stdfin = sqrtf(vartmp);
}

double dr1 = vmean; // Convert for printf
double dr2 = stdfin;
xprintf(UXPRT," x%10.2f std%10.3f",dr1,dr2);
enc_test64_n = 0; // Reset count for next round
enc_test64_sum = 0; // Reset sum for mean
#endif

#ifdef  IMDRIVECALIBRATION
xprintf(UXPRT," rev%4d %lld er2: %d", im_rev, im_tmp, im_n_er);
xprintf(UXPRT," icn: %d",&enc_main[1].enr.icn -  &enc_main[1].enr.n);
#endif

			}
		}

#ifdef IC_TO_IC_TIME_W_BIG_BUFFER
/* Compute and list the buffer when it fills, then reset the index for the next round */
static struct ENCODERCOMPUTE enc_test; // For readings-> computed stuff

if (enr_test_ct >= ENCTESTBUFFSIZE-1)
{ // Here, buffer full
	xprintf(UXPRT,"BUFFFULL\n\r");	
	for (i = 0; i < ENCTESTBUFFSIZE; i++)
	{
		enc_test.enr_prev = enc_test.enr;
		enc_test.enr = enr_test[i]; 
		encoder_speed(&enc_test);
		dtmp = (enc_test.r * enc_cal[1]); // Apply temporary test calibration
		xprintf(UXPRT," %8d %10d %12lld %12.4f %d\n\r",enc_test.enr.n,enc_test.dn,enc_test.dt,dtmp, encoder_get_reading_loop_cnt);
	}
    	        enr_test_ct = 0; // Reset for next run
}
#endif


#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS // defined in defined in encoder_timers.h file
/* Store readings each OC interval completion (maybe 1/64th sec) in circular buffer */
struct ENCODERREADING* p64;
struct ENCODERCOMPUTE enc_test64;

while(1==1)
{
p64 = encoder_getOC64(); // Get from circular buffer in 'encoder_timers.c'
if (p64 != 0) // Check for new data
{ // Here, data is available and p64 points to it!
   enc_test64.enr_prev = enc_test64.enr; // Update previous
   enc_test64.enr = *p64; // Copy new reading
   encoder_speed(&enc_test64); // Compute speed
   enc_test64.r = (enc_test64.r * enc_cal[1]); // Quick calibration
   enc_test64_sum += enc_test64.r; // Build sum for mean
   reading64buf[idxr] = enc_test64.r; // Save for std dev computation
   enc_test64_n += 1; // Number of instances for mean and std dev
   idxr += 1; if (idxr >= ENCTEST64SIZE) idxr = 0; // Round we go
}
else
  break;
}

#endif

#ifdef  IMDRIVECALIBRATION
static uint32_t im_idx_str_prev;
uint64_t imtotaltime;
double davetime;
double dim;
int im;
int imx;
/* Skip computation & output until buffer complete */
if (im_idx_str != im_idx_str_prev) // Did interrupt routine change buffers?
{ // Here, end of a buffer of accumulation
  im_idx_str_prev = im_idx_str; // Update previous
  // Total time of test data accumulation
  imtotaltime = imcaltime_end - imcaltime_begin;
  // Average time between IC's per segment
  davetime = imtotaltime;
  davetime /= (IMCANTESTREVCTMAX*IMCALTESTSEGMENTS);
  imx = im_idx_str_prev ^ 1; // Get buffer index for buffer interrupt routine not using
  xprintf(UXPRT,"\n\r##### IMTEST ####\n\rnumber of revs: %d total time: %lld avetime: %12.4f buff: %d 2er: %d\n\r",IMCANTESTREVCTMAX,imtotaltime,davetime, imx, im_n_er);
  for (im = 0; im < IMCALTESTSEGMENTS; im++) // List buffer
  {
	dim = imcalacum[imx][im];
	dim = dim / IMCANTESTREVCTMAX;
	xprintf(UXPRT,"%3d %15.5f\n\r",im, dim);
	imcalacum[imx][im] = 0;
  }
  xprintf(UXPRT,"##### IMTEST END ####\n\r");
}

#endif

// =============End of test stuff ==========================

	/* Done with a pass of this endless loop: trigger CAN poll */
		CAN_poll_loop_trigger();

	}
	return 0;	
}

