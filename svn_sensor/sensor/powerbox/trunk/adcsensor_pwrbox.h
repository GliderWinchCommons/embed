/******************************************************************************
* File Name          : adcsensor_pwrbox.h
* Date First Issued  : 06/13/2018
* Board              : POD
* Description        : ADC routines for f103 Arduino board for powerbox unit
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_PWRBOX
#define __ADCSENSOR_PWRBOX

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


#define WAITDTW(tick)	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 )

//$ #define ADC_FSMC_PRIORITY	0x80	//    
#define ADC_TIM5_PRIORITY	0x80

#define DMA1_CH1_PRIORITY	0x60	// * ### Lower than SYSTICK (ADC filtering after DMA interrupt)

#define NUMBERADCCHANNELS_PWR	8	// Number of channels in one scan cycle
#define NUMBERSEQUENCES		32	   // Number of scan cycle sequences in 1/2 of the DMA buffer
#define RBUFSIZE 64	// Number of buffers

/* Indices: Readings arrays */
#define ADCX_IVREF	7	// Index in readings array: Internal Vref
#define ADCX_ITEMP	6	// Index in readings array: Internal temp
#define ADCX_SPAR1	5	// Index in readings array: spare
#define ADCX_HALLE   4  // Index in readings array: Hall-effect current sensor
#define ADCX_INPWR	3	// Index in readings array: Input power voltage
#define ADCX_DIODE	2	// Index in readings array: Diode
#define ADCX_CANVB	1	// Index in readings array: CAN bus voltage
#define ADCX_5VREG	0	// Index in readings array: 5v regulator


/* Indices: IIR filter array */
#define IIRX_INPWR	3	// Index in iir filter array: Input power voltage
#define IIRX_CANBV	2	// Index in iir filter array: CAN bus voltage voltage
#define IIRX_HALLE	1	// Index in iir filter array: Internal Vref
#define IIRX_5VREG	0	// Index in iir filter array: 5v regulator


/* input-(3)---diode-(2)--resistor-(1)--CAN bus pwr--switcher:5V--(0) */

/* ADC usage on Arduino Blue Pill F103 board
PA 0 ADC12-IN0	 Grn 5V power supply
PA 1 ADC12-IN1	 Yel Capacitor/bus voltage, .22 ohm output side
PA 2 ADC12-IN2	 Blu 0.22 ohm resistor, input side, after diodes
PA 3 ADC12-IN3	 Wht Input voltage (before diodes)
PA 4 ADC12-IN4	 Hall-effect current sensor 
PA 5 ADC12-IN5	 Spare divider
     ADC1 -IN16 Internal temp ref
     ADC1 -IN17 Internal voltage ref (Vrefint)
*/

/* DMA double buffer ADC readings. */
struct ADCDR_PWRBOX
{
	uint32_t in[2][NUMBERSEQUENCES][NUMBERADCCHANNELS_PWR];	// ADC_DR is stored for each channel in the regular sequence
	unsigned int cnt;       // DMA interrupt counter
	unsigned short flg;     // Index of buffer that is not busy (0, or 1)
};

/******************************************************************************/
void adcsensor_pwrbox_sequence(void);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/


/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
// These hold the address of the function that will be called
extern void 	(*dma_ll_ptr)(void);		// DMA -> FSMC  (low priority)

extern uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_PWR];	// Last computed & doubly filtered value for each channel

extern int32_t	adc_temp_flag[NUMBERADCCHANNELS_PWR];	// Signal main new filtered reading ready


#endif 
