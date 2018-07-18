/******************************************************************************
* File Name          : adcsensor_pwrbox.c
* Date First Issued  : 06/13/2018
* Board              : POD
* Description        : ADC routines for f103 Arduino board for powerbox unit
*******************************************************************************/
/* 
06/13/2018 - Hack of adcsensor_tension.ch 

02/14/2015

04/20/2014
CAN messages:


11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.c
See p214 of Ref Manual for ADC section 
02/12/2015 This is a hack of adcsensor_tension.c

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

Strategy--
Four ADC pins are read at a rapid rate with the DMA storing the sequence.  The DMA wraps around
at the end of the buffer.  The code for DMA interrupts is not used.  
 
*/
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "libmiscstm32/DTW_counter.h"
#include "common.h"
#include "common_can.h"
#include "pinconfig_all.h"
#include "adcsensor_pwrbox.h"
#include "DTW_counter.h"
#include "cic_filter_l_N2_M3.h"
#include "IRQ_priority_powerbox.h"

#include "iir_filter_lx.h"
#include "pwrbox_function.h"

static void adc_accum(void);

/* ADC usage on Arduino Blue Pill F103 board
Scan seq: Port: pin: ADC: Description
0 PA 0 ADC12-IN0	 Grn 5V power supply
1 PA 1 ADC12-IN1	 Yel Capacitor/bus voltage
2 PA 2 ADC12-IN2	 Blu 0.22 ohm resistor
3 PA 3 ADC12-IN3	 Wht Input voltage
4 PA 4 ADC12-IN4	 Hall-effect current sensor
5 PA 5 ADC12-IN5	 Spare divider
6      ADC1 -IN16 Internal temp ref (17.1 us sample time target)
7      ADC1 -IN17 Internal voltage ref (17.1 us sample time target)
*/

/*
Bits 29:0 SMPx[2:0]: Channel x Sample time selection for less than 1/4 LSB of 12 bit conversion.
           These bits are written by software to select the sample time individually for each channel.
           During sample cycles channel selection bits must remain unchanged.
           000: 1.5 cycles	@14MHz  0.4K max input R (p 123 datasheet)
           001: 7.5 cycles	@14MHz  5.9K max input R 
           010: 13.5 cycles	@14MHz 11.4K max input R
           011: 28.5 cycles	@14MHz 25.2K max input R
           100: 41.5 cycles	@14MHz 37.2K max input R
           101: 55.5 cycles	@14MHz 50.0K max input R
           110: 71.5 cycles	50K max
           111: 239.5 cycles	50K max

With system clock of 64 MHz and p2 clock of 32 MHz, the ADC clock
will be 32/4 = 8 MHz (max freq must be less than 14 MHz)

With an ADC clock of 4 MHz the total conversion time--
  12.5 + SMPx time

[2] 13.5 + 12.5 = 26.0 adc cycles per External ADC
[3] 28.5 + 12.5 = 41.0 adc cycles for Internal temp
[3] 28.5 + 12.5 = 41.0 adc cycles for Internal V ref

6 External input     6 *  41 = 246
1 Internal temp      1 *  41 =  41
1 Internal volt ref  1 *  41 =  41
     Total per scan cycle    = 328
	  Total us per scan cycle = 328 / 4MHz = 82

Supose DMA 1/2 buffer holds 32 scan cycles--
  32 * 82 = 2624 us between DMA interrupts

*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP0	2
#define SMP1	2
#define SMP2	2
#define SMP3	2
#define SMP4	2
#define SMP5	2
#define SMP6	2
#define SMP7	2
#define SMP8	2
#define SMP9	3
#define SMP10	3
#define SMP11	3
#define SMP12	3
#define SMP13	3
#define SMP14	3
#define SMP15	3
#define SMP16	4
#define SMP17	4


/* ********** Static routines **************************************************/
static void adcsensor_pwrbox_init(void);
static void adcsensor_pwrbox_start_conversion(void);
static void adcsensor_pwrbox_start_cal_register_reset(void);
static void adcsensor_pwrbox_start_calibration(void);

/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority3_ptr)(void)  = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority3_ptr)(void)  = 0;	// SYSTICK handler (low high priority) continuation--1/2048th
void 	(*systickLOpriority3X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th

/* APB2 bus frequency is needed to be able to set ADC prescalar so that it is less than 14 MHz, 
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to time delays. */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

	struct ADCDR_PWRBOX  strADC1dr;	// Double buffer array of ints for ADC readings, plus count and index
static struct ADCDR_PWRBOX *strADC1resultptr;	// Pointer to struct holding adc data stored by DMA

/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
static void timedelay (unsigned int ticks)
{
	u32 t1 = ticks + DTWTIME; // DWT_CYCNT
	WAITDTW(t1);
	return;
}
/******************************************************************************
 * void adcsensor_pwrbox_sequence(void);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
void adcsensor_pwrbox_sequence(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Initialize ADC1 */
	adcsensor_pwrbox_init();	// Initialize ADC1 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adcsensor_pwrbox_start_cal_register_reset();
	timedelay(50 * ticksperus);	// Wait

	/* Start the register calibration process */
	adcsensor_pwrbox_start_calibration();
	timedelay(80 * ticksperus);	// WaitCICSCALE

/* Setup DMA for storing data in the ADC_DR as the channels in the sequence are converted */
	DMA1_CPAR1 = (u32)&ADC1_DR;				// DMA channel 1 peripheral address (adc1 data register)
	DMA1_CMAR1 = (u32)&strADC1dr.in[0][0][0];		// Memory address of first buffer array for storing data 
	DMA1_CNDTR1 = (2 * NUMBERSEQUENCES * NUMBERADCCHANNELS_PWR);// Number of data items before wrap-around

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY_PWR );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);
	DMA1_CCR1 |= DMA_CCR1_EN;

	ADC1_CR1 = (ADC_CR1_SCAN); 	// Scan mode

	/*           Use Internals   |    use DMA   |  Continuous */
	ADC1_CR2  |= (ADC_CR2_TSVREFE |  ADC_CR2_DMA | ADC_CR2_CONT);

	/* ADC is now ready for use.  Start ADC conversions (writes a 2nd ADC_CR2_ADON) */
	adcsensor_pwrbox_start_conversion();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adcsensor_pwrbox_init(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
const struct PINCONFIGALL pin_adc_0 = {(volatile u32 *)GPIOA, 0, IN_ANALOG, 0};
const struct PINCONFIGALL pin_adc_1 = {(volatile u32 *)GPIOA, 1, IN_ANALOG, 0};
const struct PINCONFIGALL pin_adc_2 = {(volatile u32 *)GPIOA, 2, IN_ANALOG, 0};
const struct PINCONFIGALL pin_adc_3 = {(volatile u32 *)GPIOA, 3, IN_ANALOG, 0};
const struct PINCONFIGALL pin_adc_4 = {(volatile u32 *)GPIOA, 4, IN_ANALOG, 0};
const struct PINCONFIGALL pin_adc_5 = {(volatile u32 *)GPIOA, 5, IN_ANALOG, 0};

static void adcsensor_pwrbox_init(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/*  Setup ADC pins for ANALOG INPUT */
	pinconfig_all( (struct PINCONFIGALL *)&pin_adc_0);
	pinconfig_all( (struct PINCONFIGALL *)&pin_adc_1);
	pinconfig_all( (struct PINCONFIGALL *)&pin_adc_2);
	pinconfig_all( (struct PINCONFIGALL *)&pin_adc_3);
	pinconfig_all( (struct PINCONFIGALL *)&pin_adc_4);
	pinconfig_all( (struct PINCONFIGALL *)&pin_adc_5);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 0;			// Division by 2	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 3;	// Division by 8
	if (pclk2_freq/6 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/4 < 14000000) ucPrescalar = 1;	// Division by 4

	/* ==>Override the above<==  32 MHz pclk2, divide by 8 -> 4 MHz. */
	ucPrescalar = 3;

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= RCC_APB2ENR_ADC1EN;	// Enable ADC1 clocking

	RCC_AHBENR |= RCC_AHBENR_DMA1EN;		// Enable DMA1 clock

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14); // Set code for bus division

	// Channel configuration register
	//          priority high  | 32b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE;//  | DMA_CCR1_EN;

	// Turn ADC on, but conversions don't start until 2nd ADC_CR2_ADON written
	ADC1_CR2  = ADC_CR2_ADON; 
	
	/* 1 us Tstab time delay */
	timedelay(2 * ticksperus);	// Wait

	/* Set sample times for channels in scan sequence */
	ADC1_SMPR2 = ((SMP1  <<  0)|(SMP2  <<  3)|(SMP3 << 6)|(SMP4 << 9)|(SMP5 << 12)|(SMP6 << 15));
	ADC1_SMPR1 = ((SMP16 << 18)|(SMP17 << 21));	// Internal temp and v reference

	/* Setup the number of channels scanned */
	ADC1_SQR1 =  ((NUMBERADCCHANNELS_PWR-1) << 20);	// Channel count

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN0 - IN5, IN16, IN17 for conversions sequences
	ADC1_SQR3 = ( 0 << 0) | ( 1 << 5) | (2 << 10) | (3 << 15) | (4 << 20) | (5 << 25);// Seq 1 - 5
	ADC1_SQR2 = (16 << 0) | (17 << 5); // Seq 7, 8

	/* Low level interrupt for doing adc filtering following DMA1 CH1 interrupt. */
	NVICIPR (NVIC_TIM4_IRQ, TIM4_PRIORITY_ADC);	// Set low level interrupt priority for post DMA1 interrupt processing
	NVICISER(NVIC_TIM4_IRQ);			// Enable low level interrupt

	return; // Initial init complete.  Calibrations needed.
}
/******************************************************************************
 * static void adcsensor_pwrbox_start_cal_register_reset(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adcsensor_pwrbox_start_cal_register_reset(void)
{
	/* Reset calibration register  */
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adcsensor_pwrbox_start_calibration(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adcsensor_pwrbox_start_calibration(void)
{
	/* Start calibration  */
	ADC1_CR2 |= ADC_CR2_CAL;			// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us) */
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adcsensor_pwrbox_start_conversion(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adcsensor_pwrbox_start_conversion(void)
{	
	/* Save pointer to struct where data will be stored.  ISR routine will use it */
	strADC1resultptr = &strADC1dr;		// Pointer to struct with data array and busy flag
	strADC1resultptr->flg = 1;		// We start filling buffer 0, so show previous buffer

/* After the initial write of this bit, subsequent writes start the ADC conversion--(p 241)
"Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application."
should allow a delay of tSTAB between power up and start of conversion. Refer to Figure 27 */
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	return;
}
/*#######################################################################################
 * ISR routine for DMA1 Channel1 reading ADC regular sequence of adc channels
 *####################################################################################### */
uint32_t dmat1;
uint32_t dmat2;
uint32_t dmatdiff;

void DMA1CH1_IRQHandler_pwrbox(void)
{
/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two buffers full a transfer complete interrut is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 

'cnt' is a running counter of sequences converted.  Maybe not too useful except for debugging */

	if ( (DMA1_ISR & DMA_ISR_TCIF1) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		strADC1resultptr->flg  = 1;	// Set the index to the second buffer.  It is ready.
		strADC1resultptr->cnt += 1;	// Running count of sequences completed
		DMA1_IFCR = DMA_IFCR_CTCIF1;	// Clear transfer complete flag (p 208)
	}
	else
	{
		if ( (DMA1_ISR & DMA_ISR_HTIF1) != 0 )	// Is this a half transfer complete interrupt?
		{ // Here, yes.  The first sequence has been converted and stored in the first buffer
dmat1 = DTWTIME;
dmatdiff = dmat1 - dmat2;
dmat2 = dmat1;
			strADC1resultptr->flg  = 0;	// Set index to the first buffer.  It is ready.
			strADC1resultptr->cnt += 1;	// Running count of sequences completed
			DMA1_IFCR = DMA_IFCR_CHTIF1;	// Clear half transfer complete flag (p 208)
		}
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_TIM4_IRQ);	// Set pending (low priority) interrupt

	return;
}
/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA -> FSMC  (low priority)

void TIM4_IRQHandler_pwr(void)
{
	adc_accum();	// Accumulate readings 1/2 DMA buffer

	/* Call other routines if an address is set up */
	if (dma_ll_ptr != NULL)	// Skip subroutine call if pointer not intialized
		(*dma_ll_ptr)();	// Go do something
	return;
}


/* ########################## UNDER MODERATE PRIORITY INTERRUPT ###################################### 
 * Accumulate ADC readings
 * This routine is entered from  'TIM4_IRQHandler' triggered by 'DMA1CH1_IRQHandler_tension'
 * ############################################################################################### */
uint32_t adcdb0;
uint32_t adcdb1;
/* Execution cycles: 1650 - 1778 every 60945 */

/* Circular buffer ADC readings accumulated in 1/2 of the DMA buffer */
uint32_t adc_readings_buf[RBUFSIZE][NUMBERADCCHANNELS_PWR]; // Accumulated readings buffer
uint32_t adc_readings_buf_idx_i = 0; // Buffer index: in
uint32_t adc_readings_buf_idx_o = 0; // Buffer index: out	

static void adc_accum(void)
{
adcdb0 = DTWTIME; // Execution time check

	int i;	// FORTRAN variable, of course
	uint32_t *pdma = &strADC1dr.in[strADC1resultptr->flg][0][0];
	uint32_t *pbuf;
	uint32_t *punf;
	
		pbuf = &adc_readings_buf[adc_readings_buf_idx_i][0];

		*(pbuf+0) = *(pdma+0);
		*(pbuf+1) = *(pdma+1);
		*(pbuf+2) = *(pdma+2);
		*(pbuf+3) = *(pdma+3);
		*(pbuf+4) = *(pdma+4);
		*(pbuf+5) = *(pdma+5);
		*(pbuf+6) = *(pdma+6);
		*(pbuf+7) = *(pdma+7);
		pdma += NUMBERADCCHANNELS_PWR;

	/* Accumulate remaining sequences */
	for (i = 1; i < NUMBERSEQUENCES; i++)
	{
		*(pbuf+0) += *(pdma+0);
		*(pbuf+1) += *(pdma+1);
		*(pbuf+2) += *(pdma+2);
		*(pbuf+3) += *(pdma+3);
		*(pbuf+4) += *(pdma+4);
		*(pbuf+5) += *(pdma+5);
		*(pbuf+6) += *(pdma+6);
		*(pbuf+7) += *(pdma+7);
		pdma += NUMBERADCCHANNELS_PWR;
	}

	/* Copy accumulated readings to a single buffer for fast CAN msg */
	punf = &pwr_f.iunf[0];
		*(punf+0) = *(pbuf+0);
		*(punf+1) = *(pbuf+1);
		*(punf+2) = *(pbuf+2);
		*(punf+3) = *(pbuf+3);
		*(punf+4) = *(pbuf+4);
		*(punf+5) = *(pbuf+5);
		*(punf+6) = *(pbuf+6);
		*(punf+7) = *(pbuf+7);

	/* Advance circular buffer with accumulated readings */
	adc_readings_buf_idx_i += 1;
	if (adc_readings_buf_idx_i >= RBUFSIZE) adc_readings_buf_idx_i = 0;

/* #defines reproduced here for convenient reference--
// Indices: Readings arrays
#define ADCX_ITEMP	7	// Index in readings array: Internal temp
#define ADCX_IVREF	6	// Index in readings array: Internal Vref
#define ADCX_SPAR1	5	// Index in readings array: spare
#define ADCX_HALLE   4  // Index in readings array: Hall-effect current sensor
#define ADCX_INPWR	3	// Index in readings array: Input power voltage
#define ADCX_DIODE	2	// Index in readings array: Diode
#define ADCX_CANVB	1	// Index in readings array: CAN bus voltage
#define ADCX_5VREG	0	// Index in readings array: 5v regulator

// Indices: IIR filter array 
#define IIRX_INPWR	3	// Index in iir filter array: Input power voltage
#define IIRX_CANBV	2	// Index in iir filter array: CAN bus voltage voltage
#define IIRX_HALLE	1	// Index in iir filter array: Internal Vref
#define IIRX_5VREG	0	// Index in iir filter array: 5v regulator

*/
	/* IIR filter readings for selected ADC readings execution: 515 cycles */
	iir_filter_lx_do(&pwr_f.adc_iir[IIRX_5VREG], (int32_t*)(pbuf+ADCX_5VREG) ); // 5v regulator
	iir_filter_lx_do(&pwr_f.adc_iir[IIRX_HALLE], (int32_t*)(pbuf+ADCX_HALLE) ); // Internal Vref
	iir_filter_lx_do(&pwr_f.adc_iir[IIRX_CANBV], (int32_t*)(pbuf+ADCX_CANVB) ); // CAN bus (output)
	iir_filter_lx_do(&pwr_f.adc_iir[IIRX_INPWR], (int32_t*)(pbuf+ADCX_INPWR) ); // Power source (input)

adcdb1 = DTWTIME - adcdb0; 
		
	return;
}

