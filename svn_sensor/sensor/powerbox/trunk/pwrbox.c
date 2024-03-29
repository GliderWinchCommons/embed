/******************************************************************************
* File Name          : pwrbox.c
* Date First Issued  : 06/13/2018
* Board              : F103 Arduino Blue Pill
* Description        : Power monitoring
*******************************************************************************/
/* 
Some info on the Blue Pill

https://wiki.stm32duino.com/index.php?title=Blue_Pill#128_KB_flash_on_C8_version

https://wiki.stm32duino.com/images/c/c1/Vcc-gnd.com-STM32F103C8-schematic.pdf

SWD-- 

 Blue Pill: Orient pins pointing horizontally to the right, top view
   4 top    GND.... blk
   3 mid    swclk.. wht
   2 mid    swd.... gry
   1 bottom 3.3v... NC 

 Discovery board--Orient with SWD header at lower right corner, top view
   6 (top)... NC
   5 ........ NC
   4 ........ gry
   3 ........ blk
   2 ........ wht
   1 (bottom) NC

	Two jumper removed on Discovery Board



06/13/2018  Hack of tension.c 
07/03/2015
02-14-2015 Hack of tilt.c (which is not debugged)
02-13-2015 Hack of se4_h.c w reference to adcpod.c
02-05-2014 Hack of se4.c routine

Strategy--
04/24/2016: I beseech you to read README.tension, and you
may be better informed than just the following...then maybe not.

07/xx/2015:
After intialization--
  main: endless loop
    computes temperatures from thermistor readings
    computes tension offset adjustment factor
    printf for monitoring
    triggers a low level interrupt to poll send/handle CAN msgs
  timer3: interrupts 2048/sec
    Runs "last ad7799 reading" through a filter.
    Starts a SPI readout if there is a reading ready
  CAN RX0, RX1: 
    triggers low level interrupt to poll send/handle CAN msgs 
 
Interrupt levels
main             = background loop: printf, temperature computation, etc.
NVIC_I2C1_ER_IRQ = Polling loop for handling/sending msgs 'CAN_poll'
NVIC_I2C1_EV_IRQ = SPI polling loop (high priority than 'ER')
DMA1 CH1         = ADC
TIM1CH1          = CAN_poll in absence of CAN msgs
CAN_TX           = CAN1 xmit
CAN_RX0          = CAN1 FIFO0 (lower priority)
CAN_RX1          = CAN1 FIFO1 (high priority msgs)
UART1            = Teletype output
*/

#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
extern int errno;

#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "SENSORpinconfig.h"

//#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/scb.h"

#include "libusartstm32/usartallproto.h"

//#include "libmiscstm32/printf.h"

#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/DTW_counter.h"

#include "libsensormisc/canwinch_setup_F103_pod.h"

#include "../../../../svn_common/trunk/common_can.h"

#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"

#include "poly_compute.h"

//#include "canwinch_pod_common_systick2048_printerr.h"
#include "pwrbox_idx_v_struct.h"
#include "pwrbox_function.h"
#include "pwrbox_printf.h"
#include "temp_calc_param.h"
#include "CAN_poll_loop.h"
#include "libmiscstm32/DTW_counter.h"
#include "adcsensor_pwrbox.h"
#include "can_nxp_setRS.h"
//#include "p1_initialization.h"
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_msg_reset.h"
#include "can_gps_phasing.h"
#include "sensor_threshold.h"
#include "panic_leds_Ard.h"
#include "db/gen_db.h"
#include "CAN_poll_loop.h"
#include "adcsensor_pwrbox.h"
#include "../../../../svn_common/trunk/common_highflash.h"

#include "fmtprint.h"
#include "iir_filter_lx.h"
#include "pwrbox_idx_v_struct.h"
#include "tim3_ten2.h"
#include "can_filter_print.h"

/* Prototypes */
static void filtered_calibrate(int i, int j);

/* ############################################################################# */
#define USEDEFAULTPARAMETERS	0	// 0 = Initialize sram struct with default parameters
/* ############################################################################# */

int dummy;

/* Make block that can be accessed via the "jump" vector (e.g. 0x08005004) that points
   to 'unique_can_block', which then starts Reset_Handler in the normal way.  The loader
   program knows the vector address so it can then get the address of 'unique_can_block'
   and get the crc, size, and data area via a fixed offset from the 'unique_can_block'
   address. 

   The .ld file assures that the 'unique_can_block' function is is followed by the data
   by using the sections.  */
//#include "startup_deh.h"
//__attribute__ ((section(".ctbltext")))
//void unique_can_block(void)
//{
//	Reset_Handler();
//	while(1==1);
//}

/* The order of the following is important. */
//extern const struct FLASHP_SE4 __highflashp;
//__attribute__ ((section(".ctbldata")))
//const unsigned int flashp_size = sizeof (struct FLASHP_SE4);
//__attribute__ ((section(".ctbldata1")))
//const struct FLASHP_SE4* flashp_se3 = (struct FLASHP_SE4*)&__highflashp;

/* Subroutine declarations. */
void ciccalibrateprint(int n);

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl0;

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
180,	/* Total number of msg blocks. */
140,	/* TX can use this huge ammount. */
64,	/* RX0 can use this many. */
8	/* RX1 can use this piddling amount. */
};

#define GRNLED 13		// PC13 is one and only green led on Blue Pill
#define GRNLEDPORT GPIOC
const struct PINCONFIGALL pin_led1 = {(volatile u32 *)GRNLEDPORT, GRNLED, OUT_OD, MHZ_2};

static void pinconfigA(void)
{
	/* Enable all the ports */
	RCC_APB2ENR |= (0x7 << 2) | (0x1); // Enable Ports A,B,C and AFIO

	/* On-board LED */
	pinconfig_all( (struct PINCONFIGALL *)&pin_led1);	

	return;
}

void toggle_1led(void)
{
	if ((GPIO_ODR(GPIOC) & (1<<GRNLED)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOC) = (1<<GRNLED);	// Set bit
	}
	else
	{ // HEre, LED bit was on
		GPIO_BRR(GPIOC) = (1<<GRNLED);	// Reset bit
	}
	return;	
}
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

void double_to_char(double f,char * buffer){
    gcvt(f,12,buffer);
}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i;
//	int j;
	int ret;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
//$	relocate_vector();
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	// Start system clocks using parameters matching CAN setup parameters for POD board
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
	pinconfigA();	// Enable ports and setup RED led

	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();
setbuf(stdout, NULL);
//setvbuf( stdout, 0, _IONBF, 0 ); 
//	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine
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
	USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\r====> PWRBOX--06/13/2018\n\r");USART1_txint_send();

	/* Display things so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

//#define TESTINGTHEFPMESS
#ifdef TESTINGTHEFPMESS
toggle_1led();
unsigned int k1,k2;
char ttbuf[96];
double ttf = -0.49999;

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();

ttf = 0.49999;
double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();


ttf = 5000000000.001;
printf("\n\r");

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();

ttf = -45678912345.6789;
printf("\n\r");

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();

ttf = 0;

printf("\n\r");

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();

printf("DONE\n\r");USART1_txint_send();

toggle_1led();
while(1==1);

#endif

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
	GPIO_BSRR(GPIOE) = (0xf<<LED3);	// Set bits = all four LEDs off

	/* Initialize CAN for POD board (F103) and get control block */
	// Set hardware filters for FIFO1 high priority ID & mask, plus FIFO1 ID for this UNIT
	pctl0 = canwinch_setup_F103_pod(&msginit, canid_ldr); // ('can_ldr' is fifo1 reset CAN ID)

	/* Check if initialization was successful. */
	if (pctl0 == NULL)
	{
		printf("CAN1 init failed: NULL ptr\n\r");USART1_txint_send(); 
		while (1==1);
	}
	if (pctl0->ret < 0)
	{ // Here, an error code was returned.
		printf("CAN init failed: return code = %d\n\r",pctl0->ret);USART1_txint_send(); 
		while (1==1);
	}
/* ------------------------ Setup CAN hardware filters ------------------------------------------------- */
	can_msg_reset_init(pctl0, pcanidtbl->unit_code);	// Specify CAN ID for this unit for msg caused RESET
ret = 0;
/* ##### THIS NEEDS TO BE REPLACED with lookup in highflash #####	
	// Add CAN IDs this function will need *from* the CAN bus.  Hardware filter rejects all others. */
	//                                      ID #1                    ID#2                 FIFO  BANK NUMBER
//$	ret  = can_driver_filter_add_two_32b_id(CANID_CMD_TENSION_a11,  CANID_MSG_TIME_POLL,   0,     2);
//	ret  = can_driver_filter_add_two_32b_id(CANID_DUMMY,  0x30400000,   0,     2);
//$	ret |= can_driver_filter_add_two_32b_id(CANID_CMD_TENSION_a21,  CANID_MSG_TIME_POLL,   0,     3);
//$	ret |= can_driver_filter_add_two_32b_id(CANID_CMD_CABLE_ANGLE_1, CANID_DUMMY,          0,     4);
	if (ret != 0)
	{
		printf("filter additions: failed init\n\r");USART1_txint_send(); 
		while (1==1);		
	}

//	/* Setup unit CAN ID in CAN hardware filter. */
	u32 id;
//	id = pcanidtbl->unit_code;
//	//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
//	ret = can_driver_filter_add_one_32b_id(0,id,1);
//	if (ret < 0)
//	{
//		printf("FLASHH unit CAN ID failed: %d\n\r",ret);USART1_txint_send(); 
//		while (1==1);
//	}

	/* Go through table and load "command can id" into CAN hardware filter. */
	//                     (CAN1, even, bank 2)
	can_driver_filter_setbanknum(0, 0, 2);
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
/* ---------------- tension A ------------------------------------------------------------------------ */
	ret = pwrbox_function_init_all();
	if (ret <= 0)
	{
		printf("pwrbox_function: table size mismatch count: %d\n\r", ret);USART1_txint_send(); 
//$		while(1==1);
	}
	printf("pwrbox_function: table size : %d\n\r", ret);USART1_txint_send();

/* ----------------- CAN filter registers ------------------------------------------------------------- */
	can_filter_print(14);	// Print the CAN filter registers
/* ----------------- Debug parameters ----------------------------------------------------------------- */
	printf("\n\rPWRBOX values\n\r");
	pwrbox_printf(&pwr_f.pwrbox);	// Printf set of parameters
/* ------------------------ Various and sundry ------------------------------------------------------- */
	// The following replaces the hacked: p1_initialization();
	/* Get SAR ADC initialized and calibrated */
	adcsensor_pwrbox_sequence();	// Initialization sequence for the adc

/* ------------------------ CAN msg loop (runs under interrupt) --------------------------------------- */
	ret = CAN_poll_loop_init();
	if (ret != 0)
	{ // Here the init failed (e.g. malloc)
		printf("CAN_poll_loop_init: failed %d\n\r",ret);USART1_txint_send(); 
		while (1==1);		
	}
/* ---------------- When CAN interrupts are enabled reception of msgs begins! ------------------------ */
USART1_txint_puts("\n\rTestPt 1 p1_initialization and CAN init\n\r");USART1_txint_send();

	can_driver_enable_interrupts();	// Enable CAN interrupts

USART1_txint_puts("\n\rTestPt 2 CAN interrupts now enabled\n\r");USART1_txint_send();toggle_1led();

/* ---------------- Some vars associated with endless loop ------------------------------------------- */
#define FLASHTIMEINC (64000000/2)	   // Green on-board LED flash tick duration
u32 ledtime = DTWTIME + FLASHTIMEINC;	// Init the first timeout
u32 tim3_ten2_ticks_prev = tim3_ten2_ticks;

/* --------------- ADC startup ----------------------------------------------------------------------- */
	adcsensor_pwrbox_sequence();
USART1_txint_puts("\n\rTestPt 3 ADC initialized\n\r");USART1_txint_send();

/* --------------- Start TIM3 CH1 and CH2 interrupts ------------------------------------------------- */
	tim3_ten2_init();	// 64E6/2048

/* --------------- Local things for endless loop tinkering ------------------------------------------- */
USART1_txint_puts("\n\rTestPt 4 tim3_ten2 initialized\n\r");USART1_txint_send();

extern uint32_t adc_readings_buf[RBUFSIZE][NUMBERADCCHANNELS_PWR];
extern uint32_t adc_readings_buf_idx_i;
extern uint32_t adc_readings_buf_idx_o;

uint32_t adc_ave[NUMBERADCCHANNELS_PWR];
uint32_t adc_ave_ct = 0;
uint32_t *pave;
uint32_t *pbuf;
double dave[NUMBERADCCHANNELS_PWR];
double dvol[NUMBERADCCHANNELS_PWR];
double diir[NUMBERADCCHANNELS_PWR];
uint8_t dvolbyte[NUMBERADCCHANNELS_PWR];
char dbuf[96];
double dcur;
double dGR = 1000.0/0.22;	// Reciprocal of series resistor

extern uint32_t adcdb1;

/* Long term tracking */
long long llivref_accum = 0;
long long llitemp_accum = 0;
long long ll5vreg_accum = 0;
double ddivref;
double dditemp;
double dd5vreg;
double drecip;
uint32_t llaccumct = 0;
#define LTTRACKINGCT (4096*20)	// Long term accumulation count
char dbuw[32] = {'n','a',0};
char *strnone = " ";

struct ADCCALIB
{
	double volts_in;
	double volts_adc;
	double ave_count;
};

#define ADCEXT 6	// Number of external ADC inputs
#define ADCINT 2	// Number of internal ADC inputs


/* Convert voltage & count to a ratio for calibration */
double dcal[NUMBERADCCHANNELS_PWR];
for ( i = 0; i < NUMBERADCCHANNELS_PWR; i++)
{
   dcal[i] = pwr_f.pwrbox.adc[i].scale; // Flashed parameter
}

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* ---------- Tick time flashing ---------- */
		if ( ((int)ledtime - (int)DTWTIME) < 0) 
		{
			ledtime += FLASHTIMEINC;
			toggle_1led();

			printf("%d ", (int)(tim3_ten2_ticks - tim3_ten2_ticks_prev));
			tim3_ten2_ticks_prev = tim3_ten2_ticks;

			for (i = 0; i < NUMBERADCCHANNELS_PWR; i++)
			{
				dave[i] = adc_ave[i];
				dave[i] /= (NUMBERSEQUENCES*adc_ave_ct);
				
				dvol[i] = dave[i] * dcal[i]; 
				fpformat(&dbuf[0],dvol[i]);
//				printf(" %s", dbuf);

				fpformat(&dbuf[0],dave[i]);
//				printf(" %s", dbuf);

				printf("%6d ", adc_ave[i]/(NUMBERSEQUENCES*adc_ave_ct));

				// Additional accumulations for long term tracking
				if (i == ADCX_IVREF) llivref_accum += adc_ave[i];
				if (i == ADCX_ITEMP) llitemp_accum += adc_ave[i];
				if (i == ADCX_5VREG) ll5vreg_accum += adc_ave[i];				

				// Zero accumulations for next round
				adc_ave[i] = 0;					
			}

			llaccumct += (NUMBERSEQUENCES*adc_ave_ct);
			if (llaccumct >= LTTRACKINGCT)
			{
				ddivref = llivref_accum;
				dditemp = llitemp_accum;
				dd5vreg = ll5vreg_accum;
				drecip = 1.0/llaccumct;
				ddivref *= drecip;
				dditemp *= drecip;
				dd5vreg *= drecip;
				llivref_accum = 0;
				llitemp_accum = 0;
				ll5vreg_accum = 0;

				fpformatn(&dbuw[0],ddivref,10000,4,12);

/* **********************************************************/
void fpformatn(char *p, double d, int n, int m, int q);
/* @brief	: Convert double to formatted ascii, e.g. ....-3.145
 * @param	: d = input double 
 * @param	: n = scale fraction, (e.g. 1000)
 * @param	: m = number of decimal of fraction, (e.g. 3)
 * @param	: q = number of chars total (e.g. 10)
 * @param	: p = pointer to output char buffer
*********************************************************** */
            llaccumct     = 0;
			}
			else
           strcpy(dbuw,strnone);


			adc_ave_ct = 0;

			/* Calibrated iir filtered reading--beware indices */
			filtered_calibrate(ADCX_INPWR, IIRX_INPWR); // Input power
			filtered_calibrate(ADCX_CANVB, IIRX_CANBV); // CAN bus
			filtered_calibrate(ADCX_HALLE, IIRX_HALLE); // Hall-effect sensor
			filtered_calibrate(ADCX_5VREG, IIRX_5VREG); // 5V regulator
	
				printf(":%s", dbuw);					

			printf("\n\r"); // Last but not least
		}

		/* Further accumulate accumlated readings until time to printf */
		pave = &adc_ave[0];
		while (adc_readings_buf_idx_o != adc_readings_buf_idx_i)
		{
			pbuf = &adc_readings_buf[adc_readings_buf_idx_o][0];
			for (i = 0; i < NUMBERADCCHANNELS_PWR; i++)
			{
				*(pave+i) += *(pbuf+i);
			}
			adc_ave_ct += 1;	// divisor for averaging
			adc_readings_buf_idx_o += 1; // Circular buffer index
			if (adc_readings_buf_idx_o >= RBUFSIZE) adc_readings_buf_idx_o = 0;
		}

		/* ---------- Trigger a pass through 'CAN_poll' to poll msg handling & sending. ---------- */
		CAN_poll_loop_trigger();
	}
	return 0;	
}
/* **************************************************************************************
 * static void filtered_calibrate(int i, int j);
 * @brief	: Calibration
 * @param	: i = index of ADC reading
 * @param	: j = index of iir filter associated with ADC (see adcsensor_pwrbox.c)
 * ************************************************************************************** */
static void filtered_calibrate(int i, int j)
{
	char dbut[32];
	struct PWRBOXFUNCTION *p = &pwr_f;

printf("  %d:",i);
			/* Calibrate iir filtered reading--beware indices */
				p->diir[i]  = p->adc_iir[j].z;   // Convert filtered reading to double
				p->diir[i] /= (NUMBERSEQUENCES*p->adc_iir[j].pprm->scale); // De-scale
            // Apply calibration
				p->diir[i]  = (p->diir[i] - p->pwrbox.adc[i].offset) * p->pwrbox.adc[i].scale;   
				fpformat(&dbut[0],p->diir[i]);printf(" %s", dbut); // print

				/* Convert to integer: volts * 10 */
//				p->dvolbyte[i] = ((p->diir[i] + 0.05)* 10.0);
//				printf("% 5d", p->dvolbyte[i]);
	return;
}


