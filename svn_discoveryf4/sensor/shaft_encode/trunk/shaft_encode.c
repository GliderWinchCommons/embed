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
#include "CAN_test_msgs.h"
#include "CAN_error_msgs.h"
#include "canwinch_setup_F4_discovery.h"
#include "can_msg_reset.h"
#include "DTW_counter.h"
#include "db/gen_db.h"
#include "encoder_timers.h"
#include "running_average.h"
#include "ledf4.h"


#ifndef NULL 
#define NULL	0
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
/* ######################### GATEWAY HEARTBEAT CAN ID ############################ */
static struct CANRCVBUF can_hb = {CANID_HB_GATEWAY1}; // See CANID_INSERT.sql
//static struct CANRCVBUF can_hb = {CANID_HB_GATEWAY2}; // See CANID_INSERT.sql
//static struct CANRCVBUF can_hb = {CANID_HB_GATEWAY3}; // See CANID_INSERT.sql
/* ############################################################################### */
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
struct CAN_CTLBLOCK* pctl1;	// CAN1 control block pointer
struct CAN_CTLBLOCK* pctl2;	// CAN2 control block pointer

static struct PCTOGATEWAY pctogateway; 	// CAN->PC
static struct PCTOGATEWAY pctogateway1; // PC->CAN
//static struct CANRCVBUF canrcvbuf;
//
static struct CANRCVBUF* 	pc1r0;	// Pointer to CAN1 buffer for incoming CAN msgs, low priority
static struct CANRCVBUF*	pc1r1;	// Pointer to CAN1 buffer for incoming CAN msgs, high priority

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
/* **************************************************************************************
 * static int CAN_gateway_send(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF* pg);
 * @brief	: Setup CAN message for sending
 * @param	: pg = Pointer to message buffer (see common_can.h)
 * @return	: 0 = OK; -1 = dlc greater than 8; -2 = illegal extended address
 * ************************************************************************************** */
static int CAN_gateway_send(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF* pg)
{
	/* Check number of bytes in payload and limit to 8. */
	if ((pg->dlc & 0x0f) > 8) 
		return -1;	// Payload ct too big
	
	/* Check if an illegal id combination */
	if ( ((pg->id & 0x001ffff9) != 0) && ((pg->id & 0x04) == 0) ) 
	{ // Here, in the additional 18 extended id bits one or more are on, but IDE flag is for standard id (11 bits)
		return -2; // Illegal id
	}

	/* Add msg to CAN outgoing buffer. */
	return can_driver_put(pctl, pg, 8, 0);
}


/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int tmp;
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

	bsp_uart_int_init_number(UXPRT, 115200,  128,  512, 0x30, 0, 0);

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


/* Testing of floating pt. */
volatile int idelay = 90000;
while (idelay-- > 0);

volatile float a = .14159265;
volatile float x;
volatile unsigned int f0 = DTWTIME;
x = a * 12.7 + 10.1;
volatile unsigned int f1 = DTWTIME;
xprintf (UXPRT, "fp test: %10.6f",(double)x);
volatile unsigned int f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

idelay = 90000;
while (idelay-- > 0);

f0 = DTWTIME;
x = atanf(a);
f1 = DTWTIME;
xprintf (UXPRT, "atanf test: %10.6f",(double)x);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

f0 = DTWTIME;
volatile double aa = .1415926535897932;
double xx = atan(aa);
f1 = DTWTIME;
xprintf (UXPRT, "atan  test: %10.6f",xx);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

f0 = DTWTIME;
volatile long double aaa = .1415926535897932;
long double xxx = atanl(aaa);
f1 = DTWTIME;
xprintf (UXPRT, "atanl test: %10.6f",xxx);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

/* Test double to float using inline asm */
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


/* --------------------- Timer setup ----------------------------------------------------------------- */
	encoder_timers_init(0x00200000);

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/*  Pin usage for CAN1 on DiscoveryF4--
	PD01 CAN1  Tx LQFP 82 Header P2|33 WHT
	PD00 CAN1  Rx LQFP 81 Header P2|36 BLU
	GRN  (MCP2551 RS pin) Grounded for high speed
	*/
	/* Setup CAN registers and initialize routine */
	pctl1 =  canwinch_setup_F4_discovery(&msginit1, 1);	// Number msg bufferblocks, CAN1

	/* Check if initialization was successful. */
	if (pctl1 == NULL)
	{
		xprintf(UXPRT,"CAN1 init failed: NULL ptr\n\r");
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	if (pctl1->ret < 0)
	{
		xprintf(UXPRT,"CAN1 init failed: return code = %d\n\r",pctl1->ret);
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	xprintf(UXPRT,"CAN1 initialized OK@ baudrate %d\n\r",CANWINCH_BAUDRATE);
	can_driver_enable_interrupts();	// Enable CAN interrupts
	xprintf(UXPRT,"CAN interrupts enabled\n\r");
/* --------------------- Initialize the time for the test msg generation ------------------------------ */
	CAN_test_msg_init();

/* --------------------- Monitoring incoming CAN ids  ------------------------------------------------- */

	can_msg_reset_init(pctl1, 0xffe00000);	// Specify CAN ID for this unit for msg caused RESET


/* --------------------- Hardware is ready, so do program-specific startup ---------------------------- */
#define FLASHCOUNT 21000000;	// LED flash
//u32	t_led = DTWTIME + FLASHCOUNT; // Set initial time

	PC_msg_initg(&pctogateway);	// Initialize struct for CAN message from PC
	PC_msg_initg(&pctogateway1);	// Initialize struct for CAN message from PC

	/* Set modes for routines that receive and send CAN msgs */
	pctogateway.mode_link = MODE_LINK;
	pctogateway1.mode_link = MODE_LINK;
#ifdef ENCODERAFUNCTIONSETUPSTUFF
/* ---------------- Encoder A function ---------------------------------------------------------------- */
	ret = encoder_a_functionS_init_all();
	if (ret <= 0)
	{
		printf("encoder_a_functionS: table size mismatch count: %d\n\r", ret); 
		while(1==1);
	}
	xprintf(UXPRT,"tension_a_functionS: table size : %d\n\r", ret);USART1_txint_send();

/* ----------------- CAN filter registers ------------------------------------------------------------- */
	can_filter_print(14);	// Print the CAN filter registers
/* ----------------- Debug parameters ----------------------------------------------------------------- */
for (i = 0; i < NUMTENSIONFUNCTIONS; i++)
{
	xprintf(UXPRT,"\n\rENCODER #%1d values\n\r",i+1);
	tension_a_printf(&enc_f[i].enc_a);	// Print parameters
}
/* ------------------------ CAN msg loop (runs under interrupt) --------------------------------------- */
int ret;
	ret = CAN_poll_loop_init();
	if (ret != 0)
	{ // Here the init failed (e.g. malloc)
		xprintf(UXPRT,"CAN_poll_loop_init: failed %d\n\r",ret);USART1_txint_send(); 
		while (1==1);		
	}
#endif
/* ---------------- When CAN interrupts are enabled reception of msgs begins! ------------------------ */
	can_driver_enable_interrupts();	// Enable CAN interrupts

/* Test disable/enable global interrupts */
__asm__ volatile ("CPSID I");

__asm__ volatile ("CPSIE I");

t_cmdn = DTWTIME + 168000000; // Set initial time
u32 hb_inc = DELAYHEARTBEAT * (sysclk_freq/64);
t_hb = DTWTIME + hb_inc;

uint32_t encode_oc_ticks_prev = 3;  // Check TIM3 1/64sec ticking

struct ENCODERCOMPUTE enc_main[2];	// Encoder count and time
float enc_cal[2] = {1.0, (84E6/720)};		// Calibration for test & debug
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
			if ((encode_oc_ticks & 0x1f) == 0)
			{ // Here, end of one second 
				xprintf(UXPRT,"\n\rTIM3_OC: %5d ",(int)encode_oc_ticks>>6);
				for (i = 0; i < 2; i++) // Do both encoders
				{
					encoder_get_all(&enc_main[i],i);     // Readings and rate computation
					dtmp = (enc_main[i].r * enc_cal[i]); // Apply temporary test calibration
					xprintf(UXPRT," %8d %10d %10lld %11.5f",enc_main[i].enr.n,enc_main[i].dn,enc_main[i].dt,dtmp );
//					enc_main[i].enr_prev = enc_main[i].enr; // Update '_prev' for next time
				}
extern unsigned int debugirq1;
extern unsigned int debugirq2;
xprintf(UXPRT," %d %d",debugirq1,debugirq2);

				LED_TOGGLE(GRN); // Green LED (Encoder #2 uses)
			}
		}

		/* Send heart-beat periodically. */
		if (((int)(DTWTIME - t_hb)) > 0) // Has the time expired?
		{ // Here, yes.
			t_hb += hb_inc; 			// Set next toggle time
			tmp = CAN_gateway_send(pctl1, &can_hb);	// Add to xmit buffer (if OK)
			Errors_CAN_gateway_send(tmp);		// Count any error returns
		}
		/* ================= CAN1 --> ? ================================================================= */
		while ( (pc1r1 = can_driver_peek1(pctl1)) != 0)	// Did we receive a HIGH PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer ;)
//printmsg(pc1r1, 1);
			can_driver_toss1(pctl1); // Release buffer block, fifo1 linked list
		}

		while ( (pc1r0 = can_driver_peek0(pctl1)) != 0)		// Did we receive a LESS-THAN-HIGH-PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer
//printmsg(pc1r0, 0);
			can_driver_toss0(pctl1); // Release buffer block, fifo0 linked list
		}

	}
	return 0;	
}

