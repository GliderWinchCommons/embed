/******************************************************************************
* File Name          : encoder_timers.c
* Date First Issued  : 07/01/2017
* Board              : Discovery F4
* Description        : TIM2,TIM5,TIM3--two encoders with speed timing
*******************************************************************************/
/* 
Encoder #1 --
  A -> TIM2 CH1, TIM3 CH3
  B -> TIM2 CH2

Encoder #2 --
  A -> TIM5 CH1, TIM3 CH4
  B -> TIM5 CH2

TIM3 --
 CH1 - Output capture: 1/64th sec synchronized ticks
 CH2 
 CH3 - Input capture #1A
 CH4 - Input capture #2A

These timers are connected to the APB1 bus with a max of 42 MHz.  
(Note, the F42x and F43X have RCC_DCKCNFG register with a bit
to run timers at 84 and 169 MHz, but not the F407.)

*/
#include "nvicdirect.h" 
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/timer.h"
#include "libopencm3/stm32/nvic.h"
#include "DTW_counter.h"

#include "DISCpinconfig.h"
#include "encoder_timers.h"
#include "can_driver.h"
#include "can_msg_reset.h"
#include "running_average.h"

struct ENCODERCOMPUTE
{
	union TIMCAPTURE64 t;	   // Time (latest)
	union TIMCAPTURE64 t_prev; // Time (previous)
	int32_t n;	// Count (latest)
	int32_t n_prev;	// Count (previous)
	int32_t dt;	// Diff = last-prev
	int32_t dn;	// Diff = last-prev
	float r;	// Rate = dn/dt
	uint16_t flag;
} en_compute[2];	

struct ENCODERDAT
{
	union TIMCAPTURE64 cnt;	
	struct	ENCODERCOMPUTE	enc;
	int16_t	flag;	//
}  en[2]; // encoders: CH3 [0] TIM2, CH4 [1] TIM5 */



volatile uint32_t dtw_oc;	// DTW time saved at OC

/* Output compare */
// Based on 42 MHz TIM3 clock rate
uint32_t interval_ctr;

static char init_sw = 0;

#define  INTERVAL_CT 30		// Number of OC intervals in 1/64th second
static uint32_t interval_ct;	// Counter of intervals
#define INTERVAL_TICKS	((42000000*2)/(64*INTERVAL_CT))// (43750) TIM3 ticks for one interval (nominal)
#define INTERVAL_DTW (186000000/(64*INTERVAL_CT))// (87500) DTW ticks for one interval (nominal)
#define DTW_64TH (168000000/64)
/* (dtw_can - dtw_can_prev) not in the following limits is considered bogus */
#define INTERVAL_DTW_HI (DTW_64TH + DTW_64TH/(INTERVAL_CT+1))
#define INTERVAL_DTW_LO (DTW_64TH - DTW_64TH/(INTERVAL_CT+1))

void can_msg_check(void* pctl, struct CAN_POOLBLOCK* pblk);

static uint32_t canid_tim;	// CAN id for time syncing output capture timing

int64_t dtw_phase;
uint64_t dtw_can_dur_ave; // SCALED by << DTWAVE_SCALE


#define DTWAVE_N	64	// Size of running average
#define DTWAVE_SCALE	8	// Number of bits to scale
static struct RUNNING_AVE dtw_ave; // Running average for CAN time msg durations

uint32_t en_interval_reset;	// Running count of interval jam resets

uint32_t encode_oc_flag;	// 1/64sec tick flag

/* **************************************************************************************
 * int encoder_timers_init(uint32_t canid);
 * @brief	: Initialize TIM2,5,3 for shaft encoder
 * @param	: canid = CAN id for time sync msgs
 * @return	: 0 = OK; not 0 = fail
 * ************************************************************************************** */
int encoder_timers_init(uint32_t canid)
{
	canid_tim = canid;	// Save CAN id for time msgs used for time syncing

	running_average_init(&dtw_ave, DTWAVE_N, DTWAVE_SCALE);
	dtw_can_dur_ave = DTW_64TH << DTWAVE_SCALE; // Nominal, scaled 64th sec

	if (init_sw != 0) return 0;	// Init just once
	init_sw = 1;

	/* can_driver finds it way here for checking time msgs */
	can_msg_reset_ptr = &can_msg_check;

	/* Enable bus clocking for TIM2,3,5, */
	RCC_APB1ENR |= 0x0B;		 

	/* Reload register for encoders (default) */
//	TIM2_ARR = ~0L;
//	TIM5_ARR = ~0L;

	/* Encoder interface mode. */
	TIM2_SMCR |= (0x1 << 0);	// Mode 1: Counter counts up/down on TI2FP1 edge
	TIM5_SMCR |= (0x1 << 0);	// Mode 1: Counter counts up/down on TI2FP1 edge

	/* Prescale divider (freq/(PSC+1)) */
	// Use default: 0 = divide by one

	/* CH4 & CH3 input capture */
	TIM3_CCMR2 = (0X01<<8) | (0X01<<0);

	/* CH1 CH2 output capture, frozen state */
	TIM3_CCR1 = 0;

	/* Set and enable interrupt controller for TIM3 interrupt (timing) */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM3
	
	/* Enable TIM3 interrupts */
#define INTERRUPTFLAGS	0X1B
	TIM3_DIER |= INTERRUPTFLAGS;	// Enable TIM3 flags

	/* Control register 1 */
	TIM2_CR1 |= TIM_CR1_CEN; 	// Counter enable: counter starts counting.
	TIM5_CR1 |= TIM_CR1_CEN; 	// Counter enable: counter starts counting.
	TIM3_CR1 |= TIM_CR1_CEN; 	// Counter enable: counter starts counting.

	return 0;
}
/******************************************************************************
 * void en_speed(struct ENCODERCOMPUTE *p)
 * @brief : compute rate and buffer stuff
 * @param : p = pointer to encoder data struct
*******************************************************************************/
void en_speed(struct ENCODERCOMPUTE *p)
{
	/* Change in time and count since last 1/64th sec. */
	p->dn = p->n - p->n_prev;
	p->dt = (int32_t)(p->t.ll - p->t_prev.ll);

	/* Rotational distance per unit of time */
	p->r = (float)(p->dn)/(float)(p->dt);

	/* Save latest count and time */
	p->n_prev = p->n;
	p->t_prev = p->t;

	p->flag += 1;	// Advance the flag counter to signal somebody(?)
	return;
}
/******************************************************************************
 * void encoder_get(struct ENCODERCOMPUTE p, uint16_t unit);
 * @brief	: Retrieve current encoder data (lock interrupts briefly)
 * @param	: p = pointer to encoder data struct to be updated/computed
 * @param	: unit = 0 TIM2 encoder; 1 = TIIM5 encoder
 * @return	: re-populated struct
*******************************************************************************/
void encoder_get(struct ENCODERCOMPUTE *p, uint16_t unit)
{
	__attribute__((__unused__))int	tmp;

	if (unit > 1) unit = 1;

	TIM3_DIER &= ~INTERRUPTFLAGS;	// Disable
	tmp = TIM3_DIER;		// Readback ensures that interrupts have locked

	/* Copy data of interest */
	// Caller is tasked with saving previous 'n' and 't'
	p->n = en[unit].enc.n;		// Latest encoder count
	p->t = en[unit].enc.t;		// Latest time of encounter count

	TIM3_DIER |= INTERRUPTFLAGS;	// Enable interrupts
	return;
}

/* ######################################################################################
   Entered via: can_driver->can_msg_reset->here with RX0, RX1 CAN msg
   ###################################################################################### */
struct ENCODERCOMPUTE en_can[2];

void can_msg_check(void* pctl, struct CAN_POOLBLOCK* pblk)
{
	/* Only deal with timing CAN msgs */
	if (pblk->can.id != canid_tim) return;

	/* Get latest data */
	en_can[0].n_prev = en_can[0].n; // Update previous count
	en_can[0].t_prev = en_can[0].t; // Update previous time
	encoder_get(&en_can[0],0);	// Copy count and time

	en_can[1].n_prev = en_can[1].n;
	en_can[1].t_prev = en_can[1].t;
	encoder_get(&en_can[1],1);

/* TODO construct msg and add to CAN output buffer */

	return;	// Piece be with you.
}
/* ######################################################################################
   TIM3 Interrupttim3_ic
   ###################################################################################### */
/* ===================================================================================== */
/* static void IC_only(struct ENCODERDAT *p, uint32_t ccr,  uint32_t enccr)
 * @brief  : Input capture flag only for CH3 or CH4
 * @param  : p = pointer to encoder data struct for the channel with interrrupt flag
 * @param  : ccr = register TIM3_CCRx (IC channel for TIM3)
 * @param  : enccr = TIMx_CCR1 (timer for encoder)
 * @return : t & n have the latest time and encoder count
======================================================================================== */
static void IC_only(struct ENCODERDAT *p, uint32_t ccr,  uint32_t enccr)
{
	/* Compute extended time for new input capture */
	p->enc.t.us[0] = ccr;		// Read the captured count which resets the capture flagDTWAVE_SCALE
	p->enc.t.us[1] = p->cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
	p->enc.t.ui[1] = p->cnt.ui[1];	// Extended time of upper 32 bits of long long

	p->enc.n = enccr;	// Save latest encoder count: read IC register from encoder timer
	return;
}
/* ===================================================================================== */
/* static void IC_ov(struct ENCODERDAT *p,  uint32_t ccr, uint32_t enccr)
 * @brief : Input capture flag and overflow flag for CH3 or CH4
 * @param : p = pointer to encoder data struct
 * @param : ccr = register TIM3_CCRx (IC channel for TIM3)
 * @param  : enccr = TIMx_CCR1 (timer for encoder)
 * @return : t & n have the latest time and encoder count
======================================================================================== */
static void IC_ov(struct ENCODERDAT *p,  uint32_t ccr, uint32_t enccr)
{
	/* Compute extended time for new input capture */
	p->enc.t.us[0] = ccr;		// Read the captured count which resets the capture flag
	p->enc.t.us[1] = p->cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
	p->enc.t.ui[1] = p->cnt.ui[1];	// Extended time of upper 32 bits of long long

	// Take care of overflow flag
	p->cnt.ll += 0x10000;	// Increment the high order 48 bits of the *time counter*

	// Adjust input capture: Determine which flag came first.  If overflow came first increment the overflow count
	if (p->enc.t.us[0] < 32768)		// Is the capture time in the lower half of the range?
	{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
		p->enc.t.ll	+= 0x10000;	// Increment the high order 48 bits if the *capture counter*
	}

	p->enc.n = enccr;	// Save latest encoder count: read IC register from encoder timer
	return;
}
/* ===================================================================================== */
/* static void get_en(struct ENCODERCOMPUTE *pm, struct ENCODERCOMPUTE *pw)
 * @brief  : Get the latest reading for the measurement interval
 * @param  : pw = pointer to encoder working interrupt data struct
 * @param  : pm = pointer to encoder readings at measurement time
 * @return : 0 = success; -1 = change
======================================================================================== */
static int32_t get_en(struct ENCODERCOMPUTE *pm, struct ENCODERCOMPUTE *pw)
{
	*pm = *pw;
// TODO read TIM2 or TIM5 and check if count changed. 
// If changed, then set a flag and exit the ISR and the interrupt that resulted from 
// the change in TIM2 or TIM5 will cause the ISR to re-enter and get the new time.
// The flag will skip the end-of-interval counting
	return 0;

}
/* ===================================================================================== */
int32_t isr_flag = 0;

void TIM3_IRQHandler(void)
{
//	__attribute__((__unused__))volatile u32 temp;

	
	uint32_t temp = DTWTIME;	// Save DTW 32b sys counter
	int32_t ret;

	uint16_t usSR = TIM3_SR & 0x1F;

	/* Get extended time for input capture on CH4 & CH3 & overflow */
	switch (usSR & 0x19)	// Seven combinations for the three flags (null means not these flags)
	{	
	case 0x01:	// Overflow flag only
		TIM3_SR = ~0x1;				// Reset overflow flag
		en[0].cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the time counter		
		en[1].cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the time counter		
		break;
		
	case 0x08:	// CH3 Input Capture flag only
		IC_only(&en[0], TIM3_CCR3,TIM2_CCR1);	// CH3
		break;

	case 0x10:	// CH4 Input Capture flag only
		IC_only(&en[1], TIM3_CCR4,TIM5_CCR1);	// CH4
		break;

	case 0x18:	// CH4 and CH3 IC flags, no overflow flag
		IC_only(&en[0],TIM3_CCR3,TIM2_CCR1);	// CH3
		IC_only(&en[1],TIM3_CCR4,TIM5_CCR1);	// CH4
		break;

	case 0x9:  	// CH3 IC & Overflow flag are on	
		TIM3_SR = ~0x1;		// Reset overflow flag
		// Set up the input capture with extended time
		IC_ov(&en[0],TIM3_CCR3,TIM2_CCR1);	// CH3
		break;

	case 0x11:	// CH4 IC & Overflow flags are on	
		TIM3_SR = ~0x1;		// Reset overflow flag
		// Set up the input capture with extended time
		IC_ov(&en[1],TIM3_CCR4,TIM5_CCR1);	// CH4
		break;

	case 0x19:	// CH4, CH3, Overflow flag are on
		TIM3_SR = ~0x1;		// Reset overflow flag
		IC_ov(&en[0],TIM3_CCR3,TIM2_CCR1);	// CH3
		IC_ov(&en[1],TIM3_CCR4,TIM5_CCR1);	// CH4
	}
    if (isr_flag == 0)
    {
	/* OC timing TIM3_CH1 */
	if ((usSR & 0x02) != 0)
	{ // Here OC flag is on
		TIM3_SR = ~0x02;	// Reset flag
		TIM3_CCR1 += INTERVAL_TICKS;	// Compute next OC time call goes here

		/* End of 1/64th sec tick */
		interval_ct += 1;
		if (interval_ct >= INTERVAL_CT) // End of 1/64th sec tick?
		{ // Here, end of 1/64th interval
			interval_ct = 0;	// Reset interval counter
			dtw_oc = temp;		// Save for maybe I'll use it
			encode_oc_flag += 1;	// Signal main

			/* Get latest readings */
			ret =  get_en(&en[0].enc, &en_compute[0]);
			ret |= get_en(&en[1].enc, &en_compute[1]);
			if (ret != 0)
			{
				isr_flag = 1;
				return;
			}
		}
    }
    else
    { // Here, this was a re-entry due to a measurement read change
	isr_flag = 0;
			/* Get latest readings */
			ret =  get_en(&en[0].enc, &en_compute[0]);
			ret |= get_en(&en[1].enc, &en_compute[1]);
			if (ret != 0)
			{
				isr_flag = 1;
				return;
			}

    }

	}
 	return;
}

