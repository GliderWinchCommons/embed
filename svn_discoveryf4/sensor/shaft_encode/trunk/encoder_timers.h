/******************************************************************************
* File Name          : encoder_timers.h
* Date First Issued  : 07/01/2017
* Board              : Discovery F4
* Description        : TIM2,TIM5,TIM3--two encoders with speed timing
*******************************************************************************/

#ifndef __ENCODER_TIMERS
#define __ENCODER_TIMERS

#define TIM3_PRIORITY 0x30	// Timer priority


union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ll;
};

struct TIMCAPTRET32
{
	unsigned int 	ic;
	unsigned short	flg;
	unsigned int	cnt;	// RTC_CNT (in memory mirror)
};

/* **************************************************************************************/
int encoder_timers_init(void);
/* @brief	: Initialize TIM2,5,3 for shaft encoder
 * @return	: 0 = OK; not 0 = fail
 * ************************************************************************************** */


#endif 

