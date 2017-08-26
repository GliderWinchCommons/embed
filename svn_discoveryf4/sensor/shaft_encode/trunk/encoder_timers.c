/******************************************************************************
* File Name          : encoder_timers.c
* Date First Issued  : 07/01/2017
* Board              : Discovery F4
* Description        : TIM2,TIM5,TIM3--two encoders with speed timing
*******************************************************************************/
/* 
Encoder phase, encoder TIM CHk (pin), timer TIM CH, (pin), wire color, (LED color)
Encoder #1 --
  A -> TIM2 CH1 (PA15)->TIM3 CH3 (PC8) green wire (GRN)
  B -> TIM2 CH2 (PB3) (BLU)

Encoder #2 --
  A -> TIM5 CH1 (PA0)->TIM3 CH4 (PC9) green wire (ORG)
  B -> TIM5 CH2 (PA1) (RED)

TIM3 --
 CH1 - Output capture: 1/64th sec synchronized ticks
 CH2 - Output capture: 1/2 ms polling ticks
 CH3 - Input capture #1A
 CH4 - Input capture #2A

These timers are connected to the APB1 bus with a max of 42 MHz.  
(Note, the F42x and F43X have RCC_DCKCNFG register with a bit
to run timers at 84 and 169 MHz, but not the F407.)

Wiring--
        Encoder   RJ14
Ground 	- black	- black
+5 	- red	- red
 A 	- green	- green
 B 	- white	- yellow

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
#include "ledf4.h"


static volatile struct ENCODERREADING enr_wrk[2];	// Latest, working, reading
static struct ENCODERCOMPUTE enc_can[2];	// Compute rate from reading

static union TIMCAPTURE64 ovcnt; // TIM3 overflow count for extending time to 64b

/* Error monitoring: Count rare occurences of 'get_reading' loop hit by interrupt */
unsigned long encoder_get_reading_loop_cnt = 0;

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

#define OCTICKSFOR1MS	42000	// OC counter ticks for 1/2 ms polling

void can_msg_check(void* pctl, struct CAN_POOLBLOCK* pblk);

static uint32_t canid_tim;	// CAN id for time syncing output capture timing

int64_t dtw_phase;
uint64_t dtw_can_dur_ave; // SCALED by << DTWAVE_SCALE


#define DTWAVE_N	64	// Size of running average
#define DTWAVE_SCALE	8	// Number of bits to scale
//static struct RUNNING_AVE dtw_ave; // Running average for CAN time msg durations

uint32_t en_interval_reset;	// Running count of interval jam resets

uint32_t encode_oc_ticks;	// 1/64sec tick flag
uint32_t encoder_timers_poll_ctr;	// Tick count

/* **************************************************************************************
 * static void led(uint32_t lednum);
 * @brief	: LED follows Encoder phase signal
 * @param	: lednum = LED number (e.g. GRN, ORG, RED, BLU)
 * ************************************************************************************** */
static char ledpin[4] = {15,0,3,1}; // GRN, ORG, RED, BLU
static void led(uint32_t lednum)
{
	/* Get encoder pin number associated with led color */
	char enc_pin = ledpin[lednum - 12];


	/* Get port for encoder pin */
	u32 p = GPIOA;	// All but ORG corresponds to GPIOA pins
	if (lednum == 14)
	{ // RED led corresponds to a PB pin (PB3)
		p = GPIOB;
	}

	/* Set LED according to encoder pin */	
	if ((GPIO_IDR(p) & (1<<enc_pin)) == 0)
	{ // Here, encoder pin is low
		GPIO_BSRR(GPIOD)=(1<<lednum); // Set LED ON
	}
	else
	{ // Here, encoder pin is high
		GPIO_BSRR(GPIOD)=(1<<(lednum+16)); // Set LED OFF
	}
	return;
}
/* **************************************************************************************
 * void encoder_leds(void);
 * @brief	: All four LEDs follow the two Encoder phase signals
 * ************************************************************************************** */
/*  Call this from 'main' so it doesn't burden the interrupt and measurement code */
void encoder_leds(void)
{
	led(GRN);
	led(ORG);
	led(RED);
	led(BLU);
	return;
}
/******************************************************************************
 struct's for configuring timer pins
*******************************************************************************/
// Note: Use the external resistors to +5v for pullup rather than internal (to +3.3v)
// TIM2 encoder #1
static const struct PINCONFIG	inputpup1 = { \
	GPIO_MODE_AF,	// mode: Input alternate function 
	0, 			// output type: not applicable 		
	0, 			// speed: not applicable
	GPIO_PUPD_NONE, 	// pull up/down: none
	GPIO_AF1 };		// AFRLy & AFRHy selection
// TIM5 encoder #2
static const struct PINCONFIG	inputpup2 = { \
	GPIO_MODE_AF,	// mode: Input alternate function 
	0, 			// output type: not applicable 		
	0, 			// speed: not applicable
	GPIO_PUPD_NONE, 	// pull up/down: none
	GPIO_AF2 };		// AFRLy & AFRHy selection

// TIM3 timer
static const struct PINCONFIG	inputpup3 = { \
	GPIO_MODE_AF,	// mode: Input alternate function 
	0, 			// output type: not applicable 		
	0, 			// speed: not applicable
	GPIO_PUPD_NONE, 	// pull up/down: none
	GPIO_AF2 };		// AFRLy & AFRHy selection

static const struct PINCONFIG	inputpup33 = { \
	GPIO_MODE_AF,	// mode: Input alternate function 
	0, 			// output type: not applicable 		
	0, 			// speed: not applicable
	GPIO_PUPD_NONE, 	// pull up/down: none
	GPIO_AF2 };		// AFRLy & AFRHy selection

/* **************************************************************************************
 * int encoder_timers_init(uint32_t canid);
 * @brief	: Initialize TIM2,5,3 for shaft encoder
 * @param	: canid = CAN id for time sync msgs
 * @return	: 0 = OK; not 0 = fail
 * ************************************************************************************** */
int encoder_timers_init(uint32_t canid)
{
	canid_tim = canid;	// Save CAN id for time msgs used for time syncinghttps://rhondastephens.wordpress.com/2016/04/01/parenting-are-we-getting-a-raw-deal/

//	running_average_init(&dtw_ave, DTWAVE_N, DTWAVE_SCALE);
//	dtw_can_dur_ave = DTW_64TH << DTWAVE_SCALE; // Nominal, scaled 64th sec

	if (init_sw != 0) return 0;	// Init just once
	init_sw = 1;

	/* can_driver finds it way here for checking time msgs */
	can_msg_reset_ptr = &can_msg_check;

	/* Enable bus clocking for TIM2,3,5, */
	RCC_APB1ENR |= 0x0B;	

	/*  Setup pins: alternate function pullup input: CH1, CH2 for TIM5, TIM2, and TIM3 IC3 IC4 */
	// TIM2 CH1 wired to TIM3 CH3 PA15->PC8 (Encoder 1 wire: GRN) (PB3: YEL)
	// TIM5 CH1 wired to TIM3 CH4 PA0 ->PC9 (Encoder 2 wire: GRN) (PB3: YEL)
	f4gpiopins_Config ((volatile u32*)GPIOA,15, (struct PINCONFIG*)&inputpup1); // TIM2 CH1	 
	f4gpiopins_Config ((volatile u32*)GPIOB, 3, (struct PINCONFIG*)&inputpup1); // TIM2 CH2
	f4gpiopins_Config ((volatile u32*)GPIOA, 0, (struct PINCONFIG*)&inputpup2); // TIM5 CH1
	f4gpiopins_Config ((volatile u32*)GPIOA, 1, (struct PINCONFIG*)&inputpup2); // TIM5 CH2
	f4gpiopins_Config ((volatile u32*)GPIOC, 8, (struct PINCONFIG*)&inputpup33); // TIM3 CH3
	f4gpiopins_Config ((volatile u32*)GPIOC, 9, (struct PINCONFIG*)&inputpup3); // TIM3 CH4

	/* Reload register for encoders (default) */
//	TIM2_ARR = ~0L;
//	TIM5_ARR = ~0L;

	/* Encoder interface mode. */
        // 001: Encoder mode 1 - Counter counts up/down on TI2FP1 edge depending on TI1FP2 level
	TIM2_SMCR |= (0x1 << 0); 
	TIM5_SMCR |= (0x1 << 0);

	/* TIM2 encoder mode: map pins */
	// CC2 channel is configured as input, IC2 is mapped on TI2
	// CC1 channel is configured as input, IC1 is mapped on TI1
	TIM2_CCMR1 = (0X01<<8) | (0X01<<0);

	/* TIM5 encoder mode: map pins */
	// CC2 channel is configured as input, IC2 is mapped on TI2
	// CC1 channel is configured as input, IC1 is mapped on TI1
	TIM5_CCMR1 = (0X01<<8) | (0X01<<0);

	/* Prescale divider (freq/(PSC+1)) */
	// Use default: 0 = divide by one

	/* TIM3 CH4 & CH3 input capture. */
	// CC4 channel is configured as input, IC4 is mapped on TI4
	// CC3 channel is configured as input, IC3 is mapped on TI3
	TIM3_CCMR2  = (0X01<< 8) | (0X01<<0);
	TIM3_CCMR2 |= (0x03<<12) | (0x03<<4); // Filter input
	

	/* Enable capture on TIM3 CH4, CH3. */
//	TIM3_CCER = (0X0B<<12) | (0X0B<<8); // Both edges
	TIM3_CCER = (0X03<<12) | (0X03<<8); // Falling edge
//	TIM3_CCER = (0X01<<12) | (0X01<<8); // Rising edge

	/* CH1 CH2 output capture, frozen state */
	TIM3_CCR1 = 0;

	/* Set and enable interrupt controller for TIM3 interrupt (timing) */

//#define TIM3_PRIORITY NVIC_CAN_RX1_IRQ_PRIORITY //   Set TIM3 priority same as CAN RX1 (time msg)
#define TIM3_PRIORITY 0x40	// Timer priority

	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM3
	
	/* Enable TIM3 interrupts */
//#define INTERRUPTFLAGS	0X1B	// Flags for CH4,3,1,and OV
#define INTERRUPTFLAGS	0X1F	// Flags for CH4,3,2,1,and OV
	TIM3_DIER = INTERRUPTFLAGS;	// Enable TIM3 flags

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
#include "lltoflt.h"
void encoder_speed(struct ENCODERCOMPUTE *p)
{
	

	/* Change in time and count since last 1/64th sec. */
	p->dt = (p->enr.t.ull - p->enr_prev.t.ull); // Time difference (long long)
	p->dn = p->enr.n - p->enr_prev.n;	// Number of counts

	/* Convert long long time difference to float */
	float ft = lltoflt(p->dt);

	/* Idle encoder has no IC flags so 't' doesn't update. */
	if (p->dt != 0) // Avoid divide by zero giving NAN
	{ 
		p->r = (float)p->dn/ft;	// encoder counts/per counter tick
	}
	else
	{
		p->r = 0.0;
	}
p->ft = ft;
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
	/* Read twice to be sure copy didn't get hit by a new encoder count */
	while (1==1)
	{
		*p = enr_wrk[unit];	// Local copy
		if ((p->n == enr_wrk[unit].n) && (p->t.ll == enr_wrk[unit].t.ll)) 
			break;
		encoder_get_reading_loop_cnt += 1;
	}
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
	p->enr_prev = p->enr;	// Update previous
	p->enr = enr_tmp;	// Copy new reading
	encoder_speed(p);	// Compute speed (rate)
	return;
}
/* ######################################################################################
   Entered via: can_driver->can_msg_reset->here with RX0, RX1 CAN msg
   ###################################################################################### */
void can_msg_check(void* pctl, struct CAN_POOLBLOCK* pblk)
{

return; // ### DEBUG
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

/* TEST: Save each IC reading in a big buffer: encoder segment-to-segment variation. */
#ifdef IC_TO_IC_TIME_W_BIG_BUFFER
struct ENCODERREADING enr_test[ENCTESTBUFFSIZE];
unsigned int enr_test_ct;
struct ENCODERREADING enr_testB[ENCTESTBUFFSIZE];
unsigned int enr_test_ctB;
#endif


/* TEST: save readings each OC timed 1/64th to look at speed variance */
#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS
struct ENCODERREADING enr_test64[ENCTESTVARBUFFSIZE];
struct ENCODERREADING *penr_test64_put = &enr_test64[0];
struct ENCODERREADING *penr_test64_get = &enr_test64[0];
struct ENCODERREADING *penr_test64_begin = &enr_test64[0];
struct ENCODERREADING *penr_test64_end = &enr_test64[ENCTESTVARBUFFSIZE];
#endif

#ifdef  IMDRIVECALIBRATION
static void imtest_put(void);
uint64_t imcalacum[2][IMCALTESTBUFFSIZE];	// Accumulators for each segment slot
uint32_t im_idx_str;	// Buff being stored (first index)
uint32_t im_idx_i;	// Next available interrupt (low ord index) 
uint32_t im_rev; // Count of revolutions
uint64_t imcaltime_prev; // Previous IC time
uint64_t im_tmp;
uint32_t im_n_prev;
uint32_t im_n_er;	// Ct times IC had a count that was not 2
int im_otosw = IMCALTESTCOUNTDOWN;	// Number of ICs before start
struct ENCODERREADING imr;
uint64_t imcaltime_begin;
uint64_t imcaltime_end;


#endif


/* ===================================================================================== */
/* static void IC_only(struct ENCODERREADING *p, uint32_t ccr,  uint32_t enccr)
 * @brief  : Input capture flag only for CH3 or CH4
 * @param  : p = pointer to encoder data struct for the channel with interrrupt flag
 * @param  : ccr = register TIM3_CCRx (IC channel for TIM3)
 * @param  : enccr = TIMx_CCR1 (timer for encoder)
 * @return : t & n have the latest time and encoder count
======================================================================================== */
static void IC_only(volatile struct ENCODERREADING *p, uint32_t ccr,  uint32_t enccr)
{
	/* Save extended time for new input capture */
	p->t.us[0] = ccr;		// Read the captured count which resets the capture flag
	p->t.us[1] = ovcnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
	p->t.ui[1] = ovcnt.ui[1];	// Extended time of upper 32 bits of long long

	p->n = enccr;	// Save latest encoder from encoder timer
p->icn +=1 ; // Count input captures
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
static void IC_ov(volatile struct ENCODERREADING *p,  uint32_t ccr, uint32_t enccr)
{
	/* Compute extended time for new input capture */
	p->t.us[0] = ccr;		// Read the captured count which resets the capture flag
	p->t.us[1] = ovcnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
	p->t.ui[1] = ovcnt.ui[1];	// Extended time of upper 32 bits of unsigned long long

	// Adjust input capture: Determine which flag came first.  If overflow came first increment the overflow count
	if (p->t.us[0] > 32767)		// Is the capture time in the upper half of the range?
	{ // Here, yes.  The IC flag must have preceded the overflow flag, so we 
		p->t.ull-= 0x10000;	// decrement the already incremented high order 48 bits 0f the *input capture time*
	}
	p->n = enccr;	// Save latest encoder count from encoder timer
p->icn +=1 ; // Count input captures
	return;
}
/* ===================================================================================== */
void TIM3_IRQHandler(void)
{
	volatile uint32_t temp = DTWTIME;// Save DTW 32b sys counter (if time sync of OC implemented)
	uint16_t usSR = TIM3_SR;	// Get flags at this instant in time

	/* OC timing TIM3_CH2 (1/2 ms CAN msg polling trigger) */
	if ((usSR & 0x04) != 0)
	{ // Here OC flag is on
		TIM3_SR = ~0x04;	// Reset flag
		TIM3_CCR2 += OCTICKSFOR1MS;	// Next OC for 1/2 ms
		NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (lower priority) interrupt
		encoder_timers_poll_ctr += 1;	// Tick count
	}
	
	if ((usSR & 0x1) != 0)
	{ // Here, overflow flag
		TIM3_SR = ~0x1;		// Reset overflow flag
		ovcnt.ull += 0x10000;	// Increment the high order 48 bits of extended 64b time counter
		
		if ((usSR & 0x08) != 0)
		{ // CH3 Input Capture flag w overflow flag
			IC_ov(&enr_wrk[0], TIM3_CCR3,TIM2_CNT);// CH3
	
#ifdef IC_TO_IC_TIME_W_BIG_BUFFER // define in .h file so 'main' sees it too
if(enr_test_ctB < ENCTESTBUFFSIZE)
{ // Here, available space remains
   encoder_get_reading(&enr_testB[enr_test_ctB], 0);
   enr_test_ctB += 1;
}
#endif
		}
		if ((usSR & 0x10) != 0)
		{ // CH4 Input Capture flag w overflow flag

			IC_ov(&enr_wrk[1], TIM3_CCR4,TIM5_CNT);// CH4

#ifdef IC_TO_IC_TIME_W_BIG_BUFFER
if(enr_test_ct < ENCTESTBUFFSIZE)
{
   encoder_get_reading(&enr_test[enr_test_ct], 1);
   enr_test_ct += 1;
}
#endif

#ifdef  IMDRIVECALIBRATION
imtest_put();
#endif
		}		
	}
	else
	{ // Here no overflow flag
		if ((usSR & 0x08) != 0)
		{ // CH3 Input Capture flag

			IC_only(&enr_wrk[0], TIM3_CCR3,TIM2_CNT);// CH3	

#ifdef IC_TO_IC_TIME_W_BIG_BUFFER
if(enr_test_ctB < ENCTESTBUFFSIZE)
{
   encoder_get_reading(&enr_testB[enr_test_ctB], 0);
   enr_test_ctB += 1;
}
#endif

		}
		if ((usSR & 0x10) != 0)
		{ // CH4 Input Capture flag 
			IC_only(&enr_wrk[1], TIM3_CCR4,TIM5_CNT);// CH4

#ifdef IC_TO_IC_TIME_W_BIG_BUFFER
if(enr_test_ct < ENCTESTBUFFSIZE)
{
   encoder_get_reading(&enr_test[enr_test_ct], 1);
   enr_test_ct += 1;
}
#endif

#ifdef  IMDRIVECALIBRATION
imtest_put();
#endif

		}
	}	
	
	/* OC timing TIM3_CH1 */
	if ((usSR & 0x02) != 0)
	{ // Here OC flag is on
		TIM3_SR = ~0x02;	// Reset flag
		TIM3_CCR1 += INTERVAL_TICKS;	// Compute next OC time call goes here
//TIM3_CCR1 += 56534; // Close to synchronous to test motor speed

// #### NOTE: Currently no syncing of OC to CAN time msgs #####

		/* End of 1/64th sec tick */
		interval_ct += 1;
		if (interval_ct >= INTERVAL_CT) // End of 1/64th sec tick?
//if (interval_ct >= 25) // Close to synchronous to test motor speed
		{ // Here, end of 1/64th interval
			interval_ct = 0;	// Reset interval counter
			dtw_oc = temp;		// Save: might be used for time sync'ing
			encode_oc_ticks += 1;	// Signal somebody that something happened

#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS // define in .h file so 'main' sees it too
encoder_get_reading(penr_test64_put, 1); // Get and store reading for UNIT 1 only
penr_test64_put += 1;	// Advance pointer in circular buffer
if (penr_test64_put >= penr_test64_end) penr_test64_put = penr_test64_begin;
#endif

		}
	}

	return;
}
/******************************************************************************
 * struct ENCODERREADING* encoder_getOC64(void);
 * @brief	: Get 1/64 second "n" and "t" if available
 * @return	: pointer: null = no new data; pointer to struct with new data
*******************************************************************************/
#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS // define in .h file so 'main' sees it too
struct ENCODERREADING* encoder_getOC64(void)
{
	struct ENCODERREADING* p;
	if (penr_test64_get == penr_test64_put) return 0;
	p = penr_test64_get;
	penr_test64_get += 1;
	if (penr_test64_get >= penr_test64_end) penr_test64_get = penr_test64_begin;
	return p;
}
#endif

/******************************************************************************
 * static void imtest_put(void);
 * @brief	: Induction motor test: Average time between ICs by segment index in an array
*******************************************************************************/
/*
  Accumulate input capture time between encoder segements by encoder segment, after
startup.  Startup delays until IC time is reasonable for a 3600 rpm induction motor,
whicha allows it to come up to speed, and after coming up to speed delays via a
countdown counter.

 When the specified number of revolutions has taken place the storing stops.  'main'
sees the im_otosw as -1, indicating the storing is complete.  After outputting the
data 'main' clears the accumulator array and resets the im_otosw for another round
of storing.

*/
#ifdef  IMDRIVECALIBRATION
static void imtest_put(void)
{
 	if (im_otosw >= 0) // Skip everything when test complete
 	{ // Here, not in stopped mode (i.e. stopped = -1)
   		encoder_get_reading(&imr, 1); // Get latest reading
   		im_tmp = imr.t.ll - imcaltime_prev; // Time between ICs
		imcaltime_prev = imr.t.ll;
   		if (im_otosw > 0) // Countdown in progress?
   		{ // Be sure motor up to speed before starting test
    			if ((im_tmp < 4500) && (im_tmp > 3500))
    			{ // Duration reasonable, now give it some more time
     				im_otosw -= 1; // Count down
     				if (im_otosw > 0) 
					return; // Return: more countdowns needed
     				imcaltime_begin = imr.t.ll; // Save start time
    			}
			else
			{
				return;
			}
   		}
   		// Here, test has started
		if ((im_n_prev - imr.n) != 2) im_n_er += 1;
		im_n_prev = imr.n;
   		imcalacum[im_idx_str][im_idx_i] += im_tmp; // Accumulate time between ICs
   		im_idx_i += 1; 	// Next encoder segment is array position
   		if (im_idx_i >= IMCALTESTBUFFSIZE) // End of a revolution?
   		{ // Here, yes.  Cycle back, or end test
    			im_idx_i = 0;	// Reset to beginning of array
    			im_rev += 1;	// Encoder revolution counts
    			if (im_rev >= IMCANTESTREVCTMAX) // End averaging?
    			{ // Time to stop collecting data
				im_rev = 0;
				im_idx_str = (im_idx_str ^ 1);
      				imcaltime_end = imr.t.ll; // Save end time
    			}
   		}
 	}
}
#endif

