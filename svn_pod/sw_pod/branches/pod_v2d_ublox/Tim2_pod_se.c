/******************************************************************************
* File Name          : Tim2_pod_se.c
* Date First Issued  : 06/02/2017
* Board              : POD board
* Description        : Input capture with TIM2 CH2 (PA1) on POD board.
* Note               : Use 'gps_1pps_se.h' with this routine
*******************************************************************************/
/*
01/29/2013 21:04 Rev 112 before big changes to Tim4, Tim2 timing scheme
02/09/2013 12:33 Rev 118 before big 8 possible combo interrupt changes

06/02/2017 Hack sensor/co1 ublox (logger) version for POD ublox, changing 
           TIM4 CH3 IC to TIM2 CH2
           TIM4 CH2 OC to TIM2 CH4.

*/

/*
Input capture is driven by the GPS 1 PPS.  This is used to phase the
OC (output capture) interrupt to produce calls to external routines
at 64 per second phased to the 1 PPS interrupts.

*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "irq_priorities_co1.h"
#include "bit_banding.h"
#include "pinconfig_all.h"
//#include "common.h"
//#include "common_time.h"
#include "gps_packetize.h"
#include "running_average.h"


/* Debug */
volatile unsigned int tim2debug0;
volatile unsigned int tim2debug1;
volatile unsigned int tim2debug2;
volatile unsigned int tim2debug3;
volatile unsigned int tim2debug4;
volatile unsigned int tim2debug5;
volatile unsigned int tim2debug6;
volatile unsigned int tim2debug7;
volatile unsigned int tim2ticks;
volatile int64_t tim2debug64a;

int db0;

unsigned int db1;
unsigned int db2;
unsigned int db3;
unsigned int db4;
unsigned int db5;
unsigned int db6;

volatile unsigned int tim2cyncnt; // DWT_CYCNT

volatile struct ALLTIMECOUNTS	strAlltime;	// All the time stuff lumped into this

/* Routine prototypes */
void TIM6_IRQHandler(void);

void (*p1_rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 32000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 64000000	*/

/* #### => POD: Base this on 48,000,000 TIM2 clock <= #### */

/* Number of OC interrupts in 1 sec */
#define TIM2_OC_PER_SEC	2048	// 

/* Number of OC interrupts in 1/64th sec */
#define TIM2_NUM_INTERVALS (TIM2_OC_PER_SEC/64)	// e.g. 2048/64 = 32; 8192/64 = 128
unsigned int tim2_num_intervals;


/* Each OC interrupt duration, nominal.  Scaling used */
#define TIM2_NOMINAL_SCALE 4	// sysclk_freq may not divide evenly, so scale upwards
unsigned int tim2_oc_ticks;  	// scaled processor ticks per OC interrupt, based on sysclk_freq
unsigned int tim2_oc_ticks_jam; // nominal ticks per OC rounded 

/* => SCALE <=  1 PPS interval durations to "whole.fraction" form. */
#define TIM2SCALE	16	// Number of bits to scale deviation of clock upwards

/* Various bit lengths of the timer counters are handled with a union */
//   Timer counter extended counts
volatile union TIMCAPTURE64	strTim2cnt;	// 64 bit extended TIM2 CH2 timer count
//   Input capture extended counts
volatile union TIMCAPTURE64	strTim2;	// 64 bit extended TIM2 CH2 capture
volatile union TIMCAPTURE64	strTim2m;	// 64 bit extended TIM2 CH2 capture (main)

volatile uint32_t tim2_ic;	// Number of processor ticks between IC interrupts
uint64_t tim2_ic_ave_scaled;	// 'tim2_ic' averaged and scaled
uint32_t tim2_ic_prev;		// Previous Tim2 extended IC counter

uint64_t ticks_per_oc_scaled;	// whole.fraction
uint32_t ticks_per_oc_whole;	// whole
uint32_t ticks_per_oc_fraction;	// fraction

int64_t tim2_diffphz_ave_scaled;	// Phase difference ('tim2_diff') averaged and scaled
int64_t ticks_per_ocphz_scaled; // 'tim2_diffphz_ave_scaled' per OC (whole.fraction)
int32_t ticks_per_ocphz_whole;  // whole
int32_t ticks_per_ocphz_fraction;	// fraction

/* The readings and flag counters are updated upon each capture interrupt */
volatile unsigned int		uiTim2ch2_Flag;	// Incremented when a new capture interrupt serviced, TIM2CH2

/* POD: TIM2 CH2 (PA1) GPS 1 PPS. Configure input, floating */
// (Logger) const struct PINCONFIGALL pb8  = {(volatile u32 *)GPIOB, 8, IN_FLT, 0};
const struct PINCONFIGALL pa1  = {(volatile u32 *)GPIOA, 1, IN_FLT, 0};

/* Various and sundry items to satisfy the meticulous programmer. */
u16 tim2_oc_ctr;	// 'tim2_interval_ctr' at capture time
u32 Tim2tickctr_prev;	// "Previous" tickctr
u32 tim2_ccr4;		// "Previous" OC time

/* Sanity limits for processor ticks in one second.  (Set at initialization) */
u32	tickspersecHI;	// TIM2 clock ticks for one sec: high limit
u32	tickspersecLO;	// TIM2 clock ticks for one sec: low limit

/* "whole.fraction" amounts for adjusting ticks in each OC interval */

//   deviation_oneinterval: difference between measured ticks between 1PPS pulses
//        and ideal (e.g. 48000000), then scaled upwards and divided by 64.
s32	deviation_oneinterval = 0;	// Interval adjustment for duration

//   phasing_oneinterval: difference between the IC and OC, scaled upwards
//        and divided by 64.
s32	phasing_oneinterval = 0;	// Interval adjustment for phasing

/* Running sums where the whole is added to an OC interval (and removed from the sum) */
int64_t	deviation_sum = 0;		// Running sum of accumulated error
int32_t	deviation_sum_g = 0;		// Save for 'g' command monitoring

int64_t phase_sum = 0;	// Running sum of phase difference, scaled

/* Count cases where there is a '000' interrupt flag isr entry (not expected) */
u32	tim2_zeroflag_ctr;

/* Processor tick measurement out-of-bounds */
uint32_t tim2_ticks_outofbounds;	// Count cases
uint32_t tim2_phase_oneOCplus;	
uint32_t tim2_phase_diffdiff;


/* Switch to show that the time is ready and logging can start */
u8	tim2_readyforlogging;

/* Ticks  */
unsigned int ticks;	// Number of clock ticks between 1 PPS
unsigned int ticks_flg;	// Incremented each 1 PPS

/* Subroutine prototypes */
static void tim2lowlevel_init(void);

/* Averaging processor ticks per second */
#define RUNNINGAVE_CT	8
struct RUNNING_AVE rave;

/* Averaging phase difference ticks per second */
#define RUNNINGPHZAVE_CT	8
struct RUNNING_AVE ravephz;

/******************************************************************************
 * void Tim2_pod_init(void);
 * @brief	: Initialize Tim2 for input capture
*******************************************************************************/
void Tim2_pod_init(void)
{
	/* Initialize for averaging processor ticks */
	running_average_init(&rave, RUNNINGAVE_CT, TIM2SCALE);

	/* Initialize for averaging 1 PPS versus OC phase difference */
	running_average_init(&ravephz, RUNNINGPHZAVE_CT, TIM2SCALE);

	/* Setup limits on tick counts +/- 10% */
	tim2lowlevel_init();

	/* Set processor tick count, nominal, for OC interrupts */
	// E.g. (48000000 << 4) / 2048 = 93750
	tim2_oc_ticks = (sysclk_freq << TIM2_NOMINAL_SCALE)/TIM2_OC_PER_SEC;

	// Nominal ticks for one OC (not scaled)
	tim2_oc_ticks_jam = (tim2_oc_ticks)/(1<<TIM2_NOMINAL_SCALE); // Not rounded

	/* Number of OC interrupts per 1/64th sec */
	// e.g. 2048/64 = 32; 8192/64 = 128
	tim2_num_intervals = TIM2_OC_PER_SEC/64;

/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CCNCNT (Data Watch counter)

	/* ----------- TIM2: CH2 input capture, CH4 output capture---------------------------------------*/
	/* Enable bus clocking for TIM2 (p 335 for beginning of section on TIM2-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;		//

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		//

	/*  Setup TIM2 CH2 (PA1) GPS 1 PPS */
	pinconfig_all((struct PINCONFIGALL *)&pa1);	

	/* TIMx capture/compare mode register 1 & 2 */
	TIM2_CCMR1  = (0x01 << 8);	// 01: CC2 channel is configured as input, IC2 is mapped on TI2

	TIM2_CCMR2  = (0x001 << 12);	// 001: CH4 Set to active level on match.
	TIM2_CCMR2 |= (0x00 << 8);	// 00:  CH4 configure as output (default)

	/* Compare/Capture Enable Reg */
	// CH2 Configured as input: rising edge trigger
	// CH4 Configured as output: active high
	TIM2_CCER  = (0x1 << 4); 	// 

	TIM2_CCR4 = tim2_oc_ticks_jam;	// Set first OC duration

	/* Set and enable interrupt controller for doing software interrupt */

	/* Set and enable interrupt controller for doing software interrupt */
        // Execute a series of routines for measurement and logging
	NVICIPR (NVIC_I2C2_EV_IRQ, NVIC_I2C2_EV_IRQ_PRIORITY_CO );	// Set interrupt priority 
	NVICISER(NVIC_I2C2_EV_IRQ);			// Enable interrupt controller

	/* Set and enable interrupt controller for TIM2 interrupt */
	NVICIPR (NVIC_TIM2_IRQ, TIM2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM2_IRQ);			// Enable interrupt controller for TIM1

	/* Control register 1 */
	TIM2_CR1  = TIM_CR1_CEN; 			// Counter enable: counter begins counting
	
	/* Enable output, input, and overflow capture interrupts */
	TIM2_DIER |= (TIM_DIER_CC4IE | TIM_DIER_CC2IE | TIM_DIER_UIE);

	return;
}
/******************************************************************************
 * unsigned int Tim2_gettime_ui(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim2_gettime_ui(void)
{
	union TIMCAPTURE64 strX;			

	strTim2cnt.us[0] = TIM2_CNT;	// Get current counter value (16 bits)

	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX.ui[0] = strTim2cnt.ui[0];	// Get low order word of current extended count
		strTim2cnt.us[0] = TIM2_CNT;	// Get current counter value (16 bits)
	}
	while ( strX.us[1] != strTim2cnt.us[1] );// Check if extended count changed on us
	return strX.ui[0];			// Return lower 32 bits
}
/******************************************************************************
 * struct TIMCAPTRET32 Tim2_inputcapture_ui(void);
 * @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim2_inputcapture_ui(void)
{
	struct TIMCAPTRET32 strY;			// 32b input capture time and flag counter
	__attribute__((__unused__))int	tmp;

	TIM2_DIER &= ~(TIM_DIER_CC4IE | TIM_DIER_CC2IE | TIM_DIER_UIE);	// Disable CH2 capture interrupt and counter overflow
	tmp = TIM2_DIER;				// Readback ensures that interrupts have locked

	strY.ic  = strTim2m.ui[0];			// Get 32b input capture time
	strY.flg = uiTim2ch2_Flag; 			// Get flag counter
	TIM2_DIER |= (TIM_DIER_CC4IE | TIM_DIER_CC2IE | TIM_DIER_UIE);	// Enable CH2 capture interrupt and counter overflow
	
	return strY;
}
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// 1 PPS starts OC in phase with pulse (which may have jitter). Diff in 1 PPS averaged to give better duration
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/* 
'ocphasing' is entered when there was an IC interrupt (caused by the gps 1 PPS).  The time between 1 PPS
interrupts is the count of the processor system clock ticks.  This varies slowly with temperature and has
shows a small amount of jitter.

Goal: Generate 2048 interrupts per second with the leading edge phased to the 1 PPS.

Strategy: average the ticks-per-second and divide that by 2048 to get the number of processor ticks per
output capture timing.  The remainder of the division, i.e the fractional part, is the amount not accounted
for in each OC duration.  Since these accumulate over the course of the 1 second interval the fraction is
summed each time the OC register is incremented to compute next OC duration and when the whole part of the sum
is greater than zero, the whole part is added to the duration, and also subtracted from the sum.  This injects
an extra tick periodically to account for the non-integer division.
*/

#define DIFFDIFF 250	// Phase difference limit
int64_t ticks_ave_scaled; // debug
int32_t tim2_diff;
uint32_t tim2_ccr4_last;

/*-----------------------------------------------------------------------------
 * void ocphasing(void);
 * @brief	: Compute value (whole.fraction) used in 'setnextoc()' to bring into phase at next 1 PPS
 -----------------------------------------------------------------------------*/
void reset_phase(uint32_t ctr)
{
	running_average_reset(&ravephz, 0);	// Reset phase average to zero
	strAlltime.SYS.ull &= ~(uint64_t)(TIM2_OC_PER_SEC-1);	// RE-Sync sub-second time
	TIM2_CCR4 = TIM2_CCR2 + ticks_per_oc_whole; // Load first OC reg with first OC counter ct
	TIM2_SR = ~0x10;	// Reset OC flag if it came on
	deviation_sum = ticks_per_oc_fraction;	// Start sum with fraction for first OC duration
	phase_sum = 0;		// Assume start is exactly in phase
	if (ctr != 0)
	{
		strAlltime.SYS.ull += 0x800; 	// Advance unix time 1 second
	}
	return;
}

void ocphasing(void)
{
tim2debug7 = tim2_oc_ctr;
tim2debug0 = db0;


	uint32_t oc_ctr;

	/* Discard out-of-bounds ticks per 1 PPS (e.g. the 1st 1 PPS after a gps no-lock). */
	if ( ( tim2_ic < tickspersecLO) || ( tim2_ic > tickspersecHI) ) // Allow +/- 10%
	{ // 
		tim2_ticks_outofbounds += 1;	// Count cases
		return;
	}

	// Duration
	tim2_ic_ave_scaled = running_average(&rave, tim2_ic);	// ticks per 1PPS: Averaged 
	ticks_per_oc_scaled = tim2_ic_ave_scaled / (uint64_t)TIM2_OC_PER_SEC;// Ticks per OC (scaled)
	ticks_per_oc_whole = (uint32_t)(ticks_per_oc_scaled >> TIM2SCALE);// Ticks per OC: Whole amount
	ticks_per_oc_fraction = (uint32_t)((ticks_per_oc_scaled) - (uint64_t)(ticks_per_oc_whole << TIM2SCALE));

	/* Low order bits of SYS time is sub-second tick count. */
	oc_ctr = strAlltime.SYS.ull & (uint64_t)(TIM2_OC_PER_SEC-1);

	/* The OC count should be within one OC around the 1 PPS. */
	if ( !((oc_ctr != 0) || (oc_ctr != (TIM2_OC_PER_SEC-1))) )
	{ // Here, 1 PPS occured more than +/- one OC interval.  Rephase needed
tim2debug6 = oc_ctr;
		reset_phase(oc_ctr);
		tim2_phase_oneOCplus += 1;	// Count cases
		return;
	}

	/* Determine phase difference between OC and 1 PPS */
	/* Are we in to the first OC interval? */	
	if (oc_ctr == 0)
	{ // Here, yes.  The OC terminated second ended before the 1 PPS interrupt second
		tim2_diff = (int)TIM2_CCR2 - (int)tim2_ccr4_last; // (IC - latest OC)	
		if (tim2_diff < 0) tim2_diff += 65536;	// Wrap around
	}
	else
	{ // Here, the last (most likely) OC has not completed
	
		tim2_diff = (int)TIM2_CCR4 - (int)TIM2_CCR2; // (OC - IC)
		if (tim2_diff < 0) tim2_diff += 65536;	// Wrap around
	
		tim2_diff = -tim2_diff;
	}
	/* tim2_diff: plus = OC before IC. minus = IC before OC (yet to occur) */

	if ((tim2_diff < -DIFFDIFF) || (tim2_diff > DIFFDIFF))
	{ // Phase difference is a bit large, so reset
		reset_phase(oc_ctr);	
		tim2_phase_diffdiff += 1;	// Count cases
		return;	
	}

	// Phase
	// PI control loop?
	tim2_diffphz_ave_scaled = running_average(&ravephz, tim2_diff) / 8; // diff ticks: averaged
	tim2_diffphz_ave_scaled += (tim2_diff << TIM2SCALE);	// Difference at this 1 PPS interrupt
	ticks_per_ocphz_scaled = tim2_diffphz_ave_scaled / (uint64_t)TIM2_OC_PER_SEC; // Ticks per OC (scaled)

	/* Remove 1/2 processor tick bias */
//	if (ticks_per_ocphz_scaled < 0) 
//		ticks_per_ocphz_scaled -=0x8000/TIM2_OC_PER_SEC; // Round downward

ticks_ave_scaled  = tim2_ic_ave_scaled; // Debug 'g' cmd

	return;
}

/*-----------------------------------------------------------------------------
 * void setnextoc(void);
 * @brief	: Set up next OC register as an increment from 'base'
 -----------------------------------------------------------------------------*/
void setnextoc(void)
{
	int32_t dur;
	int32_t phz;	

	uint32_t oc_ctr;

	TIM2_SR = ~0x10; // Reset OC interrupt flag. Routine was called because flag was on.

	/* Add to 1/2048 time count */
	strAlltime.SYS.ull += 1;	// Update 1/2048th sec ticks
	
	/* Low order bits of SYS time is 1/2048th tick count. */
	oc_ctr = strAlltime.SYS.ull & (TIM2_OC_PER_SEC-1);

	/* Call series of routines at TIM2_OC_PER_SEC (e.g. 2048) per sec rate */
	NVICISPR(NVIC_I2C2_EV_IRQ);	// Set pending (lower priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

	/* This is where the GPS date/time is sync'd to GPS. */
	// See if the gps sentence handling signals a new date/time is to be jammed
	if (gps_poll_flag != 0)
	{ // Here, gps_packetize found GPS and SYS times differ at the GPS x.0 second sentences
		gps_poll_flag = 0;	// Reset flag

		// Jam GPS date/time into SYS with lower 11 bits zero
		strAlltime.SYS.ull = strAlltime.GPS.ull & ~(TIM2_OC_PER_SEC-1);

		// Low order bit, phase sync is determined by 1 PPS in 'ocphasing', so continue to use it.
		strAlltime.SYS.ull |= oc_ctr;	
	} 

	/* Goal of following: set the OC register with the next OC time. */

	// The following is signed 'whole.fraction' += '0.fraction'
	deviation_sum += ticks_per_oc_fraction;	// Add scaled deviation to the running sum

	/* Get the (signed) whole of the 'whole.fraction' */
	dur = (deviation_sum/(1 << TIM2SCALE));	// dur = 'whole'

	/* Adjust the sum to account for the amount applied to the interval */
	//   Remove the 'whole' from 'whole.fraction' (Remember: this is signed)
	deviation_sum -= (dur << TIM2SCALE);	// Remove 'whole' from sum

	phase_sum += ticks_per_ocphz_scaled;
	phz = (phase_sum/(1 << TIM2SCALE));	// phz = 'whole'
	phase_sum -= (phz << TIM2SCALE);	// Remove 'whole' from sum

	// Add an increment of ("nominal" + duration adjustment + phase adjustment)
	tim2_ccr4_last = TIM2_CCR4;	// Save for possible later 1 PPS IC interrupt
	TIM2_CCR4 += (ticks_per_oc_whole + dur + phz);	// Set next interval end time

tim2debug3 = (uint32_t)phase_sum;
tim2debug4 += phz;	// running sum of whole
tim2debug64a = phase_sum;

db0 += 1;	// Count OC interrupts

	return;
}
/*#######################################################################################
 * ISR routine for TIM2
 *####################################################################################### */
void p1_TIM2_IRQHandler(void)
{
	int32_t diff;

	u16 usSR = TIM2_SR & 0x15;
	__attribute__((__unused__))volatile u32 temp;

	/* We don't expect this, but to comfirm such a bold expectation, we had better count them! */
	if (usSR == 0)		tim2_zeroflag_ctr += 1;

	/* Extended timing: Handle the 1 PPS input capture (CC2IF) and counter overflow (UIF) flags */
	switch (usSR & 0x5)
	{
	case 0x01:	// Overflow flag only
			TIM2_SR = ~0x1;				// Reset overflow flag
			strTim2cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the time counter
			break;
		
	case 0x04:	// Capture flag only
		strTim2.us[0] = TIM2_CCR2;		// Read the captured count which resets the capture flag
		strTim2.us[1] = strTim2cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
		strTim2.ui[1] = strTim2cnt.ui[1];	// Extended time of upper 32 bits of long long
		strTim2m = strTim2;			// Update buffered value		
		uiTim2ch2_Flag += 1;			// Advance the flag counter to signal mailine IC occurred

		/* Ticks since last 1 PPS */
		tim2_ic = strTim2.ui[0] - tim2_ic_prev;
		tim2_ic_prev = strTim2.ui[0];

		break;

	case 0x5:  	// Both flags are on	
		// Set up the input capture with extended time
		strTim2.us[0] = TIM2_CCR2;		// Read the captured count which resets the capture flag
		strTim2.us[1] = strTim2cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
		strTim2.ui[1] = strTim2cnt.ui[1];	// Extended time of upper 32 bits of long long

		// Take care of overflow flag
		TIM2_SR = ~0x1;				// Reset overflow flag
		strTim2cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the *time counter*

		// Adjust inpute capture: Determine which flag came first.  If overflow came first increment the overflow count
		if (strTim2.us[0] < 32768)		// Is the capture time in the lower half of the range?
		{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
			strTim2.ll	+= 0x10000;	// Increment the high order 48 bits if the *capture counter*
		}
		strTim2m = strTim2;			// Update buffered value		
		uiTim2ch2_Flag += 1;			// Advance the flag counter to signal mailine that an IC occurred

		/* Ticks since last 1 PPS */
		tim2_ic = strTim2.ui[0] - tim2_ic_prev;
		tim2_ic_prev = strTim2.ui[0];

		break;
	}

	/* OC output duration & phasing: Handle the 1 PPS input capture (CC2IF) and CH4 output capture (CC4IF) flags */
	switch (usSR & 0x14)	
	{ // Here, an OC interrupt flag
	case 0x04:	// IC only
			ocphasing();	// Phasing adjustment 
		break;
	case 0x10:	// OC only

			setnextoc();	// Set next interval for OC
		break;
	case 0x14:	// IC and OC were both on at isr entry (i.e. when usSR stored the SR)
			/* Is IC ahead of OC? */

			diff = ((int)TIM2_CCR2 - (int)TIM2_CCR4);	// IC - OC (registers)
			if (diff < 0)
				diff += 65536;

			if (diff > 32767)
			{ // Here, OC leads IC
				ocphasing();	// Phasing adjustment 
				setnextoc();	// Set up next OC time
			}
			else
			{ // Here, IC leads OC
				setnextoc();	// Set up next OC time
				ocphasing();	// Phasing adjustment 
			}
		break;
	}
	temp = TIM2_SR;	// Prevent tail-chaining
	return;
}
/*-----------------------------------------------------------------------------
 * static void tim2lowlevel_init(void);
 * @brief	: Compute +/- limits on reasonable processor ticks per second
 -----------------------------------------------------------------------------*/
/* This routine is a mere shadow of the original */
static void tim2lowlevel_init(void)
{
	unsigned int temp = ((2 * pclk1_freq)/10);	// Allow +/- 10% range on ticks per sec

	/* Set limits for valid processor ticks between 1 PPS events */
	tickspersecHI = (2 * pclk1_freq) + temp;
	tickspersecLO = (2 * pclk1_freq) - temp;
	return;
}
/*#######################################################################################
 * ISR routine for executing a series of routines for measurement and logging
 *####################################################################################### */
void I2C2_EV_IRQHandler(void)
{
	if (p1_rtc_secf_ptr != 0)	// Having no address for the following is bad.
		(*p1_rtc_secf_ptr)();	// Go do something (e.g. poll the AD7799)	
	return;
}

