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

// Logger /* #### => Base this on 64,000,000 TIM4 clock <= #### */
/* #### => POD: Base this on 48,000,000 TIM2 clock <= #### */

/* Number of intervals in 1/64th sec */
#define TIM2_NUM_INTERVALS	20	// 1/64th sec is covered by 20 intervals (i.e. interrupts)

/* Each interrupt duration, nominal. */
//#define TIM2NOMINAL	50000	// At 64,000,000 p1clk_freq, (50000 * 20) -> 1/64th sec
#define TIM2NOMINAL	37500	// At 48,000,000 p1clk_freq, (37500 * 20) -> 1/64th sec

/* Scale interval durations scale deviation to "whole.fraction" form. */
#define TIM2SCALE	18	// Number of bits to scale deviation of clock upwards

/* Various bit lengths of the timer counters are handled with a union */
//   Timer counter extended counts
volatile union TIMCAPTURE64	strTim2cnt;	// 64 bit extended TIM2 CH2 timer count
//   Input capture extended counts
volatile union TIMCAPTURE64	strTim2;	// 64 bit extended TIM2 CH2 capture
volatile union TIMCAPTURE64	strTim2m;	// 64 bit extended TIM2 CH2 capture (main)

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
static u32	tickspersecHI;	// TIM2 clock ticks for one sec: high limit
static u32	tickspersecLO;	// TIM2 clock ticks for one sec: low limit

/* "whole.fraction" amounts for adjusting ticks in each OC interval */

//   deviation_oneinterval: difference between measured ticks between 1PPS pulses
//        and ideal (e.g. 48000000), then scaled upwards and divided by 64.
s32	deviation_oneinterval = 0;	// Interval adjustment for duration

//   phasing_oneinterval: difference between the IC and OC, scaled upwards
//        and divided by 64.
s32	phasing_oneinterval = 0;	// Interval adjustment for phasing

/* Running sums where the whole is added to an OC interval (and removed from the sum) */
s32	phasing_sum = 0;		// Running sum of accumulated phase adjustment
s32	deviation_sum = 0;		// Running sum of accumulated error

/* Running count of instances where measured tickspersec of clock is outside limits. */
u32	tim2_tickspersec_err;

/* Count cases where there is a '000' interrupt flag isr entry (not expected) */
u32	tim2_zeroflag_ctr;

/* Switch to show that the time is ready and logging can start */
u8	tim2_readyforlogging;

/* Ticks  */
unsigned int ticks;	// Number of clock ticks between 1 PPS
unsigned int ticks_dev;	// Number of ticks between 1 PPS less nominal
unsigned int ticks_flg;	// Incremented each 1 PPS


/* Count of instances where strAlltime.ull lower 6 bits were not zero at the end of second */
u16	tim2_64th_0_er = 0;	// Count of date/time adjustments at interval 0
u16	tim2_64th_19_er = 0;	// Count of date/time adjustments at interval 19
static u16	tim2_64th_0_er_prev = 0;	// Count of date/time adjustments at interval 0, previous
static u16	tim2_64th_19_er_prev = 0;	// Count of date/time adjustments at interval 19, previous
static u16	tim2_64th_0_ctr = 0;	// Count of changes of 'tim2_64th_0_er'
static u16	tim2_64th_19_ctr = 0;	// Count of changes of 'tim2_64th_19_er'

/* Index for double buffer that switches each second when GPS 1 PPS (ic) arrives. */
volatile u32 idx_cmd_n_ct;	// Index for double buffering can msg counts


/* Subroutine prototypes */
static void tim2lowlevel_init(void);


/******************************************************************************
 * void Tim2_pod_init(void);
 * @brief	: Initialize Tim2 for input capture
*******************************************************************************/
void Tim2_pod_init(void)
{
	/* Setup limits */
	tim2lowlevel_init();


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

	/* Control register 1 */
	TIM2_CR1  = TIM_CR1_CEN; 			// Counter enable: counter begins counting

	/* Set and enable interrupt controller for doing software interrupt */
	NVICIPR (NVIC_TIM6_IRQ, TIM6_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_TIM6_IRQ);			// Enable interrupt controller for RTC ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for TIM2 interrupt */
	NVICIPR (NVIC_TIM2_IRQ, TIM2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM2_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM2_DIER |= (TIM_DIER_CC4IE | TIM_DIER_CC2IE | TIM_DIER_UIE);	// Enable CH2 capture interrupt and counter overflow (p 376,7)

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
/*-----------------------------------------------------------------------------
 * void ocphasing(void);
 * @brief	: Compute value (whole.fraction) used in 'setnextoc()' to bring into phase at next 1 PPS
 -----------------------------------------------------------------------------*/
/* The following "jams" interval 0 each time there is a 1 PPS interrupt.  Works good as long as
   the xtal osc is stable second-to-second, which seems to be the case where it varies only a couple
   of ticks between 1 PPS interrupts. */
//{
//	if ((strAlltime.SYS.ull & 0x3f) != 0)
//	{
//		strAlltime.SYS.ull &= ~0x3f;	// Reset the lower order 1/64th tick counts
//		tim2_64th_0_er += 1;		// Count these for the hapless programmer
//	}	
//	deviation_sum = 0;	// Reset running sum for the next 1 second
//	tim2_oc_ctr = 0;	// Interval counter

//	return;
//}

/* This is the code for locking the tick to the 1 PPS. Left here jic we want to revive the concept. */
/* 
We enter this when there is a 1 PPS IC.  The goal is to either synchronize by jamming
in new values to the OC interrupt scheme if we are way off of correct phasing.
Or, if we are in the 0 or 19 intervals of the 1/64th sec, then we compute a small 
adjustment in the form (whole.fraction) to bring it into close phase over the next
1 sec of OC interrupts. 

Item:  When we come into this routine, as part of the 1 PPS IC handling, the OC may have *preceded*
the IC.  When this happens the oc counter might be at 19, whereas is the OC had been handled first, it
would be at 0.  Hence, the 19 case is checked to see if the OC time difference is "way off".
*/

void ocphasing(void)
{
	s32	diff;
	__attribute__((__unused__))volatile u16 tmp;

tim2debug7 = tim2_oc_ctr;


	if (tim2_oc_ctr == 0)	// Are we in the 1st interval of 1/64th sec?
	{ // Here, the ending OC was after the 1 PPS, so longer OC interval times are needed
		/* Compute the time difference */
		diff = ((int)TIM2_CCR2 - (int)tim2_ccr4);	// (1 PPS IC - OC)
		if (diff < 0)	diff += 65536;	// Adjust for the wrap-around
tim2debug0 = diff;
		if (diff > 12000) diff = 12000;

		/* 'phasing_oneinterval' = ticks difference / number of intervals in 1 second */
		phasing_oneinterval = +( (diff << (TIM2SCALE - 6)) / TIM2_NUM_INTERVALS );	// Interval = whole.fraction

		if ((strAlltime.SYS.ull & 0x3f) != 0)
		{
			strAlltime.SYS.ull &= ~0x3f;	// Reset the lower order 1/64th tick counts
			tim2_64th_0_er += 1;		// Count these for the hapless programmer
		}
	}
	else
	{
		if (tim2_oc_ctr == (TIM2_NUM_INTERVALS - 1)) // Are we in the 20th (last) interval of 1/64th sec?
		{ // Here, the OC ending has yet to occur, so shorter OC interval times are needed
			/* Compute the time difference */
			diff = ((int)TIM2_CCR4 - (int)TIM2_CCR2);	// (OC - 1 PPS IC)
			if (diff < 0)	diff += 65536;	// Adjust for wrap-around
tim2debug0 = -diff; // 
			if (diff > 12000) diff = 12000;
			phasing_oneinterval = -( (diff << (TIM2SCALE - 6)) / TIM2_NUM_INTERVALS );	// Interval = whole.fraction

			if ((strAlltime.SYS.ull & 0x3f) != 63)
			{ // Here, the soon-to-follow OC should increment the 1/64th tick to 0
				strAlltime.SYS.ull |= 0x3f;	// 
				tim2_64th_19_er += 1;		// Count these for the hapless programmer
			}
		}
		else
		{ // Here, we are further out than one OC interval, so "jam" i.e re-initialize.
			TIM2_CCR4 = (TIM2_CCR4 + TIM2NOMINAL);	// Set a new "nominal" increment
			tim2_oc_ctr = 0;		// Interval counter for intervals within 1/64th sec
			phasing_oneinterval = 0;	//
			TIM2_SR = ~0x4;			// Get rid of OC if flag it had come on
			tmp = TIM2_CCR4;		// Dummy readback to assure new OC register has the new value

tim2debug1 += 1;
		}
	}

	return;
}

/*-----------------------------------------------------------------------------
 * void setnextoc(void);
 * @brief	: Set up next OC register as an increment from 'base'
 -----------------------------------------------------------------------------*/
void setnextoc(void)
{
	s32 	tmp;
	s32	tmp1;

	TIM2_SR = ~0x4;				// Reset OC interrupt flag
	tim2_oc_ctr += 1;			// Count OC intervals to get 1/64th sec
	if (tim2_oc_ctr >= TIM2_NUM_INTERVALS)
	{ // Here, the end of the last interval is the demarcation of the 1/64th sec tick
		/* ======== This is where the CAN msg is intiated ======== */
		strAlltime.SYS.ull += 1;	// Update ticks
//		can_sync_msg.cd.uc[0] = strAlltime.SYS.uc[0] & 63; // Tick count

		/* Call other routines if an address is set up */
		if (p1_rtc_secf_ptr != 0)	// Having no address for the following is bad.
			(*p1_rtc_secf_ptr)();	// Go do something (e.g. poll the AD7799)	
		
		/* ======== this is where the time stamp is sync'd to 1/64th sec tick zero ==== */
		// See if 'gps_poll.c' signals a new date/time is to be jammed
		if (gps_poll_flag != 0)
		{
			gps_poll_flag = 0;	// Reset flag
			strAlltime.SYS.ull = strAlltime.GPS.ull; // Load new time (lower 6 bits are zero)
		}
		tim2_oc_ctr = 0;
	}

	/* Goal: set the OC register with the next OC time */

	/* Determine the time increment for the next interval */

	// The following is signed 'whole.fraction' + 'whole.fraction'
	deviation_sum += deviation_oneinterval;	// Add scaled deviation to the running sum

	/* Get the (signed) whole of the 'whole.fraction' */
	tmp = (deviation_sum/(1 << TIM2SCALE));	// Tmp is 'whole'

	/* Adjust the sum to account for the amount applied to the interval */
	//   Remove the 'whole' from 'whole.fraction' (Remember: this is signed)
	deviation_sum -= (tmp  << TIM2SCALE);	// Remove 'whole' from sum

	/* Same process as above, but for phasing. */
	phasing_sum += phasing_oneinterval;
	tmp1 = phasing_sum/(1 << TIM2SCALE);
	phasing_sum -= (tmp1 << TIM2SCALE);

	/* Set up CH4 OC register for next interval duration */
	tim2_ccr4 = TIM2_CCR4;	// Save "previous" OC interval end time

	// Add an increment of ( "nominal" + duration adjustment + phasing adjustment)
	TIM2_CCR4 += (TIM2NOMINAL + tmp + tmp1);	// Set next interval end time
//	TIM2_CCR4 += (TIM2NOMINAL + tmp);	// Set next interval end time

db4 += tmp1;
db6 += (tmp + tmp1);

db5 += (tmp + tmp1);
db3 += 1;	// Count OC interrupts

	return;
}
/*#######################################################################################
 * ISR routine for TIM2
 *####################################################################################### */
void TIM2_IRQHandler(void)
{
	u16 usSR = TIM2_SR & 0x15;
	__attribute__((__unused__))volatile u32 temp;
	s32	diff;

	/* We don't expect this, but to comfirm such a bold expectation, we had better count them! */
	if (usSR == 0)		tim2_zeroflag_ctr += 1;

	/* Extended timing: Handle the 1 PPS input capture (CC2IF) and counter overflow (UIF) flags */
	switch (usSR & 0x5)
	{
	case 0x01:	// Overflow flag only
			TIM2_SR = ~0x1;				// Reset overflow flag
			strTim2cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the time counter
db2 += 1;	// Count OV interrupts
			break;
		
	case 0x04:	// Capture flag only
tim2cyncnt = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
tim2debug5 = db5;
tim2debug3 = db3;
tim2debug2 = db2;
tim2debug4 = db4;
tim2debug6 = db6;
		strTim2.us[0] = TIM2_CCR3;		// Read the captured count which resets the capture flag
		strTim2.us[1] = strTim2cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
		strTim2.ui[1] = strTim2cnt.ui[1];	// Extended time of upper 32 bits of long long
		strTim2m = strTim2;			// Update buffered value		
		uiTim2ch2_Flag += 1;			// Advance the flag counter to signal mailine IC occurred

		idx_cmd_n_ct += 1;	// Index for double buffering can msg counts

		/* Trigger a pending interrupt, to compute interval duration adjustment factor. */
		NVICISPR(NVIC_TIM6_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

		break;


	case 0x5:	// Both flags are on	
			// Set up the input capture with extended time
tim2cyncnt = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
tim2debug5 = db5;
tim2debug3 = db3;

db2 += 1;	// Count OV interrupts
tim2debug2 = db2;
tim2debug4 = db4;
tim2debug6 = db6;


		strTim2.us[0] = TIM2_CCR3;		// Read the captured count which resets the capture flag
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

		idx_cmd_n_ct += 1;	// Index for double buffering can msg counts

		/* Trigger a pending interrupt, to compute interval duration adjustment factor. */
		NVICISPR(NVIC_TIM6_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

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
/*#######################################################################################
 * ISR routine for TIM2 input capture low priority level
 *####################################################################################### */

unsigned int ticksadj;

void TIM6_IRQHandler(void)
{
/* This interrupt is caused by the GPS 1PPS input capture handling, used for further processing at a low interrupt priority */
	s32 ticks_temp;
	s32 tmp;
	struct TIMCAPTRET32 strIC;	// 1 PPS input capture time (32b), and flag counter

	// Here GPS time is good so we can begin measuring tim2 clock rate
//	strIC = Tim2_inputcapture_ui();			// Get input capture time and flag ctr
	strIC.ic  = strTim2m.ui[0];			// Get 32b input capture time
	strIC.flg = uiTim2ch2_Flag; 
	ticks_temp = (s32)(strIC.ic - Tim2tickctr_prev);// Ticks in one second
	ticks = ticks_temp;
	Tim2tickctr_prev = strIC.ic;			// Save for computing difference next time

	ticks_flg += 1;	// main will use this flag for pacing

	/* Check that we don't have some bogus value (e.g. a startup problem, etc.) */
	if ( ( (u32)ticks_temp > tickspersecLO) && ( (u32)ticks_temp < tickspersecHI) ) // Allow +/- 10%
	{ // Here, passing the check makes it "good enough" 
		/* Compute a "whole.fraction" for each interval */
		tmp = (ticks_temp - (2 * pclk1_freq));		// Deviation of ticks in one sec from perfection
ticks_dev = tmp;
		tmp *= (1 << (TIM2SCALE - 6));	// Scale upwards and divide by 64 (hence the -6);
		deviation_oneinterval = ( tmp / TIM2_NUM_INTERVALS );	// Interval = whole.fraction
ticksadj = deviation_oneinterval;
	}
	else
	{
		tim2_tickspersec_err += 1;
	}

	/* Logic to wait for GPS and time sync'ing to settled down before signalling  that logging can begin */
	/* 'ocphasing()' is where the 'er count is made */
	/* When 'tim2_readyforlogging == 0x3' then the time is stable */
	if (tim2_64th_0_er != tim2_64th_0_er_prev)
	{ // Here, the 1 PPS at interval 0 needed adjustment...not ready
		tim2_64th_0_er_prev = tim2_64th_0_er;
		tim2_64th_0_ctr = 0;		 
		tim2_readyforlogging &= ~0x1;
	}
	else
	{ // Here, no adjustment needed.  Count a "few" 
		if (tim2_64th_0_ctr >= 3)
			tim2_readyforlogging |= 0x1;
		else
			tim2_64th_0_ctr += 1;
	}

	/* Do the same as above, but for interval 19. */
	if (tim2_64th_19_er != tim2_64th_19_er_prev)
	{
		tim2_64th_19_er_prev = tim2_64th_19_er;
		tim2_64th_19_ctr = 0;		 
		tim2_readyforlogging &= ~0x2;
	}
	else
	{ // Here, no adjustment needed.  Count a "few" 
		if (tim2_64th_19_ctr >= 3)
			tim2_readyforlogging |= 0x2;
		else
			tim2_64th_19_ctr += 1;
	}

	/* Check that the adjustment factor is under control. */
	if ( (deviation_oneinterval > 500000) || (deviation_oneinterval < -500000) )
	{ // Here, the adjustment is way out we might get bogus no-change intervals 0 and 19.
		tim2_64th_0_ctr = 0; tim2_64th_19_ctr = 0; tim2_readyforlogging = 0;
	}

	return;
}
/*-----------------------------------------------------------------------------
 * static void tim2lowlevel_init(void);
 * @brief	: Setup for low level Tim2_pod_se.c
 * @return	: 
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

