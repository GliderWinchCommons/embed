/******************************************************************************
* File Name          : tim4_shaft.h
* Date First Issued  : 04/23/2018
* Board              : RxT6
* Description        : Reflective shaft encoder
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM4_SHAFT
#define __TIM4_SHAFT

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

#include "can_driver.h"

#define SUBINTERVALTRIGGER 14

union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ll;
};

struct TIMCAPTRET32
{
	unsigned int 	ic;	// 32b capture tick count
	unsigned int	flg;	// running count of ic's
//	unsigned int	cnt;	// not used in this implementation
};

/******************************************************************************/
void tim4_shaft_init(void);
/* @brief 	: Initialize TIM4 and routines to measure rpm
*******************************************************************************/
void tim4_shaft_enable_interrupts(void);
/* @brief	: Enable interrupts for EXTI0
 * ************************************************************************** */

/* ########################## UNDER HIGH PRIORITY CAN INTERRUPT ############# */
void tim4_shaft_reset_timer(void);
/* Can time sync msg: reset subinterval and OC 
 * ########################################################################## */

extern void (*tim4ocLOpriority_ptr)(void);	// CH3 OC triggers EXTI0_IRQHANDLER -> rpmsensor_compute -> tim4ocLOpriority_ptr
extern void (*tim4_shaft_tim_oc_ptr)(void);	// Low level interrupt trigger function callback	

extern void (*tim4_shaft_can_msg_reset_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk);	// Pointer for extending this routine's interrupt processing

/* Timing counter */
extern volatile uint32_t tim4_tim_ticks; // Running count of time ticks
extern uint32_t tim4_tim_rate_shaft;  // Number of ticks per sec (64E6/32E3)
extern uint32_t subinterval_ct_flag; // not zero = subinterval triggered
extern uint32_t subinterval_ct;	// Current subinterval count

#endif 

