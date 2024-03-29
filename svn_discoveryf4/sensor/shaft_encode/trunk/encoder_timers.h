/******************************************************************************
* File Name          : encoder_timers.h
* Date First Issued  : 07/01/2017
* Board              : Discovery F4
* Description        : TIM2,TIM5,TIM3--two encoders with speed timing
*******************************************************************************/

#ifndef __ENCODER_TIMERS
#define __ENCODER_TIMERS

union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ull;
	signed short	s[4];
	signed int	i[2];
	long long          ll;	
};

struct ENCODERREADING
{
	union TIMCAPTURE64 t;	// TIM3 latest extended time
	int32_t n;		// Encoder Count (latest)
int32_t icn; // DEBUG: input capture counter
};

struct ENCODERCOMPUTE
{
   struct ENCODERREADING enr;		// Lastest
   struct ENCODERREADING enr_prev;	// Previous
	union TIMCAPTURE64 t;	   // Time (latest)
	uint64_t dt;	// Diff = last-prev
	int32_t dn;	// Diff = last-prev
	float r;	// Rate = dn/dt
//	uint16_t flag;	// Patriotism
float ft; // Debug
};

/* ************************************************************************************** */
 int encoder_timers_init(uint32_t canid);
/* @brief	: Initialize TIM2,5,3 for shaft encoder
 * @param	: canid = CAN id for time sync msgs
 * @return	: 0 = OK; not 0 = fail
 * ************************************************************************************** */
void encoder_get_all(struct ENCODERCOMPUTE *p, uint16_t unit);
/* @brief	: Retrieve current encoder reading with computed speed
 * @param	: p = pointer to encoder data struct to be updated/computed
 * @param	: unit = 0 TIM2 encoder; 1 = TIIM5 encoder
 * @return	: re-populated struct
***************************************************************************************** */
void encoder_leds(void);
/* @brief	: All four LEDs follow the two Encoder phase signals
 * ************************************************************************************** */
void encoder_speed(struct ENCODERCOMPUTE *p);
/* @brief : compute rate and buffer stuff
 * @param : p = pointer to encoder data struct
***************************************************************************************** */

extern uint32_t encode_oc_ticks;	// 1/64sec tick flag

/* Count rare occurences of 'encoder_get_reading' loop hit by interrupt */
extern unsigned long encoder_get_reading_loop_cnt;
extern uint32_t encoder_timers_poll_ctr;	// Tick count

/* Test: store readings each IC  */
//#define IC_TO_IC_TIME_W_BIG_BUFFER  // Uncomment this statement to enable

#ifdef IC_TO_IC_TIME_W_BIG_BUFFER
#define ENCTESTBUFFSIZE 1600
extern struct ENCODERREADING enr_test[ENCTESTBUFFSIZE];
extern unsigned int enr_test_ct;

#endif

/* Test: store readings for 2nd encoder (unit=1) each OC timed 1/64th sec */
//#define CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS  // Uncomment this statement to enable

#ifdef CIRCULARBUFF_64PERSEC_OC_TIMED_READINGS
#define ENCTESTVARBUFFSIZE 64
/******************************************************************************/
struct ENCODERREADING* encoder_getOC64(void);
/* @brief	: Get 1/64 second "n" and "t" if available
 * @return	: pointer: null = no new data; pointer to struct with new data
*******************************************************************************/
#endif

/* Test: induction motor drive encoder to determine encoder error calibration */
/*
The time between the current input capture (IC) and the previous one is accumulated in
an array each IC.  After a specified number number of rotations the arrays are switched.
The array with the test just completed is dumped with a 'xprintf' in the main, 
'shaft_encoder.c' and the array is zero'ed.

The instantaneous speed of the induction motor driving the encoder test varies as
the stator vector rotates.  The rotor has a small slip so the array position slowly
shifts with respect to the stator.  After a large number of rotations the variations
in the instantaneous speed of the rotor has been averaged out, leaving the calibration
for the encoder, by segment number.  Note, however, the encoders do not have a
zero channel so each time the program is started the calibration will be have an
arbitrary start point.
*/
//#define IMDRIVECALIBRATION	// Uncomment to enable test code

#ifdef  IMDRIVECALIBRATION
#define IMCALTESTSEGMENTS  360	// Number of encoder segments
#define IMCALTESTBUFFSIZE  IMCALTESTSEGMENTS	// Size equals number of encoder segments
#define IMCANTESTREVCTMAX  (3570*10)		// Number of revolutions in the test
#define IMCALTESTCOUNTDOWN 	(360*60*1)	// Count down delay
extern uint64_t imcalacum[2][IMCALTESTBUFFSIZE];	// Accumulators for each segment slot
extern uint32_t im_idx_str;	// Buff being stored (first index)
extern uint32_t im_idx_i;	// Next available interrupt (low ord index) 
extern uint32_t im_rev; 	 // Count of revolutions before switching buffers
extern uint64_t imcaltime_prev; // Previous IC time
extern uint64_t imcaltime_begin;// Start of data storing
extern uint64_t imcaltime_end;	 // End time of data storing
extern int im_otosw;		 // Number of ICs before start
extern uint64_t im_tmp;	 // Use 'extern' for debugging
extern uint32_t im_n_prev;
extern uint32_t im_n_er;	// Ct times IC had a count that was not 2
#endif

#endif 

