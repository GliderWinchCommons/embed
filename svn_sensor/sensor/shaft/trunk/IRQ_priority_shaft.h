/******************************************************************************
* File Name          : IRQ_priority_shaft.h
* Date First Issued  : 04/07/2018
* Board              : f103
* Description        : IRQ priorities for SE1
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IRQ_PRIORITIES_SE1
#define __IRQ_PRIORITIES_SE1

#define ADC1_2_PRIORITY_FOTO_SHAFT       0x40  // ADC1_2 and ADC3 priority

#define DMA1_CH1_PRIORITY	              0x60	// ADC circular buffering interrupts
#define DMA2_CH5_PRIORITY	              0x60	// 

#define TIM4_PRIORITY_SHAFT              0x70  // RPM input capture & timing

#define NVIC_I2C1_ER_IRQ_PRIORITY_SE1    0xa0  // CAN_poll_loop_trigger
#define NVIC_EXTI0_IRQ_PRIORITY_SE1	NVIC_I2C1_ER_IRQ_PRIORITY_SE1 // TIM4 'rpmsensor.c' CH3 subinterval

#define NVIC_CAN_RX1_IRQ_PRIORITY		  0x70  // CAN1: Receive FIFO 1 (and related) [time sync]
#define NVIC_CAN2_RX1_IRQ_PRIORITY		  0x70  // CAN2: Receive FIFO 1 (and related) [time sync]

#define NVIC_CAN_SCE_IRQ_PRIORITY		  0x70  // Errors & status change
#define NVIC_USB_HP_CAN_TX_IRQ_PRIORITY  0x70  // CAN1: Transmit FIFO
#define NVIC_CAN2_TX_IRQ_PRIORITY		  0x70  // CAN2: Transmit FIFO
#define NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY 0x70  // CAN1: Receive FIFO 0 (and related) 
#define NVIC_CAN2_RX0_IRQ_PRIORITY       0x70  // CAN2: Receive FIFO 0 (and related)

#define DMA1_CH1_PRIORITY_SHAFT          0x05  // DMA 1/2 and end of buffer interrupt for adc


#endif
