/******************************************************************************
* File Name          : adcsensor_eng.c
* Date First Issued  : 06/24/2013
* Board              : RxT6
* Description        : ADC routines for f103 sensor--engine configuration
*******************************************************************************/
/* 
This routine handles the ADC for--
1) Throttle position pot
2) Thermistor
3) Pressure sensor

04/07/2018
Major revision to se1 using database and parameters.

04/20/2014
CAN messages:
0x30800000 Throttle & thermistor (int & int)
0x40800000 RPM & Pressure (int & int)
0x70800000 Calibrated Temperature (deg C x100) (signed short)
0x80800000 Throttle (short (max value 4095) )

11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.c
See p214 of Ref Manual for ADC section 

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

Strategy--
Three ADC pins are read at a rapid rate with the DMA storing the sequence.  The DMA wraps around
at the end of the buffer.  The code for DMA interrupts is not used.  The 2048 per sec SYSTICK
interrupts in 'canwinch_pod_common_systick2048.c' comes (under high priority interrupt) to 
'adcsensor_eng_bufadd' where the latest adc readings read are stored in a buffer.  Next, under
low priority interrupt the readings are entered into a CIC filter.

When the SYSTICK routine ends a cycle of 32, which is the end of a 1/64th sec duration, a flag counter
increments so that cic filter readings are then loaded into a CAN message the message setup for sending.
 
*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "libusartstm32/commonbitband.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "pinconfig_all.h"
#include "adcsensor_eng.h"
//#include "canwinch_pod_common_systick2048.h"
#include "rpmsensor.h"
#include "CANascii.h"
#include "../../../../svn_common/trunk/db/gen_db.h"
#include "canmsg_send_pay_type.h"

#include "engine_function.h"
#include "IRQ_priority_se1.h"
#include "libmiscstm32/DTW_counter.h"

/* ADC usage
PA0 ADC123-IN0	Throttle potentiometer
PA1 ADC123-IN1 Thermistor
PA3 ADC123-IN2	Pressure sensor  
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
will be 32/8 = 4 MHz (max freq must be less than 14 MHz)
With an ADC clock of 4 MHz the total conversion time--
12.5 + SMPx time
At 0.250 us per adc clock cycle 
(1) Throttle: 		12.5 + 55.5 = 68; * 0.250 us = 17.00 us per conversion
(2) Thermistor: 	12.5 + 55.5 = 68; * 0.250 us = 17.00 us per conversion
(3) Pressure sensor:	12.5 + 55.5 = 68; * 0.250 us = 17.00 us per conversion
Total for sequence of three .......................  = 51.00 us
For 16 sequences per DMA interrupt................. = 816.00 us
The logging/message sample rate (1/2048)........... = 488.28125 us

*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP1	7	
#define SMP2	7
#define SMP3	7
#define SMP4	3
#define SMP5	3
#define SMP6	3
#define SMP7	3
#define SMP8	6
#define SMP9	6
#define SMP10	6
#define SMP11	6
#define SMP12	3
#define SMP13	3
#define SMP14	3
#define SMP15	3
#define SMP16	7
#define SMP17	7

/* ********** Static routines **************************************************/
static void adc_init_se_eng(void);
static void adc_start_conversion_se_eng(void);
static void adc_start_cal_register_reset_se_eng(void);
static void adc_start_calibration_se_eng(void);

/* CIC routines buried somewhere below */
static void adc_cic_init(void);

/* Circular buffer of sequences for delayed CIC filtering */
int adcrawbuff[ADCRAWBUFFSIZE * NUMBERADCCHANNELS_SE];
uint32_t adcidx_in = 0;		// Incoming index
uint32_t adcidx_out = 0;	// Outgoing index


/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority3_ptr)(void) = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority3_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/2048th
void 	(*systickLOpriority3X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th


/* APB2 bus frequency is needed to be able to set ADC prescalar so that it is less than 14 MHz, 
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

struct ADCDR_ENG  strADC1dr;	// Double buffer array of ints for ADC readings, plus count and index
struct ADCDR_ENG *strADC1resultptr;	// Pointer to struct holding adc data stored by DMA

/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
static void timedelay (unsigned int ticks)
{
	/* Start time count */
	u32 t0 = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	t0 += ticks;
	while (( (int)t0 ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 );
	return;
}
/******************************************************************************
 * void adc_init_se_eng_sequence(void);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
void adc_init_se_eng_sequence(void)
{
	/* Setup for lower level filtering of ADC readings buffered by DMA. */
	adc_cic_init();

	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Initialize ADC1 */
	adc_init_se_eng();			// Initialize ADC1 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adc_start_cal_register_reset_se_eng();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adc_start_calibration_se_eng();
	timedelay(80 * ticksperus);	// Wait

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion_se_eng();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adc_init_se_eng(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
const struct PINCONFIGALL pin_pa0 = {(volatile u32 *)GPIOA, 0, IN_ANALOG, 0};
const struct PINCONFIGALL pin_pa1 = {(volatile u32 *)GPIOA, 1, IN_ANALOG, 0};
const struct PINCONFIGALL pin_pa2 = {(volatile u32 *)GPIOA, 3, IN_ANALOG, 0};


static void adc_init_se_eng(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/*  Setup ADC pins for ANALOG INPUT (p 148) */
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa0);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa1);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa2);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 0;			// Division by 2	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 3;	// Division by 8
	if (pclk2_freq/6 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/4 < 14000000) ucPrescalar = 1;	// Division by 4

	/* ==>Override the above<==  32 MHz pclk2, divide by 8 -> 4 MHz.  This is a good compromise for
           the thermistor and throttle pot offsets due to source impedance. */
	ucPrescalar = 3;

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14)	; // Set code for bus division (p 98)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= RCC_APB2ENR_ADC1EN;	// Enable ADC1 clocking (see p 104)
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." 
	AWDEN - enable watchdog register p 229,30. */
	//         (   scan      | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (ADC_CR1_SCAN );

	/*               use DMA   |  Continuous   | Power ON 	*/
	ADC1_CR2  = (  ADC_CR2_DMA | ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)
	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */
	timedelay(2 * ticksperus);	// Wait

	/* Set sample times for channels used on POD board (p 236) */	
	ADC1_SMPR2 = ( (SMP3 << 6) | (SMP2 << 3) | (SMP1 << 0) );

	/* Setup the number of channels scanned (p 238) */
	ADC1_SQR1 =  ( (NUMBERADCCHANNELS_SE-1) << 20) ;	// Chan count, sequences IN0,1,2 (p 217, 238)

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN0, IN1, IN2, for conversions sequences (p 240)
	ADC1_SQR3 = (0 << 0) | (1 << 5) | (3 << 10); 

	/* Setup DMA for storing data in the ADC_DR as the channels in the sequence are converted (p 199) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;			// Enable DMA1 clock (p 102)
	DMA1_CNDTR1 = (NUMBERSEQUENCES * NUMBERADCCHANNELS_SE * 2 );// Number of data items before wrap-around
	DMA1_CPAR1 = (u32)&ADC1_DR;				// DMA channel 1 peripheral address (adc1 data register) (p 211, 247)
	DMA1_CMAR1 = (u32)&strADC1dr.in[0][0][0];		// Memory address of first buffer array for storing data (p 211)

	// Channel configurion reg (p 209)
	//          priority high  | 32b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE  | DMA_CCR1_EN;

/* NOTE: The following skips the DMA interrupt */
//	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_EN;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY_SE1 );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);

	return;
}
/******************************************************************************
 * static void adc_start_cal_register_reset_se_eng(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adc_start_cal_register_reset_se_eng(void)
{
	/* Reset calibration register  */
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adc_start_calibration_se_eng(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adc_start_calibration_se_eng(void)
{
	/* Start calibration  */
	ADC1_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adc_start_conversion_se_eng(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adc_start_conversion_se_eng(void)
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
uint32_t adcsensorDebugdmact;
uint32_t adcdb1;
uint32_t adcdb1_prev;
uint32_t adcdbdiff;
uint32_t dbgX;
#define DBGX 1
void DMA1CH1_IRQHandler(void)
{
/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two buffers full a transfer complete interrut is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 

'cnt' is a running counter of sequences converted.  Maybe not too useful except for debugging */


adcdb1 = DTWTIME;
adcdbdiff = adcdb1 - adcdb1_prev;
adcdb1_prev = adcdb1;


	int *p1 = &adcrawbuff[adcidx_in];
	int *p2;
	int i;

	if ( (DMA1_ISR & DMA_ISR_TCIF1) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
//		strADC1resultptr->flg  = 1;	// Set the index to the second buffer.  It is ready.
//		strADC1resultptr->cnt += 1;	// Running count of sequences completed
		p2 = &strADC1dr.in[1][0][0];
		DMA1_IFCR = DMA_IFCR_CTCIF1;	// Clear transfer complete flag (p 208)
	}
	if ( (DMA1_ISR & DMA_ISR_HTIF1) != 0 )	// Is this a half transfer complete interrupt?
	{ // Here, yes.  The first sequence has been converted and stored in the first buffer
//		strADC1resultptr->flg  = 0;	// Set index to the first buffer.  It is ready.
//		strADC1resultptr->cnt += 1;	// Running count of sequences completed
		p2 = &strADC1dr.in[0][0][0];
		DMA1_IFCR = DMA_IFCR_CHTIF1;	// Clear transfer complete flag (p 208)
	}
dbgX += 1;
if (dbgX >= DBGX)
{
dbgX = 0;
adcsensorDebugdmact += 1;
	/* Copy 1/2 buffer just filled to a circular buffer */
	for (i = 0; i < NUMBERSEQUENCES; i++)
	{
		*(p1+0) = *(p2+0); // Fast in-line copy
		*(p1+1) = *(p2+1);
		*(p1+2) = *(p2+2);
		p1        += NUMBERADCCHANNELS_SE; // Advance by one sequence
		p2        += NUMBERADCCHANNELS_SE;
	}
	/* Advance index in circular buffer by number of sequences added */
	adcidx_in += (NUMBERSEQUENCES * NUMBERADCCHANNELS_SE);
	if (adcidx_in >= (ADCRAWBUFFSIZE*NUMBERADCCHANNELS_SE)) adcidx_in = 0;//-= ADCRAWBUFFSIZE*NUMBERADCCHANNELS_SE;
}
	/* Trigger a pending interrupt that will handle filter the ADC readings. */
//	NVICISPR(NVIC_FSMC_IRQ);	// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')


	return;
}
/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA -> FSMC  (low priority)

void FSMC_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (dma_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma_ll_ptr)();	// Go do something
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below is CIC Filtering of DMA stored ADC readings 
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* **** CIC filtering stuff **** */
#include "cic_filter_l_N2_M3.h"

long	adc_last_filtered[NUMBERADCCHANNELS_SE];	// Last computed & filtered value for each channel
uint8_t    adc_cic_flag[NUMBERADCCHANNELS_SE];	// 1 = cic filtering complete, 0 = not ready
long	adc_temperature;		// Thermistor filter/decimate to 2/sec
int	adc_temp_flag;			// Signal main new filtered reading ready
int	adc_calib_temp;			// Temperature in deg C 

static struct CICLN2M3 adc_cic[NUMBERADCCHANNELS_SE];	// CIC intermediate storage adc readings

struct RUNNINGADCAVERAGE radcave[NUMBERADCCHANNELS_SE];	// Running averages

void adc_cic2_init(struct CICLN2M3* p);	// Declaration
/******************************************************************************
 * static void adc_cic_init(u32 iamunitnumber);
 * @brief 	: Setup for cic filtering of adc readings buffered by dma
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
*******************************************************************************/
/*
struct CICLN2M3
{
	unsigned short	usDecimateNum;	// Downsampling number
	unsigned short	usDiscard;	// Initial discard count
	int		nIn;		// New reading to be filtered
	long 		lIntegral[3];	// Three stages of Z^(-1) integrators
	long		lDiff[3][2];	// Three stages of Z^(-2) delay storage
	long		lout;		// Filtered/decimated data output
	unsigned short	usDecimateCt;	// Downsampling counter
	unsigned short	usFlag;		// Filtered/decimated data ready counter
};
*/

static void adc_cic_init(void)
{
	int i;

	/* Initialize the structs that hold the CIC filtering intermediate values. */
	strADC1dr.cnt = 0;
	strADC1dr.flg = 0;
	for (i = 0; i < NUMBERADCCHANNELS_SE; i++)	
	{
		
		/* Running average init */
		radcave[i].accum = 0;	// Zero running average accumulators
		radcave[i].pend  = &radcave[i].old[RAVESIZE];
		radcave[i].p     = &radcave[i].old[0];
		radcave[i].pbegin= radcave[i].p;

		/* Init cic filters with constants from #defines in .h file */
		adc_cic2_init(&adc_cic[i]);
	}
	return;
}
/* **************************************************************
 * void adc_cic2_init(struct CICLN2M3* p);
 * @brief	: init struct for cic filtering with constants & zeroing
 * @param	: p = pointer to cic struct
*****************************************************************/
void adc_cic2_init(struct CICLN2M3* p)
{
	int j;
	p->usDecimateNum = DECIMATION_SE; // Decimation number
	p->usDecimateCt = 0;		// Decimation counter
	p->usDiscard = DISCARD_SE;	// Initial discard count
	p->usFlag = 0;			// 1/2 buffer flag
	for (j = 0; j < 3; j++)
	{ // Very important that the integrators begin with zero.
		p->lIntegral[j] = 0;
		p->lDiff[j][0] = 0;
		p->lDiff[j][1] = 0;
	}	
	return;
}

/* ########################## UNDER LOW PRIORITY INTERRUPT Triggered by ADC/DMA ##################
 * Run the latest adc readings through the cic filter.
 * ############################################################################################### */
unsigned int cicdebug0,cicdebug1;

u32 cic_sync_err[NUMBERADCCHANNELS_SE];	// CIC sync errors

/* Low priority post SYSTICK interrupt call, 2048 per sec.,
   executes "rpmsensor_computerpm()", which enters this routine.  */
void adc_cic_filtering(void)
{
	int i;
	int *p2;

//while (adcidx_out != adcidx_in) {cicdebug1 += 1; adcidx_out += 3;if (adcidx_out >= (ADCRAWBUFFSIZE * NUMBERADCCHANNELS_SE)) adcidx_out = 0;}
//return;


	/* Run accumulated data in circular buffer through cic filter */
	while (adcidx_out != adcidx_in)
	{
cicdebug1 += 1;
		p2 = &adcrawbuff[adcidx_out];
		adcidx_out += NUMBERADCCHANNELS_SE;
		if (adcidx_out >= ADCRAWBUFFSIZE * NUMBERADCCHANNELS_SE) adcidx_out = 0;

		/* Add the latest buffered sequence of readings to the cic filter */
		for (i = 0; i < NUMBERADCCHANNELS_SE; i++)	
		{
			adc_cic[i].nIn = *p2++; 	// Load reading to filter struct
			if (cic_filter_l_N2_M3 (&adc_cic[i]) != 0) // Filtering complete?
			{ // Here, yes.
				adc_last_filtered[i] = adc_cic[i].lout >> CICSCALE; // Scale and save
				adc_cic_flag[i] = 1;
				
				/* Running average */
				radcave[i].accum += adc_last_filtered[i];	// Add new reading
				radcave[i].accum -= *radcave[i].p;			// Sub old reading
				*radcave[i].p     = adc_last_filtered[i];	// Save new reading
				radcave[i].p += 1;
				if (radcave[i].p >= radcave[i].pend) radcave[i].p = radcave[i].pbegin;

cicdebug0 += 1;
			}
 		}
	}
	return;
}
/* ############################################################################################### */
void adc_cic_filtering2(struct COMMONFUNCTION* p)
{
	p->cic2.nIn = p->ilast1; 	// Load reading to filter struct
	if (cic_filter_l_N2_M3 (&p->cic2) != 0) // Filtering complete?
	{ // Here, yes.
		p->ilast2 = p->cic2.lout >> CICSCALE; // Scale and save
		p->cic2.usFlag = 1;
	}
	return;
}

/* **************************************************************************************
 * static void canprep(struct CANRCVBUF* pcan, float f, uint8_t status);
 *	@brief	: Setup payload in bytes [1]-[4]
 * @param	: pcan = pointer to can msg buffer
 * @param	: f = float pulled into bytes
 * ************************************************************************************** */
static void canprep(struct CANRCVBUF* pcan, uint8_t status, float f)
{
	union ITOF
	{
		uint8_t b[4];
		float f;
	} uf;
	uf.f = f;
	pcan->cd.uc[0] = status;
	pcan->cd.uc[1] = uf.b[0]; 
	pcan->cd.uc[2] = uf.b[1];
	pcan->cd.uc[3] = uf.b[2];
	pcan->cd.uc[4] = uf.b[3];
	return;
}
/* ################## UNDER LOW PRIORITY originating with TIM4 CH3, rpmsensor.c ######## */
// Entered every subinterval OC (62500/64E6 -> ~976 us)
// cic with decimation = 16 is about 1 ms.

void adcsensor_reading(uint32_t subinterval_ct)
{
	int32_t del;
	int32_t rng;	
	int32_t pctscaled;

	/* Run accumulated data through cic filtering & decimation and averaging */
	adc_cic_filtering();

	if (subinterval_ct >= SUBINTERVALTRIGGER) // Time to prep a CAN msg?
	{ // Here, yes

		/* Separate ADC sequence data into functions */
		ethr_f.cf.ilast1 = radcave[0].accum/RAVESIZE; // Throttle
		 et1_f.cf.ilast1 = radcave[1].accum/RAVESIZE; // Thermistor
		eman_f.cf.ilast1 = radcave[2].accum/RAVESIZE; // Manifold pressure

		/* Thermistor->temperature is taken care at mainline */
		adc_cic_filtering2(&et1_f.cf);	// Filter & decimate for console & hb

		/* Throttle */
		del = ethr_f.cf.ilast1 - ethr_f.lc.throttle_close;
		rng = ethr_f.lc.throttle_open - ethr_f.lc.throttle_close;
		pctscaled = (del * (100 * 1024))/rng; // Scaled
		ethr_f.cf.flast1 = pctscaled;	// Convert to float for CAN msg
		ethr_f.cf.flast1 = ethr_f.cf.flast1/1024; // Scale back to pct
		canprep(&ethr_f.cf.can_msg,ethr_f.cf.status,ethr_f.cf.flast1);	// CAN msg setup
		adc_cic_filtering2(&ethr_f.cf);	// Filter & decimate for console & hb

		/* Manifold pressure */
		eman_f.dlast1      = eman_f.cf.ilast1;
		eman_f.dcalibrated = (eman_f.dlast1 * eman_f.dpress_scale) + eman_f.dpress_offset;
		eman_f.cf.flast1   = eman_f.dcalibrated; // Convert to float for CAN msg
		canprep(&eman_f.cf.can_msg,eman_f.cf.status,eman_f.cf.flast1);	// CAN msg setup
		adc_cic_filtering2(&eman_f.cf);	// Filter & decimate for console & hb
		
	}

	return;	
}
