/******************************************************************************
* File Name          : IRQ_priority_powerbox.h
* Date First Issued  : 06/15/2018
* Board              : f103
* Description        : IRQ priorities for powerbox
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IRQ_PRIORITIES_PWRBOX
#define __IRQ_PRIORITIES_PWRBOX

#define DMA1_CH1_PRIORITY_PWR            0x05  // DMA 1/2 and end of buffer interrupt for adc

#define TIM4_PRIORITY_ADC                0x90  // Post DMA interrupt processing of adc/dma buffer

#define NVIC_CAN_RX1_IRQ_PRIORITY		  0x70  // CAN1: Receive FIFO 1 (and related) [time sync]

#define NVIC_CAN_SCE_IRQ_PRIORITY		  0x70  // Errors & status change
#define NVIC_USB_HP_CAN_TX_IRQ_PRIORITY  0x70  // CAN1: Transmit FIFO
#define NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY 0x70  // CAN1: Receive FIFO 0 (and related) 



#endif
