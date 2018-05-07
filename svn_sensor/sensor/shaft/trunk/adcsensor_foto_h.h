/******************************************************************************
* File Name          : adcsensor_foto_h.h
* Date First Issued  : 01/31/2014
* Board              : RxT6
* Description        : ADC routines for f103 sensor--sensor board histogram collection
*******************************************************************************/
/*
11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.h
04/21/2018 This is a hack of svn_sensor/sensor/se4_h/trunk/adcsensor_foto.h

Strategy--
Two photocell emitters are measured using on ADC3 and ADC2.  The threshold registers
are used to trigger an interrupt when there is a transition between states of the 
sensed photo reflections.

DMA stores readings for generation of a histogram.  Since DMA1 stores ADC2 in the high
half of the 32b ADC1 data register and the watchdog for ADC1 is 16b, the comparison
to the ADC1 DR no longer works.  Therefore, ADC1 is used for the DMA storing of ADC2, but
ADC2 and ADC3 are used for the watchdog detection.

11/15/2012 - POD prototype setup
Emitter resistors--3.3K
Diode resistors--330
ADC voltage applied--3.3v

05/03/2018
Command CAN msg from PC:
  SHAFT_CANID_HW_FILT3  (e.g. CANID_CMD_SHAFT1I for first instance)
Payload for this CAN msg:
 [0] command code: CMD_REQ_HISTOGRAM (e.g.40)
 [1]-[4] Duration for building table with bin's


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_FOTO_H
#define __ADCSENSOR_FOTO_H

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "common_misc.h"
#include "common_highflash.h"
#include "shaft_function.h"

#define NUMBERADCCHANNELS_FOTO	1	// Number of channels each ADC is to scan
#define DISCARD_FOTO		32	// Number of readings to discard before filtering starts
/* Decimation|CICSCALE (for unity scaling) 
    1  3
    2	 6
    4	 9
    8	12
   16	15
   32	18                                                                     */
#define DECIMATION_FOTO		4	// Decimation ratio
#define CICSCALE_FOTO		9	// Right shift count to scale result

#define ADCERRORCTRSIZE	4	// Number of error counters for adcsensor.c

/* These define the initial values for the ADC watchdog registers */
#define ADC3_HTR_INITIAL	900		// High threshold register setting, ADC3
#define ADC3_LTR_INITIAL	400		// Low  threshold register setting, ADC3
#define ADC2_HTR_INITIAL	1100		// High threshold register setting, ADC2
#define ADC2_LTR_INITIAL	700		// Low  threshold register setting, ADC2

/* Histogram additions */
union ADC12VAL		// In dual mode: DMA stores pairs of ADC readings, two 16b readings in one 32b word
{
	u32	ui;
	u16	us[2];
};

#define ADCVALBUFFSIZE	64	// Number of ADC readings in a *half* DMA buffer
#define ADCHISTOSIZEN      6 // Histogram bin size  2^N
#define ADCHISTOSIZE (1 << ADCHISTOSIZEN)	// Number of histogram bins (that accumulate counts)



/******************************************************************************/
void adc_init_sequence_foto_h(struct SHAFT_FUNCTION* p);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: p = pointer to sram with variables and local copy of parameters
*******************************************************************************/
void adcsensor_foto_h_enable_interrupts(void);
/* @brief 	: do as the name says
*******************************************************************************/
void adcsensor_foto_h_disable_histogram(void);
/* @brief 	: Stop DMA interrupts for histogram (storing stills goes on)
*******************************************************************************/
void adcsensor_foto_h_enable_histogram(void);
/* @brief 	: Enable DMA and low level interrupts for histogram
*******************************************************************************/
void adcsensor_rpm_compute(void);
/*	@brief	: Compute rpm based on differences from last call to this routine
 ******************************************************************************/
void adcsensor_load_pay_hb_speed(float f);
/*	@brief	: Assemble payload from input of status byte, and a unsigned int
 * @param	: f = float to be loaded into CAN msg paylaod
 ******************************************************************************/


extern long speed_filteredA2;		// Most recent computed & filtered rpm
extern u32 encoder_ctrA2;		// Most recent encoder count
extern u32 adc_encode_time_prev2;	// Previous time at SYSTICK
extern s32 encoder_ctr_prev2;		// Previous encoder running count
extern s32 encoder_ctr2;

/* Error counters */
extern u32 adcsensor_foto_err[ADCERRORCTRSIZE];	// Error counters
extern uint32_t adcsensor_nrpmhb;	// Latest nrpm buffered for 'main'
extern uint8_t iirflag;	// Flag 'main' for new rpm data in buffer


/* ============ Histogram related ======================*/
/*
CAN received: histogram request id: SHAFT_CANID_HW_FILT3 (e.g. CANID_CMD_SHAFT1I)
 [0] CMD_REQ_HISTOGRAM (e.g. code = 40)
 [1] ADC number ADC number (2, 3, or 4 for both)
 [2] Number of consecutive histograms taken & sent
 [3]-[6] Duration: number of DMA buffers in histogram

CAN received: histogram request id: CANID_CMD_SHAFT1R
 [0] CMD_THISIS_HISTODATA (e.g. code = 41)
 [1] ADC number (2, 3)
 [2] bin number (0-255)
 [3]-[6] bin count

*/
#define HISTOBINBUFNUM 4 // Number of bin buffers
struct ADCHISTO
{
	u32  bin[HISTOBINBUFNUM][ADCHISTOSIZE];	// Double buffer bin counts
	u32* pbegin;   // Beginning of bin buffer
	u32* pend;     // End of bin buffer
	u32* ph;       // Working bin buffer ptr
	int  idx_tmp;       // index: Temporary
	int  idx_build;     // index: adding buffer
	int  idx_send;      // index: sending buffer
	int  ctr;           // Number of ADC buffers in histogram
	int  dur;           // Number to accumulate
	int  sw;            // Histogram request switch
struct CANRCVBUF can_cmd_histo;	    // CAN msg: Histogram data
struct CANRCVBUF can_cmd_histo_final;// CAN msg: Signal end of sending
};
extern u32 adchisto_complete_flag;   // Flag when duration count reached
/******************************************************************************/
void adc_histo_request(struct CANRCVBUF* p);
/* @brief 	: Handle CAN msg request for histogram
 * @param	: p = pointer to 'struct' with CAN msg
*******************************************************************************/
void adc_histo_send(struct ADCHISTO* p);
/* @brief 	: Handle CAN msg request for histogram
 * @param	: p = pointer to struct holding ADC bins
*******************************************************************************/

extern struct ADCHISTO adchisto2; // ADC2
extern struct ADCHISTO adchisto3; // ADC3

#endif 
