/******************************************************************************
* File Name          : rpmsensor.c
* Date First Issued  : 07/04/2013
* Board              : RxT6
* Description        : RPM sensing
*******************************************************************************/
/* 
Routine to measure rpm on RxT6 board with spark detection hardware fitted.

PB9 - TIM4_CH4, TIM11_CH1, SDIO5_D5, I2C1_SDA, CAN_TX: input from inverter.


*/
#include <math.h>
#include "libopenstm32/timer.h"
#include "libusartstm32/nvicdirect.h" 
#include "libusartstm32/commonbitband.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "common.h"
//#include "../../../../svn_common/trunk/common_can.h"
#include "IRQ_priority_se1.h"
#include "pinconfig_all.h"
#include "adcsensor_eng.h"
#include "canwinch_pod_common_systick2048.h"
#include "rpmsensor.h"
#include "libmiscstm32/DTW_counter.h"
#include "engine_function.h"

/* Static routines buried in this mess. */
static void Tim4_eng_init(void);
static void rpmsensor_computerpm(struct ENG_RPM_FUNCTION* p);

/* Low level trigger for doing computation */
void 	(*tim4ocLOpriority_ptr)(void) = 0;	// CH3 OC -> EXTI0 (low priority)
void (*tim4_tim_oc_ptr)(void);	// Low level interrupt trigger function callback	

/* 
TIM4 is on APB1 @ 32 MHz pclk1_freq
One 1/64th sec interval = 1,000,000 ticks
16 sub-intervals -> 1000000/16 = 62500
Duration of one subinterval: 62500/64 = 976.563 us
*/
#define SUBINTERVAL 62500	
uint32_t subinterval_inc;	// Increment (62500 nominal)

#define NUMSUBINTERVALS 16
uint32_t subinterval_ct;	// Current subinterval count

/* 
   Allow about 2 ms for computation of readings before
expected CAN poll msg, i.e. two subintervals.

   CAN time sync msg resets the subinterval counter and output
capture count. The following is the sub-interval count
that computation is triggered.

  Shameless hack to avoid the issue of CAN time sync msg coincides with
the TIM4 subinterval interrupt is to offset the subinterval time by
less than the time duration of a CAN msg.  Should there be a coincidence
it is not a big deal.  The alternative is the rather intricate phasing
of the timer ticks to an average time of arrival of the sync CAN msg.  For
the engine measurements this is not necessary.
*/
#define SUBINTERVALOFFSET (SUBINTERVAL/2)	// Offset to avoid interrupt conflicts

/* Pointers to functions to be executed under a low priority interrupt */
// The following is loaded with the address of the function that will be called
void 	(*rpmsensor_ptr)(void) = NULL;

/* Various bit lengths of the timer counters are handled with a union */
// Timer counter extended counts
volatile union TIMCAPTURE64	strTim4cnt;	// 64 bit extended TIM4 CH4 timer count

// Input capture extended counts
volatile union TIMCAPTURE64	strTim4;  // 64 bit extended TIM4 CH4 capture
volatile union TIMCAPTURE64	strTim4m; // 64 bit extended TIM4 CH4 capture (main)

/* The readings and flag counters are updated upon each capture interrupt */
volatile u32 usTim4ch4_Flag; // Incremented when a new capture interrupt serviced, TIM4_CH4

/* Timing counter */
uint32_t tim4_tim_ticks;  // Running count of time ticks
uint32_t tim4_tim_rate;   // Number of ticks per sec (64E6/16E3)
#define TIM4TICKINC 32000 // Timer ct for 0.5 ms between interrupts

/******************************************************************************
 * void rpmsensor_init(void);
 * @brief 	: Initialize TIM4_CH4 and routines to measure rpm
*******************************************************************************/
void rpmsensor_init(void)
{
	double dclk1_freq = pclk1_freq;
	
	/* factor to yield rpm */
	erpm_f.dk1 = (2 * 60 * dclk1_freq)/erpm_f.lc.seg_ct;

	/* Input capture and 64/sec timing */
	Tim4_eng_init();	// Initialize TIM4_CH4

	return;
}
/******************************************************************************
 * static void Tim4_eng_init(void);
 * @brief	: Initialize Tim4 for input capture
*******************************************************************************/
const struct PINCONFIGALL pin_pb9 = {(volatile u32 *)GPIOB, 9, IN_FLT, 0};


static void Tim4_eng_init(void)
{
	/* Note: variables are used for SUBINTERVAL and TIM4TICKINC should
      later some timer phasing be implemented. */

	/* Set subinterval count increment to initial nominal value */
	subinterval_inc = SUBINTERVAL;
	subinterval_ct = 0;	// Subinterval counter

	/* Rate for CH2, for polling and timing */
	// 2000 interrupts per sec
	tim4_tim_rate = pclk1_freq/TIM4TICKINC;

	/* Setup the gpio pin for PB9 is not really needed as reset default is "input" "floating" */
	pinconfig_all( (struct PINCONFIGALL *)&pin_pb9);	// p 156

	/* ----------- TIM4 CH4  ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM4 (p 335 for beginning of section on TIM4-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;		// (p 112) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 110) 

	/* TIMx capture/compare mode register 1  (p 400) */
	TIM4_CCMR2 |= TIM_CCMR2_CC4S_IN_TI4;		// (p 354)Fig 100 CC4 channel is configured as input, IC4 is mapped on TI4

	/* Compare/Capture Enable Reg (p 324,5) */
	//  Configured as input: falling edge trigger
	TIM4_CCER |= TIM_CCER_CC4E; 	// (0x3<<12);	// Capture Enabled (p 401) 

	/* Control register 2 */
	// Default: The TIMx_CH2 pin is connected to TI1 input (p 372)

	/* Control register 1 */
	TIM4_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 371,2)

	/* Enable a lower priority interrupt for handling post-systick timing. */
	NVICIPR (NVIC_EXTI0_IRQ, NVIC_EXTI0_IRQ_PRIORITY_SE1);	// Set interrupt priority
	NVICISER(NVIC_EXTI0_IRQ);			// Enable interrupt controller 

	/* Set and enable interrupt controller for TIM4 interrupt */
	NVICIPR (NVIC_TIM4_IRQ, TIM4_PRIORITY_SE1 );	// Set interrupt priority
	NVICISER(NVIC_TIM4_IRQ);			// Enable interrupt controller for TIM4
	
	/* Enable input capture interrupts */
   // Enable CH2 and CH3 output compare, CH4 input capture, and counter overflow
	TIM4_DIER |= (TIM_DIER_CC2IE) | (TIM_DIER_CC3IE | TIM_DIER_CC4IE | TIM_DIER_UIE);	

	return;
}
/******************************************************************************
 * unsigned long long Tim4_gettime_ll(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned long long Tim4_gettime_ll(void)
{
	union TIMCAPTURE64 strX;
	strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)
	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX = strTim4cnt;			// Get current extended count
		strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( ( strX.ll & ~0xffffLL)  !=   (strTim4cnt.ll & ~0xffffLL) );	// Check if count changed on us
	return strX.ll;	
}
/******************************************************************************
 * unsigned int Tim4_gettime_ui(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim4_gettime_ui(void)
{
	union TIMCAPTURE64 strX;			

	strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)

	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX.ui[0] = strTim4cnt.ui[0];	// Get low order word of current extended count
		strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( strX.us[1] != strTim4cnt.us[1] );// Check if extended count changed on us
	return strX.ui[0];			// Return lower 32 bits
}
/******************************************************************************
 * struct TIMCAPTRET32 Tim4_inputcapture_ui(void);
 * @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim4_inputcapture_ui(void)
{
	 __attribute__((__unused__))int tmp; // Dummy for readback of hardware registers
	struct TIMCAPTRET32 strY; // 32b input capture time and flag counter

	TIM4_DIER &= ~(TIM_DIER_CC4IE | TIM_DIER_UIE);	// Disable CH4 capture interrupt and counter overflow (p 315)
	tmp = TIM4_DIER; // Readback ensures that interrupts have locked

/* The following is an alternative to the two instructions above for assuring that the interrupt enable bits
   have been cleared.  The following results in exactly the same number of instructions and bytes as the above.
   The only difference is the last compiled instruction is 'str' rather than 'ldr'. */
//	tmp = TIM4_DIER;
//	tmp &= ~(TIM_DIER_CC4IE | TIM_DIER_UIE);
//	TIM4_DIER = tmp;
//	TIM4_DIER = tmp;

	strY.ic  = strTim4m.ui[0];  // Get 32b input capture time
	strY.flg = usTim4ch4_Flag;  // Get flag counter
	TIM4_DIER |= (TIM_DIER_CC4IE | TIM_DIER_UIE); // Enable CH4 capture interrupt and counter overflow (p 315)
	
	return strY;
}
/*#######################################################################################
 * ISR routine for TIM4
 *####################################################################################### */
uint32_t debugrpmsensor1 ;
uint32_t debugrpmsensorch3ctr ;

void TIM4_IRQHandler(void)
{
	 __attribute__((__unused__))int temp;	// Dummy for readback of hardware registers

	unsigned short usSR = TIM4_SR & 0x19;	// Get capture & overflow flags

	switch (usSR & 0x11)	// There are three cases where we do something.  The "00" case is bogus.
	{
	// p 394

	case 0x01:	// Overflow flag only
			TIM4_SR = ~0x1;				// Reset overflow flag
			strTim4cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter
			temp = TIM4_SR;				// Readback register bit to be sure is cleared
			break;

	case 0x00:	// Case where ic flag got turned off by overlow interrupt reset coinciding with ic signal

	case 0x10:	// Capture flag only
			strTim4.us[0] = TIM4_CCR4;		// Read the captured count which resets the capture flag
			strTim4.us[1] = strTim4cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim4.ui[1] = strTim4cnt.ui[1];	// Extended time of upper 32 bits of long long
			usTim4ch4_Flag += 1;			// Advance the flag counter to signal mailine IC occurred
			strTim4m = strTim4;			// Update buffered value		

			erpm_f.ct += 1;	// Running count of input captures
			erpm_f.endtime = strTim4m.ui[0];	// Get 32b input capture time

			temp = TIM4_SR;				// Readback register bit to be sure is cleared
			break;

	case 0x11:	// Both flags are on	

			// Take care of overflow flag
			TIM4_SR = ~0x1;				// Reset overflow flag

			// Set up the input capture with extended time
			strTim4.us[0] = TIM4_CCR4;		// Read the captured count which resets the capture flag
			strTim4.us[1] = strTim4cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim4.ui[1] = strTim4cnt.ui[1];	// Extended time of upper 32 bits of long long
			// Adjust inpute capture: Determine which flag came first.  If overflow came first increment the overflow count
			if (strTim4.us[0] < 0x8000)		// Is the capture time in the lower half of the range?
			{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
				// First copy the extended time count upper 48 bits (the lower 16 bits have already been stored)
				strTim4.ll	+= 0x10000;	// Increment the high order 48 bits
			}

			usTim4ch4_Flag += 1;			// Advance the flag counter to signal mailine IC occurred		
			strTim4m = strTim4;			// Update buffered value		

			strTim4cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter

			erpm_f.ct += 1; // Running count of input captures
			erpm_f.endtime = strTim4m.ui[0]; // Save 32b input capture time

			temp = TIM4_SR; // Readback register bit to be sure is cleared
			break;
	}

	/* OC flag for polling & timing trigger. */
	if ((TIM4_SR & 0x04) != 0)
	{
		TIM4_SR = ~0x04;	// Reset CH2 flag
		tim4_tim_ticks += 1;		// Timing tick running count
		TIM4_CCR2 += TIM4TICKINC;	// Next interrupt time
		if (tim4_tim_oc_ptr != 0)	// Skip? Having no address for the following is bad.
			(*tim4_tim_oc_ptr)();	   // Go do something for somebody
	}

	/* OC flag for sub_interval timing */
	if ((TIM4_SR & 0x08) != 0)
	{
			TIM4_SR = ~0x08;	// Reset CH3 output capture flag
			TIM4_CCR3  += subinterval_inc;	// Next sub-interval interrupt time
			subinterval_ct += 1;	// Count sub-intervals
			if (subinterval_ct >= NUMSUBINTERVALS) // End of sub-interval?
			{ // Here, yes.
				subinterval_ct = 0;
			}
			NVICISPR(NVIC_EXTI0_IRQ);	// Set pending (low priority) interrupt 
	}
	return;
}
/* ########################## UNDER HIGH PRIORITY CAN INTERRUPT ############################### 
 * void rpmsensor_reset_timer(void);
 * Can time sync msg: reset subinterval and OC 
 * ############################################################################################### */
void rpmsensor_reset_timer(void)
{
	subinterval_ct = 0;
	TIM4_CCR3 = TIM4_CNT + subinterval_inc - SUBINTERVALOFFSET;
	return;
}

/* ########################## UNDER LOW PRIORITY from TIM4 INTERRUPT ############################### 
 * Compute the rpm
 * ############################################################################################### */
uint32_t rpmsensor_dbug1;	// Check timing of fp operations
uint32_t rpmsensor_dbug2;

/* Enter this is from low priority interrupt triggered by TIM4, about 2 ms before 
   next expected poll msg.  These are nominally 64 per sec and sync'ed to the poll
   msg so that there computations are ready slightly before the poll msg. */
static void rpmsensor_computerpm(struct ENG_RPM_FUNCTION* p)
{
	int32_t inic;
	int32_t itim;
	double dnic;
	double dtim;

rpmsensor_dbug1 = DTWTIME;
	// Number of input captures since last computation
		inic = (p->ct - p->ct_prev);
		p->ct_prev = p->ct;
		dnic = inic;	// Convert to double

	// Time duration between previous and latest ic
		itim = (p->endtime - p->endtime_prev);
		p->endtime_prev = p->endtime;
		dtim = itim;

	// (input capture counts) / (time duration) scaled to rpm
		p->drpm = (dnic * p->dk1)/dtim;
		p->frpm = p->drpm;	// Convert to float for CAN msg)
rpmsensor_dbug2 = DTWTIME;
	
	return;
}
/*#######################################################################################
 * ISR routine for output caputure low priority level
 * TIME4 CH3 (above) at high priority triggers this low priority interrupt
 *####################################################################################### */
void EXTI0_IRQHANDLER(void)
{
	if (subinterval_ct >= SUBINTERVALTRIGGER) // Time to trigger (lower level) computation?
	{ // Here, yes
		rpmsensor_computerpm(&erpm_f);	// Do the rpm computation
	}

debugrpmsensorch3ctr += 1;

	/* Filter and prepare readings for ADC readings (manifold, throttle, thermistor) */
	// Do this each subinterval "tick"
	adcsensor_reading(subinterval_ct);
	
	// Set this to filter ADC readings
	if (tim4ocLOpriority_ptr != 0)	// Skip? Having no address for the following is bad.
		(*tim4ocLOpriority_ptr)();	   // Go do something for somebody

	return;
}

