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
};

struct ENCODERCOMPUTE
{
   struct ENCODERREADING enr;		// Lastest
   struct ENCODERREADING enr_prev;	// Previous
	union TIMCAPTURE64 t;	   // Time (latest)
	uint64_t dt;	// Diff = last-prev
	int32_t dn;	// Diff = last-prev
	float r;	// Rate = dn/dt
	uint16_t flag;	// Patriotism
};

/* **************************************************************************************/
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
*******************************************************************************/

extern uint32_t encode_oc_flag;	// 1/64sec tick flag


#endif 

