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

#endif
