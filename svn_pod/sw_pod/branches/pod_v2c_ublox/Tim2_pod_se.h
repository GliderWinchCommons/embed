/******************************************************************************
* File Name          : Tim2_pod_se.h
* Date First Issued  : 06/05/2017
* Board              : STM32F103VxT6_pod_mm
* Description        : GPS 1PPS and 64 Hz interrupt
*******************************************************************************/
#ifndef __TIM2_POD_SE
#define __TIM2_POD_SE

/******************************************************************************/
void Tim2_pod_init(void);
/* @brief	: Initialize Tim2 for input capture
*******************************************************************************/
unsigned int Tim2_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 Tim2_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/

extern u16	tim2_64th_0_er;	// Count of date/time adjustments at interval 0
extern u16	tim2_64th_31_er;// Count of date/time adjustments at interval 31

/* Running sums where the whole is added to an OC interval (and removed from the sum) */
extern s32	phasing_sum;		// Running sum of accumulated phase adjustment
extern s32	deviation_sum;		// Running sum of accumulated error

extern unsigned int ticksm;	// Save ticks per sec for 'g' command monitoring

/* Save foregoing for gps monitor command */
extern s32	phasing_sum_g;		// Running sum of accumulated phase adjustment
extern s32	deviation_sum_g;	// Running sum of accumulated error

/* ==== Make available for printf Debugging ==== */
/* Scale 1 PPS interval durations to "whole.fraction" form. */
#define TIM2SCALE	16	// Number of bits to scale deviation of clock upwards

extern u32 tim2_oc_ticks_jam;
extern unsigned int ticksadj;
extern unsigned int ticks_dev;	// Number of ticks between 1 PPS less nominal
extern unsigned int tim2_num_intervals;
extern uint32_t ticks_per_oc_whole;
extern uint64_t tim2_ic_scaled;
extern uint32_t tim2_ic;	// Number of processor ticks between IC interrupts
extern uint64_t ticks_per_oc_scaled;
extern uint32_t ticks_per_oc_fraction;
extern int64_t ticks_ave_scaled;
extern int32_t tim2_diff;

extern volatile unsigned int tim2debug0;
extern volatile unsigned int tim2debug1;
extern volatile unsigned int tim2debug2;
extern volatile unsigned int tim2debug3;
extern volatile unsigned int tim2debug4;
extern volatile unsigned int tim2debug5;
extern volatile unsigned int tim2debug6;
extern volatile unsigned int tim2debug7;
extern volatile unsigned int tim2ticks;

extern u32	tickspersecHI;	// TIM2 clock ticks for one sec: high limit
extern u32	tickspersecLO;	// TIM2 clock ticks for one sec: low limit

#endif
