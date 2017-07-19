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
#include "common_can.h"


struct ENCODERREADING enr_wrk[2];	// Latest, working, reading
struct ENCODERCOMPUTE enc_can[2];	// Compute rate from reading
struct ENCODERCOMPUTE enc_use[2];	// Compute rate from reading

volatile uint32_t dtw_oc;	// DTW time saved at OC

/* Output compare */
// Based on 42 MHz TIM3 clock rate
uint32_t interval_ctr;

static char init_sw = 0;

#define  INTERVAL_CT 30		// Number of OC intervals in 1/64th second
static uint32_t interval_ct;	// Counter of intervals
#define INTERVAL_TICKS	((42000000*2)/(64*INTERVAL_CT))// (43750) TIM3 ticks for one interval (nominal)
#define INTERVAL_DTW (186000000/(64*INTERVAL_CT))// (87500) DTW ticks for one interval (common_all/trunk/common_can.h:61:#define NVIC_CAN_RX1_IRQ_PRIORITY		0x30	//   Receive FIFO 1 (and related) [time sync]nominal)
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

// ##### NOTE: INTERRUPT PRIORITY OF CAN AND RX1 MUST BE THE SAME #####################
#define TIM3_PRIORITY NVIC_CAN_RX1_IRQ_PRIORITY //   Set TIM3 priority same as CAN RX1 (time msg)
//#define TIM3_PRIORITY 0x30	// Timer priority

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
 * void encoder_speed(struct ENCODERCOMPUTE *p);
 * @brief : compute rate and buffer stuff
 * @param : p = pointer to encoder data struct
*******************************************************************************/
void encoder_speed(struct ENCODERCOMPUTE *p)
{
	/* Change in time and count since last 1/64th sec. */
	p->dt = (p->enr.t.ull - p->enr_prev.t.ull); // Time difference
	p->dn = p->enr.n - p->enr_prev.n;	// Number of counts
	p->r = (float)(p->dn)/(float)(p->dt);	// encoder counts/per counter tick
	return;
}
/******************************************************************************
 * void encoder_get_reading(struct ENCODERREADING *p, uint16_t unit);
 * @brief	: Retrieve current encoder
 * @param	: p = pointer to encoder data struct to be updated/computed
 * @param	: unit = 0 TIM2 encoder; 1 = TIIM5 encoder
 * @return	: re-populated struct
*******************************************************************************/
void encoder_get_reading(struct ENCODERREADING *p, uint16_t unit)
{
	struct ENCODERREADING enr_tmp;

	/* Read twice to be sure copy didn't get hit by a new encoder count */
	do
	{
		*p = enr_wrk[unit];		// Latest encoder reading
		enr_tmp = enr_wrk[unit];	// Copy again
	} while ((enr_tmp.n != p->n) || (enr_tmp.t.ll != p->t.ll));
	return;
}
/******************************************************************************
 * void encoder_get_all(struct ENCODERCOMPUTE *p, uint16_t unit);
 * @brief	: Retrieve current encoder reading with computed speed
 * @param	: p = pointer to encoder data struct to be updated/computed
 * @param	: unit = 0 TIM2 encoder; 1 = TIIM5 encoder
 * @return	: re-populated struct
*******************************************************************************/
void encoder_get_all(struct ENCODERCOMPUTE *p, uint16_t unit)
{
	struct ENCODERREADING enr_tmp;

	encoder_get_reading(&enr_tmp,unit);// Get latest reading

	/* Skip updating 'prev if there are no counts */
	if (p->enr.n != enr_tmp.n)
	{ // Here, there was one or more encoder counts since last time
		p->enr_prev = p->enr;		// Update previous reading				
	}
	
	p->enr = enr_tmp;	// Copy new reading
	encoder_speed(p);	// Compute speed (rate)
	return;
}
/* ######################################################################################
   Entered via: can_driver->can_msg_reset->here with RX0, RX1 CAN msg
   ###################################################################################### */
void can_msg_check(void* pctl, struct CAN_POOLBLOCK* pblk)
{
// NOTE: CAN and TIM3 are at same interrupt priority

	/* Only deal with timing CAN msgs */
	if (pblk->can.id != canid_tim) return; // Return: Not a time sync msg

	/* Here--This msg arrives at 64/sec.  Save the measurement */
	encoder_get_all(&enc_can[0],0);
	encoder_get_all(&enc_can[1],1);

/* TODO construct msgs and add to CAN output buffer */

	return;	// Piece be with you.
}
/* ######################################################################################
   TIM3 Interrupt routines
   ###################################################################################### */
/* ===================================================================================== */
/* static void IC_only(struct ENCODERREADING *p, uint32_t ccr,  uint32_t enccr)
 * @brief  : Input capture flag only for CH3 or CH4
 * @param  : p = pointer to encoder data struct for the channel with interrrupt flag
 * @param  : ccr = register TIM3_CCRx (IC channel for TIM3)
 * @param  : enccr = TIMx_CCR1 (timer for encoder)
 * @return : t & n have the latest time and encoder count
======================================================================================== */
static void IC_only(struct ENCODERREADING *p, uint32_t ccr,  uint32_t enccr)
{
	/* Compute extended time for new input capture */
	p->t.us[0] = ccr;		// Read the captured count which resets the capture flag
	p->t.us[1] = p->t.us[1];	// Extended time of upper 16 bits of lower order 32 bits
	p->t.ui[1] = p->t.ui[1];	// Extended time of upper 32 bits of long long
	p->n = enccr;	// Save latest encoder from encoder timer
	return;
}
/* ===================================================================================== */
/* static void IC_ov(struct ENCODERREADING *p,  uint32_t ccr, uint32_t enccr)
 * @brief : Input capture flag and overflow flag for CH3 or CH4
 * @param : p = pointer to encoder data struct
 * @param : ccr = register TIM3_CCRx (IC channel for TIM3)
 * @param  : enccr = TIMx_CCR1 (timer for encoder)
 * @return : t & n have the latest time and encoder count
======================================================================================== */
static void IC_ov(struct ENCODERREADING *p,  uint32_t ccr, uint32_t enccr)
{
	/* Compute extended time for new input capture */
	p->t.us[0] = ccr;		// Read the captured count which resets the capture flag
	p->t.us[1] = p->t.us[1];	// Extended time of upper 16 bits of lower order 32 bits
	p->t.ui[1] = p->t.ui[1];	// Extended time of upper 32 bits of long long

	// Adjust input capture: Determine which flag came first.  If overflow came first increment the overflow count
	if (p->t.us[0] < 32768)		// Is the capture time in the lower half of the range?
	{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
		p->t.ull+= 0x10000;	// Increment the high order 48 bits 0f the *capture time*
	}

	p->n = enccr;	// Save latest encoder count from encoder timer
	return;
}
/* ===================================================================================== */
void TIM3_IRQHandler(void)
{
//	__attribute__((__unused__))volatile u32 temp;

	
	uint32_t temp = DTWTIME;	// Save DTW 32b sys counter (if time sync of OC implemented)
	uint16_t usSR;	// Status register save upon entry

	usSR = TIM3_SR & 0x1F;	// Get current flags
	
	/* Get extended time for input capture on CH4 & CH3 & overflow */
	switch (usSR & 0x19)	// Seven combinations for the three flags (null means not these flags)
	{	
	case 0x01:	// Overflow flag only
		TIM3_SR = ~0x1;				// Reset overflow flag
		enr_wrk[0].t.ull += 0x10000;		// Increment the high order 48 bits of the time counter		
		enr_wrk[1].t.ull += 0x10000;		// Increment the high order 48 bits of the time counter		
		break;
		
	case 0x08:	// CH3 Input Capture flag only
		IC_only(&enr_wrk[0], TIM3_CCR3,TIM2_CNT);// CH3
		break;

	case 0x10:	// CH4 Input Capture flag only
		IC_only(&enr_wrk[1], TIM3_CCR4,TIM5_CNT);// CH4
		break;

	case 0x18:	// CH4 and CH3 IC flags, no overflow flag
		IC_only(&enr_wrk[0],TIM3_CCR3,TIM2_CNT);	// CH3
		IC_only(&enr_wrk[1],TIM3_CCR4,TIM5_CNT);	// CH4
		break;

	case 0x9:  	// CH3 IC & Overflow flag are on	
		TIM3_SR = ~0x1;		// Reset overflow flag
		// Set up the input capture with extended time
		IC_ov(&enr_wrk[0],TIM3_CCR3,TIM2_CNT);	// CH3
		break;

	case 0x11:	// CH4 IC & Overflow flags are on	
		TIM3_SR = ~0x1;		// Reset overflow flag
		// Set up the input capture with extended time
		IC_ov(&enr_wrk[1],TIM3_CCR4,TIM5_CNT);	// CH4
		break;

	case 0x19:	// CH4, CH3, Overflow flag are on
		TIM3_SR = ~0x1;		// Reset overflow flag
		IC_ov(&enr_wrk[0],TIM3_CCR3,TIM2_CNT);	// CH3
		IC_ov(&enr_wrk[1],TIM3_CCR4,TIM5_CNT);	// CH4
	}


	/* OC timing TIM3_CH1 */
	if ((usSR & 0x02) != 0)
	{ // Here OC flag is on
		TIM3_SR = ~0x02;	// Reset flag
		TIM3_CCR1 += INTERVAL_TICKS;	// Compute next OC time call goes here

// #### NOTE: Currently no syncing of OC to CAN time msgs ####
		/* End of 1/64th sec tick */
		interval_ct += 1;
		if (interval_ct >= INTERVAL_CT) // End of 1/64th sec tick?
		{ // Here, end of 1/64th interval
			interval_ct = 0;	// Reset interval counter
			dtw_oc = temp;		// Save: might be used for time sync'ing
			encode_oc_flag += 1;	// Signal somebody that something happened
		}
	}
	return;
}

