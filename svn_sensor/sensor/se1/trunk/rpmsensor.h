/******************************************************************************
* File Name          : rpmsensor.h
* Date First Issued  : 07/04/2013
* Board              : RxT6
* Description        : RPM sensing
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RPMSENSOR
#define __RPMSENSOR

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

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
void rpmsensor_init(void);
/* @brief 	: Initialize TIM4_CH4 and routines to measure rpm
*******************************************************************************/
struct TIMCAPTRET32 Tim4_inputcapture_ui(void);
/* @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
/* ########################## UNDER HIGH PRIORITY CAN INTERRUPT ############################### */
void rpmsensor_reset_timer(void);
/* Can time sync msg: reset subinterval and OC 
 * ############################################################################################### */

extern void 	(*tim4ocLOpriority_ptr)(void);	// CH3 OC triggers EXTI0_IRQHANDLER -> rpmsensor_compute -> tim4ocLOpriority_ptr

extern void (*rpmsensor_can_msg_reset_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk);	// Pointer for extending this routine's interrupt processing


#endif 

