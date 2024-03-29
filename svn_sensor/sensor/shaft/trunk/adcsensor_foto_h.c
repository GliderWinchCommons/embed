/******************************************************************************
* File Name          : adcsensor_foto_h.c
* Date First Issued  : 05/07/2018
* Board              : RxT6
* Description        : ADC routines for f103 sensor--sensor board histogram collection
*******************************************************************************/
/*
02-07-2014 rev 370: poll-response scheme.  Error msg sending disabled.  Needs PC to retry if hangs.

02-05-2014: Hack of adcsensor_foto.c to include building a histogram of ADC readings
and providing a provision for a request|response readout of these readings via the
CAN bus.  See comments in 'int adc_histo_cansend(struct CANRCVBUF* p);' below for more
detail.

To store the readings DMA is required and with DMA enabled ADC1 data register has ADC2
readings stored in the high half of the ADC1 data register.  This renders the ADC1
watchdog inoperable.  The workaround is to use ADC1 DMA to store ADC2 readings and
ADC3 with its own DMA to store ADC3 readings.  ADC3 watchdog function replaces ADC1
of the early version.
*/
/*
07-13-2013: earlier rev 206 works with 64/sec compute rate.  Major changes started
    to cic filter rpm, with cic input at a higher than 64 rate.  Works except for
    some glitch when the rpm hovers around the no encoder counts to one count within
    one 1/64th sec frame.

04/27/2018: Revisons when updating se4_h->shaft programs to use database parameter
    scheme.
*/

/* 
07/08/2013 This is a hack of svn_pod/sw_stm32/trunk/devices/adcsensor_pod.c
See p214 of Ref Manual for ADC section 

ADC3 and ADC2 are used for photocell A and B, respectively, emitter measurements.
The photocell transmit diode is driven by the 3.3v analog voltage via a 150 ohm resistor.
The photocell photodetector transistor emitter has a 470 ohm resistor to ground.  The 
voltage across the 470 ohm resistor is measured by the ADC.

Each ADC "scans" just one channel continuously.  The high and low threshold watchdog
registers are used to trigger an interrupt.  When the converted signal value is greater than
the high threshold register (HTR), or is less than the low threshold register (LTR), the
watddog flag is set, and when the watchdog interrupt is enabled an interrupt is triggered.

Each conversion that results in a value greater than the HTR, or less than the LTR, will
set the interrupt flag.

ADC1 and ADC2 have their interrupts mapped together.  Using DMA to store values causes the
ADC1 data register to be loaded with ADC2 in the upper half of the 32b register which means
the ADC1 watchdog register can never be higher, unless ADC2 reading happens to be zero.
Therefore, ADC3 is used with it's independent DMA channel and ADC1 DMA is used to store
ADC2 readings.

The interrupt routine checks the watchdog flag in the status register for each ADC and 
for the ADC that is set the data register is compared to the high threshold register 
to see if the interrupt was a measurement that was going high, or going low, thereby 
determining the 0|1 status of that photocell.

The interrupts are caused when the measurement completes and the reading is above the high
register threshold, or below the low register threshold.  To prevent interrupts after every
ADC conversion the high register threshold is raised to the the max so that no further interrupts 
can occur, and the low register is set with the low threshold.  When the signal drops and 
drops below the low register threshold the interrupt servicing sets the low register at zero 
so that it cannot cause further interrupts, and the high level is set at the high threshold.

The 0|1 levels of the two photocells is used to provide quadrature detection and counting
of the shaft.  The count is 4 counts for a segment.  The quadrature encoding is done using a
table lookup that is based on the new state and the previous ("old") state.  ADC1 and ADC2
provide 1 bit each, so the new state can be 00, 01, 11, 10.  When combined with the old state
there are 16 possibilities.  The table lookup gives +1, -1, and zero.  Zero is an illegal
change, e.g. 00 to 11 has both changing state at the same time.  The +/- 1 gives a signed
count to the shaft position.

NOTE:  There is an issue with regard to the ADC measurement rate being short enough that by the
time the interrupt service routine is serviced the next ADC measurement might have completed.  To
prevent multiple counts, the threshold register is first set to an impossible value, then the quadrature
logic executed, which allows time for the register value to be set in the peripheral.  If a 2nd completion 
occurred before the threshold register was set to an impossible value (and the measurement was
above|below the threshold register) the interrupt request would not change an already set interrupt
flag.  The last action in the interrupt service is to reset the interrupt flag, and at this point
in time the new, impossible threshold register value would prevent a 2nd interrupt request.  If the
foregoing sequence was not done, e.g. the interrupt flag reset at the beginning of the interrupt
service routine, it would be make possible a double count if the exact timing circumstances would
occur.

11/12/2012 -- Initial code for ADC1|2 approach.
11/16/2012 -- CAW & DEH Skype through the code

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

 p 219
11.3.7 Analog watchdog
       The AWD analog watchdog status bit is set if the analog voltage converted by the ADC is
       below a low threshold or above a high threshold. These thresholds are programmed in the
       12 least significant bits of the ADC_HTR and ADC_LTR 16-bit registers. An interrupt can be
       enabled by using the AWDIE bit in the ADC_CR1 register.
       The threshold value is independent of the alignment selected by the ALIGN bit in the
       ADC_CR2 register. The comparison is done before the alignment (see Section 11.5).
       The analog watchdog can be enabled on one or more channels by configuring the
       ADC_CR1 register as shown in Table 69.
*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "libusartstm32/commonbitband.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "common_can.h"
#include "common_misc.h"

#include "pinconfig_all.h"
#include "adcsensor_foto.h"
#include "../../../../svn_common/trunk/can_driver.h"
#include "SENSORpinconfig.h"

#include "libopenstm32/scb.h"

#include "../../../../svn_common/trunk/pay_type_cnvt.h"
#include "CANascii.h"
#include "../../../../svn_common/trunk/can_gps_phasing.h"
#include "adcsensor_foto_h.h"
#include "tim4_shaft.h"
#include "IRQ_priority_shaft.h"
#include "iir_filter_l.h"
#include "db/gen_db.h"
#include "libmiscstm32/DTW_counter.h"



/* Pointer to control block for CAN1. */
extern struct CAN_CTLBLOCK* pctl1;

uint32_t adcsensor_nrpmhb;	// Latest nrpm buffered for 'main'
uint8_t iirflag = 0;	// Flag 'main' for new rpm data in buffer

/* Delay for CR2_ADON (adc power up) (about 1 us) */
#define DELAYCOUNTCR2_ADON	2	// Delay in microseconds

/* ADC usage (only Encoder A & B used)

PA0 ADC123-IN0	Encoder A (Throttle potentiometer)
PA1 ADC123-IN1  Encoder B (Thermistor)
PA3 ADC123-IN2	Pressure sensor  

*/
/*
Bits 29:0 SMPx[2:0]: Channel x Sample time selection (1)
           These bits are written by software to select the sample time individually for each channel.
           During sample cycles channel selection bits must remain unchanged.
           000: 1.5 cycles	@14MHz  0.4K max input R (p 123 datasheet)
           001: 7.5 cycles	@14MHz  5.9K max input R  ### (3.3K in prototype 11/15/2012)
           010: 13.5 cycles	@14MHz 11.4K max input R
           011: 28.5 cycles	@14MHz 25.2K max input R
           100: 41.5 cycles	@14MHz 37.2K max input R
           101: 55.5 cycles	@14MHz 50.0K max input R
           110: 71.5 cycles	50K max
           111: 239.5 cycles	50K max
(1) For less than 1/4 LSB of 12 bits conversion

With an ADC clock of 12MHz the total conversion time--
12.5 + SMPx time = 12.5 + 7.5 = 20 adc clock cycles
At 12 MHz, 20 cycles * 83 1/2 ns per adc clock cycle = 1 2/3 us per conversion
*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP1	1	// ### Photocell emitter A
#define SMP2	1	// ### Photocell emitter B
#define SMP3	1
#define SMP4	1
#define SMP5	1
#define SMP6	3
#define SMP7	3
#define SMP8	6
#define SMP9	6
#define SMP10	 1	
#define SMP11	 1		
#define SMP12	3
#define SMP13	3
#define SMP14	3
#define SMP15	3
#define SMP16	7
#define SMP17	7


/* ********** Static routines **************************************************/
static void adc_init_foto(void);
static void adc_start_conversion_foto(void);
static void adc_start_cal_register_reset_foto(void);
static void adc_start_calibration_foto(void);
static void compute_init(struct SHAFT_FUNCTION* p);
static void adc_histo_build_ADC12(void);
static void adc_histo_build_ADC3(void);

/* Error counter array */
u32 adcsensor_foto_err[ADCERRORCTRSIZE];
	// [0] =  Error count: shaft encoding illegal transitions
	// [1] =  Error count: for illogical tick durations (re: the systick interrupt priority caper.)
	// [2] =  Error count: for illogical tick durations (re: the systick interrupt priority caper.)
	// [3] =  Error count: cic out of sync with end of 1/64th sec


/* APB2 bus frequency is needed to set ADC prescalar so that it is less than 14 MHz,
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* High and low watchdog register threshold values */
static unsigned short usHtr1;// = ADC3_HTR_INITIAL;	// ADC1 High register initial value
static unsigned short usLtr1;// = ADC3_LTR_INITIAL;	// ADC1 Low  register initial valueit
static unsigned short usHtr2;// = ADC2_HTR_INITIAL;	// ADC2 High register initial value
static unsigned short usLtr2;// = ADC2_LTR_INITIAL;	// ADC2 Low  register initial value

/* DMA circular buffered with 1/2 way interrupt double buffers ADC1,2 pairs. */

 //  DMA1 CH1 32b elements [pairs of 16b ADC readings with ADC1, ADC2]
 union ADC12VAL adc12valbuff[2][ADCVALBUFFSIZE];
static volatile int adc12valbuffflag = 0;		// 0 = 1st half; 1 = 2nd half

 //  DMA2 CH5 buffer [16b ADC readings with ADC3]
unsigned short adc3valbuff[2][ADCVALBUFFSIZE];
static volatile int adc3valbuffflag = 0;	// 0 = 1st half; 1 = 2nd half

/* Pointer into histogram bins, used for readout. */
u32* can_msg_histo_ptr = 0;
#ifdef HISTOGRAM_MAY_HAVE_SOME_USEFUL_CODE_USED_LATER
static u32 histthrottle = 0;	// Throttle for readouts of bins
#endif

/* (ADCTIMSCALE/time_diff_between_ADC_transition) */
#define ADCTIMSCALEBITS 32
#define ADCTIMSCALE (1 << ADCTIMSCALEBITS)	// Scale 
#define ADCTRANSITIONPERSEG 4	// Transition per encoding wheel segment

/* Circular buffer for DWT times of ADC transitions */
#define ADCTIMBUFSIZE 256	// Circular buffer size

/* Buffer and parameters for photocells */
struct ADCTIMBUF
{
	uint32_t* padd;   // Buffer position to be added
	uint32_t* psub;	// Position to be removed
	uint32_t* plast;  // Latest/last added transition time
	uint32_t* p_begin;// Beginning of buffer
	uint32_t* p_end;	// End of buffer
	uint32_t  rct;		// Running count
	 int32_t  n;		// Number in average
	uint32_t  zeroct;	// Consecutive intervals with no transitions
	uint32_t  tdiff;	// System ticks time diff between transitions
	uint32_t  tlast;	// Last time reading
	int32_t   irecip;	// (n/delta_t)
   uint32_t  buf[ADCTIMBUFSIZE]; // 32b sys counter times
};

/* Circular buffers for "up" and "down" transitions */
static struct ADCTIMBUF adctim2[2]; // Photocell B up:down
static struct ADCTIMBUF adctim3[2]; // Photocell A up:down

/* Histogram bin counts plus related */
struct ADCHISTO adchisto2; // ADC2
struct ADCHISTO adchisto3; // ADC3

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
 * static void adctimbuf_init(struct ADCTIMBUF* p);
 * @brief 	: Initialize struct for circular buffering ADC transisiton times
 * @param	: p = pointer to wonderful things
*******************************************************************************/
static void adctimbuf_init(struct ADCTIMBUF* p)
{
	p->padd    = &p->buf[0]; // Add to buffer pointer
	p->psub    = p->padd;    // Take from buffer pointer
	p->p_begin = p->psub;	 // Beginning of buffer
	p->p_end   = &p->buf[ADCTIMBUFSIZE]; // End of buffer
	p->plast   = p->p_end;
	p->rct     = 0; // Running count
	p->n       = 0;
	p->zeroct  = 0;
	*p->psub   = *(volatile unsigned int *)0xE0001004; // Current sys counter time
	return;
}
/******************************************************************************
 * void adc_init_sequence_foto_h(struct SHAFT_FUNCTION* p);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: p = pointer to sram of variables & parameters for this function instance
*******************************************************************************/
void adc_init_sequence_foto_h(struct SHAFT_FUNCTION* p)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Set ADC threshold register parameters from local copy parameter area. */
	usHtr1 = p->lc.adc3_htr_initial; // High threshold register setting, ADC3
	usLtr1 = p->lc.adc3_ltr_initial; // Low  threshold register setting, ADC3
	usHtr2 = p->lc.adc3_htr_initial; // High threshold register setting, ADC2
	usLtr2 = p->lc.adc3_ltr_initial; // Low  threshold register setting, ADC2

	/* Set up for computations and CAN messaging shaft count and speed */
	compute_init(p);

	/* Init circular buffers that store sys tick time of photocell transitions */
	adctimbuf_init(&adctim2[0]);
	adctimbuf_init(&adctim3[0]);
	adctimbuf_init(&adctim2[1]);
	adctimbuf_init(&adctim3[1]);

	/* Pre-compute scale factor for scaling interger raw rpm to scaled rpm double */
	// Numerator
	shaft_f.dk1  = 60.0;	// RPS -> RPM
	shaft_f.dk1 *= sysclk_freq;	// e.g. 64,000,000
	// Denominator
	shaft_f.dk1 /=(double)(1<<16);       // equivalent of 32b integer scaling
	shaft_f.dk1 /=(double)(1<<16);       //
	shaft_f.dk1 /= 4;                    // four counts per transition
	shaft_f.dk1 /= 4;                    // Sum for four transition streams
	shaft_f.dk1 /=(double)p->lc.num_seg; // e.g. 16 segs per rev

	/* Initialize ADC1 & ADC2 */
	adc_init_foto();			// Initialize ADC1 and ADC2 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adc_start_cal_register_reset_foto();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adc_start_calibration_foto();
	timedelay(80 * ticksperus);	// Wait
	
	/* DMA has been setup, but not started. */
	DMA1_CCR1 |= DMA_CCR1_EN;	// Start stream
	DMA2_CCR5 |= DMA_CCR1_EN;	// Start stream

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion_foto();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adc_init_foto(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
static const struct PINCONFIGALL pin_pa0 = {(volatile u32 *)GPIOA, 0, IN_ANALOG, 0};
static const struct PINCONFIGALL pin_pa1 = {(volatile u32 *)GPIOA, 1, IN_ANALOG, 0};
static const struct PINCONFIGALL pin_pa2 = {(volatile u32 *)GPIOA, 2, IN_ANALOG, 0};

static void adc_init_foto(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/*  Setup ADC pins for ANALOG INPUT (p 148) */
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa0);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa1);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa2);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 3;		// Division by 8	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/6 < 14000000) ucPrescalar = 1;	// Division by 4
	if (pclk2_freq/4 < 14000000) ucPrescalar = 0;	// Division by 2

//ucPrescalar = 3;	// Slow it down for debugging and test

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14)	; // Set code for bus division

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= ( (RCC_APB2ENR_ADC1EN) | (RCC_APB2ENR_ADC2EN) | (RCC_APB2ENR_ADC3EN) );	// Enable ADC1, ADC2, ADC3 clocking
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." 
	AWDEN - enable watchdog register p 229,30. */
	//         (   dual mode    |  scan        | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (     (6 << 16)  | ADC_CR1_SCAN                                   		     );	// ADC1
	ADC2_CR1 = (                  ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE 		 | 1 );	// ADC2 chan 1
	ADC3_CR1 = (                  ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE 		 | 0 );	// ADC3 chan 0

	/*          (  Continuous   | Power ON     | DMA	) */
	ADC1_CR2  = ( ADC_CR2_CONT  | ADC_CR2_ADON | ADC_CR2_DMA); 
	ADC2_CR2  = ( ADC_CR2_CONT  | ADC_CR2_ADON              ); 
	ADC3_CR2  = ( ADC_CR2_CONT  | ADC_CR2_ADON | ADC_CR2_DMA); 

	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions */
	timedelay(2 * ticksperus);	// Wait

	// ---Modified for sensor---
	/* Set sample times  */	
	ADC1_SMPR2 = ( (SMP1 << 0) );
	ADC2_SMPR2 = ( (SMP1 << 0) );
	ADC3_SMPR2 = ( (SMP1 << 0) );

	/* Set high and low threshold registers for both ADCs with initial values */
	ADC3_HTR = usHtr1; //ADC3_HTR_INITIAL;
	ADC3_LTR = usLtr1; //ADC3_LTR_INITIAL;
	ADC2_HTR = usHtr2; //ADC2_HTR_INITIAL;
	ADC2_LTR = usLtr2; //ADC2_LTR_INITIAL;

	/* Setup the number of channels scanned (zero = just one) */
	ADC3_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13
	ADC2_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13
	ADC1_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN11,IN10, for conversions sequences
	ADC3_SQR3 = ( (0 << 0) ); // Conversion sequence number one sensor photocell A emitter measurement
	ADC2_SQR3 = ( (1 << 0) ); // Conversion sequence number one sensor photocell B emitter measurement
	ADC1_SQR3 = ( (2 << 0) ); // Conversion sequence number dummy

	/* Set and enable interrupt controller for ADCs. */
	NVICIPR (NVIC_ADC1_2_IRQ,ADC1_2_PRIORITY_FOTO_SHAFT );// Set priority ADC1/2

	NVICIPR (NVIC_ADC3_IRQ,ADC1_2_PRIORITY_FOTO_SHAFT );	// Set priority  *SAME AS* ADC1|ADC2

	/* Low level interrupt for doing adc filtering following DMA1 CH1 interrupt. */
	// Set low level interrupt priority for post DMA1 interrupt processing
	NVICIPR (NVIC_FSMC_IRQ, ADC_FSMC_PRIORITY_FOTO);	

	/* Low level interrupt for doing adc filtering following DMA2 CH5 interrupt. */
	// Set low level interrupt priority for post DMA2 interrupt processing
	NVICIPR (NVIC_SDIO_IRQ, ADC_SDIO_PRIORITY_FOTO);

	/* Setup DMA1 for storing data in the ADC_DR (with 32v ADC1|ADC2 pairs) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;      // Enable DMA1 clock
	DMA1_CNDTR1 = (2 * ADCVALBUFFSIZE);   // Number of data items before wrap-around
	DMA1_CPAR1 = (u32)&ADC1_DR;           // DMA1 channel 1 peripheral address (adc1|2 data register)
	DMA1_CMAR1 = (u32)&adc12valbuff[0][0];// Memory address of first buffer array for storing data

	// Channel configurion reg (ADC1 & ADC2 store together in 32b)
	//          priority high  | 32b mem xfrs | 32b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x02 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE  | 0 ;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY_SHAFT );	// Set interrupt priority

	/* Setup DMA2 for storing data in the ADC_DR (with 16b ADC3) */
	RCC_AHBENR |= RCC_AHBENR_DMA2EN;      // Enable DMA2 clock
	DMA2_CNDTR5 = (2 * ADCVALBUFFSIZE);   // Number of data items before wrap-around
	DMA2_CPAR5 = (u32)&ADC3_DR;           // DMA2 channel 5 peripheral address (adc3 data register)
	DMA2_CMAR5 = (u32)&adc3valbuff[0][0]; // Memory address of first buffer array for storing data

	// Channel configurion reg (ADC3 stores as 16b) 
	//          priority high  | 16b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA2_CCR5 =  ( 0x02 << 12) | (0x01 << 10) |  (0x01 << 8) | DMA_CCR5_MINC | DMA_CCR5_CIRC |DMA_CCR5_HTIE | DMA_CCR5_TCIE  | 0 ;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA2_CHANNEL4_5_IRQ, DMA1_CH1_PRIORITY_SHAFT );	// Set interrupt priority *SAME AS* DMA1 for ADC1|ADC2
	
	return;
}
/******************************************************************************
 * void adcsensor_foto_h_enable_interrupts(void);
 * @brief 	: do as the name says
*******************************************************************************/
void adcsensor_foto_h_enable_interrupts(void)
{	
	NVICISER(NVIC_ADC1_2_IRQ);			// Enable interrupt ADC1|ADC2 Watchdog
	NVICISER(NVIC_ADC3_IRQ);			// Enable interrupt ADC3 Watchdog
	return;
}
/******************************************************************************
 * void adcsensor_foto_h_disable_histogram(void);
 * @brief 	: Stop DMA interrupts for histogram (storing stills goes on)
*******************************************************************************/
void adcsensor_foto_h_disable_histogram(void)
{	
	/* Clear interrupt enable in NVIC */
	NVICICER(NVIC_DMA1_CHANNEL1_IRQ);  // DMA ADC 3 store reading Photocell A
	NVICICER(NVIC_DMA2_CHANNEL4_5_IRQ);// DMA ADC 1 & 2 store reading Photocell B
	NVICICER(NVIC_FSMC_IRQ);			// Low level interrupt, build histogram
	NVICICER(NVIC_SDIO_IRQ);			// Low level interrupt, build histogram
	return;
}
/******************************************************************************
 * void adcsensor_foto_h_enable_histogram(void);
 * @brief 	: Enable DMA and low level interrupts for histogram
*******************************************************************************/
void adcsensor_foto_h_enable_histogram(void)
{	
	/* Enable interrupts in NVIC */
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);  // DMA ADC 3 store reading Photocell A
	NVICISER(NVIC_DMA2_CHANNEL4_5_IRQ);// DMA ADC 1 & 2 store reading Photocell B
	NVICISER(NVIC_FSMC_IRQ);			// Enable low level interrupt, build histogram
	NVICISER(NVIC_SDIO_IRQ);			// Enable low level interrupt, build histogram
	return;
}
/******************************************************************************
 * static void adc_start_cal_register_reset_foto(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adc_start_cal_register_reset_foto(void)
{
	/* Reset calibration register  */
	ADC3_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	ADC2_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC3_CR2 & ADC_CR2_RSTCAL) != 0);
	while ((ADC2_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adc_start_calibration_foto(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adc_start_calibration_foto(void)
{
	/* Start calibration  */
	ADC2_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset
	ADC3_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC2_CR2 & ADC_CR2_CAL) != 0);
	while ((ADC3_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adc_start_conversion_foto(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adc_start_conversion_foto(void)
{	
/* After the initial write of this bit, subsequent writes start the ADC conversion--(p 241)
"Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application."
should allow a delay of tSTAB between power up and start of conversion. Refer to Figure 27 */
	ADC3_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)
	ADC2_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	return;
}
/*#######################################################################################
 * ISR routine for DMA1 Channel1 reading ADC regular sequence of adc channels
 *####################################################################################### */
/* Timing test
System ticks between interrupt entries: 3634 +/- 3 
*/
uint32_t adcbuild0;
uint32_t adcbuild1;
 int32_t adcbuildmax;
void DMA1CH1_IRQHandler(void)
{
adcbuild1 = DTWTIME;
if ((int)(adcbuild1 - adcbuild0) > adcbuildmax) adcbuildmax = (adcbuild1 - adcbuild0);
adcbuild0 = DTWTIME;
/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two buffers full a transfer complete interrut is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 
*/
	if ( (DMA1_ISR & DMA_ISR_TCIF1) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		adc12valbuffflag  = 1;		// Set the index to the second buffer.  It is ready.
		DMA1_IFCR = DMA_IFCR_CTCIF1;	// Clear transfer complete flag (p 208)
	}
	if ( (DMA1_ISR & DMA_ISR_HTIF1) != 0 )	// Is this a half transfer complete interrupt?
	{ // Here, yes.  The first sequence has been converted and stored in the first buffer
		adc12valbuffflag  = 0;		// Set index to the first buffer.  It is ready.
		DMA1_IFCR = DMA_IFCR_CHTIF1;	// Clear transfer complete flag (p 208)
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_FSMC_IRQ);		// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')

	return;
}
/*#######################################################################################
 * ISR routine for DMA2 Channel5 reading ADC regular sequence of adc channels on ADC3
 *####################################################################################### */
/* Timing test
System ticks between interrupt entries: 5126 +/- 5 
*/

void DMA2CH4_5_IRQHandler(void)
{
/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two buffers full a transfer complete interrut is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 
*/
	if ( (DMA2_ISR & DMA_ISR_TCIF5) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		adc3valbuffflag  = 1;		// Set the index to the second buffer.  It is ready.
		DMA2_IFCR = DMA_IFCR_CTCIF5;	// Clear transfer complete flag (p 208)
	}
	if ( (DMA2_ISR & DMA_ISR_HTIF5) != 0 )	// Is this a half transfer complete interrupt?
	{ // Here, yes.  The first sequence has been converted and stored in the first buffer
		adc3valbuffflag  = 0;		// Set index to the first buffer.  It is ready.
		DMA2_IFCR = DMA_IFCR_CHTIF5;	// Clear transfer complete flag (p 208)
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_SDIO_IRQ);		// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')

	return;
}
/*#######################################################################################
 * ISR routine for ADC 1 & ADC2 (they are mapped together by ST to make life difficult)
 * ADC1 is not used because the DMA stores ADC2 in the high half of the ADC1 DR making the
 * watchdog inoperable.  Therefore, Foto A uses ADC3 with DMA5CH5, and Foto B uses ADC2, and ADC1 
 * with DMA1CH1 to store ADC2.
 *####################################################################################### */
static unsigned short new;	// Latest 0|1 pair--bit 0 = ADC1 (photocell A); bit 1 = ADC2 (photocell B);
static unsigned short old;	// Saved "new" from last interrupt.

/* Values generated in interrupt routine */
s32 encoder_ctr2;       // Shaft encoder running count (+/-)
s32 encoder_ctr2_prev;  // Previous

/* Table lookup for shaft encoding count: -1 = negative direction count, +1 = positive direction count, 0 = error */
/* One sequence of 00 01 11 10 results in four counts. */
static const signed char en_state[16] = {0,+1,-1, 0,-1, 0, 0,+1,+1, 0, 0,-1, 0,-1,+1, 0};

void ADC1_2_IRQHandler2(void)
{ // Here, interrupt of ADC12 (Photocell B)

	__attribute__((__unused__))int temp; // Register readback to avoid IRQ tail-chaining

	int both;	// Table lookup index = ((old << 2) | new)

	struct ADCTIMBUF *p;

	/* ADC2, IN1, Photocell B  */	
	if ((ADC2_SR & ADC_SR_AWD) != 0)	// ADC2 watchdog interrupt?
	{ // Here, yes.  ADC Watchdog flag is on

		/* Is the data *above* the high threshold register? */
		if (ADC2_DR > ADC2_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC2_HTR = 0x0fff;  // Change high register to max and impossible to exceed
			ADC2_LTR = usLtr2;  // Bring low register up to threshold
			new |= 0x02;        // Turn 0|1 status bit ON
			LED19RED_off;
			/* Save time of shaft encoding transitions (for speed computation) */
			p = &adctim2[0];	// UP transition
			*p->padd = *(volatile unsigned int *)0xE0001004; // Save sys time (DWT_CYCNT)
			p->padd += 1; if (p->padd >= p->p_end) p->padd = p->p_begin;
			p->rct += 1;	// Cumulative counter--either direction
		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC2_LTR = 0;      // Set low register to impossible
			ADC2_HTR = usHtr2; // Set high register to threshold
			new &= ~0x02;      // Turn 0|1 status bit OFF
			LED19RED_on;
			/* Save time of shaft encoding transitions (for speed computation) */
			p = &adctim2[1];	// Down transition
			*p->padd = *(volatile unsigned int *)0xE0001004; // Save sys time (DWT_CYCNT)
			p->padd += 1; if (p->padd >= p->p_end) p->padd = p->p_begin;		
			p->rct += 1;	// Cumulative counter--either direction
		}
		both = (old << 2) | new;        // Make lookup index
		encoder_ctr2 += en_state[both]; // Add +1, -1, or 0
		if (en_state[both] == 0)        // Was this an error
			adcsensor_foto_err[0] += 1;  // Cumulative count

		old = new;                 // Update for next 
		temp = ADC2_HTR | ADC2_LTR;// Be sure new threshold values are set before turning off interrupt flag
		ADC2_SR = ~0x1;            // Reset adc watchdog flag
		temp = ADC2_SR;            // Readback assures interrupt flag is off (prevent tail-chaining)
	}
	return;
}
/*#######################################################################################
 * ISR routine for ADC3 (Photocell A)
 *####################################################################################### */
void ADC3_IRQHandler(void)
{ 
	__attribute__((__unused__))int temp; // Register readback to avoid IRQ tail-chaining

	int both;	// Table lookup index = ((old << 2) | new)

	struct ADCTIMBUF *p;

	/* ADC3, IN0, Photocell A  */
	if ((ADC3_SR & ADC_SR_AWD) != 0)	// ADC1 watchdog interrupt?
	{ // Here, yes.  ADC Watchdog flag is on

		/* Is the data *above* the high threshold register? */
		if (ADC3_DR > ADC3_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC3_HTR = 0x0fff;   // Change high register to max and impossible to exceed
			ADC3_LTR = usLtr1;   // Bring low register up to threshold
			new |= 0x01;         // Turn 0|1 status bit ON
			LED20RED_off;
			p = &adctim3[0];	// Up transition
			*p->padd = *(volatile unsigned int *)0xE0001004; // Save sys time (DWT_CYCNT)
			p->padd += 1; if (p->padd >= p->p_end) p->padd = p->p_begin;		
			p->rct += 1;	// Cumulative counter--either direction
		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC3_LTR = 0;        // Set low register to impossible
			ADC3_HTR = usHtr1;   // Set high register to threshold
			new &= ~0x01;        // Turn 0|1 status bit OFF
			LED20RED_on;
			p = &adctim3[1];	// Down transition
			*p->padd = *(volatile unsigned int *)0xE0001004; // Save sys time (DWT_CYCNT)
			p->padd += 1; if (p->padd >= p->p_end) p->padd = p->p_begin;		
			p->rct += 1;	// Cumulative counter--either direction

		}
		both = (old << 2) | new;        // Make lookup index
		encoder_ctr2 += en_state[both]; // Add +1, -1, or 0
		if (en_state[both] == 0)        // Was this an error
			adcsensor_foto_err[0] += 1;  // Cumulative count

		old = new;					// Update for next
		temp = ADC3_HTR | ADC3_LTR;	// Be sure new threshold values are set before turning off interrupt flag
		ADC3_SR = ~0x1;			// Reset adc watchdog interrupt flag
		temp = ADC3_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below deals with the handling of the encoding done in the foregoing routines
   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
#include "cic_filter_l_N2_M3.h"

/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority3_ptr)(void) = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority3X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th

/******************************************************************************
 * static void  histo_init(struct ADCHISTO* pu, struct SHAFT_FUNCTION* p);
 * @brief 	: Initialize for computations & sending CAN msg
 * @param	: pu = pointer to histogram stuct for ADC3 or ADC12
 * @param	: p = pointer to shaft function with parameters et al.
*******************************************************************************/
/*
	u32  bin[HISTOBINBUFNUM][ADCHISTOSIZE];	// Double buffer bin counts
	u32* pbegin;   // Beginning of bin buffer
	u32* pend;     // End of bin buffer
	u32* ph;       // Working bin buffer ptr
	int  idx_build;     // index: adding buffer
	int  idx_send;      // index: sending buffer
	int  ctr;           // Number of ADC buffers in histogram
	u32  dur;           // Number to accumulate
	int  sw;            // Histogram request switch
struct CANRCVBUF can_cmd_histo;	    // CAN msg: Histogram data
struct CANRCVBUF can_cmd_histo_final;// CAN msg: Signal end of sending
*/

static void  histo_init(struct ADCHISTO* pu, struct SHAFT_FUNCTION* p, uint8_t adc)
{
	/* Preload: Histogram bin count CAN msg with fixed items */
	pu->can_cmd_histo.id       = *p->cf.pcanid_cmd_func_r; // Response ID
	pu->can_cmd_histo.dlc      = PAYSIZE_U8_U8_U8_U32;
	pu->can_cmd_histo.cd.ull   = 0;	// Zero entire payload
	pu->can_cmd_histo.cd.uc[0] = CMD_THISIS_HISTODATA; // Type of CMD msg
	pu->can_cmd_histo.cd.uc[1] = adc; // ADC number

	/* Preload: Final end CAN msg with fixed items */
	pu->can_cmd_histo_final.id       = *p->cf.pcanid_cmd_func_r;
	pu->can_cmd_histo_final.dlc      = PAYSIZE_U8_U8_U8_U32;
	pu->can_cmd_histo_final.cd.ull   = 0;	// Zero entire payload
	pu->can_cmd_histo_final.cd.uc[0] = CMD_THISIS_HISTODATA;
	pu->can_cmd_histo_final.cd.uc[1] = adc; // ADC number
	pu->can_cmd_histo_final.cd.uc[2] = 255; // Bin number signals last msg

	pu->ctr       = 0;
	pu->dur       = 0;
	pu->idx_build = 1; // Head: bin buffer index
	pu->idx_send  = 1; // Tail: bin buffer index
	pu->sw        = 0;
	pu->idx_tmp   = pu->idx_build + 1;

	return;
}

static void compute_init(struct SHAFT_FUNCTION* p)
{
	/* Pre-load CAN messages wiht values from parameters for msgs sent periodically. */
	// CAN id
	p->can_msg_speed.id = p->lc.cid_msg_speed; //  Shaft speed, calibrated, response to poll
	p->can_msg_count.id = p->lc.cid_msg_ct;    //  CAN msg. Drive Shaft--odometer
	p->can_hb_speed.id  = p->lc.cid_hb_speed;  //  CAN hearbeat. Drive Shaft--speed
	p->can_hb_count.id  = p->lc.cid_hb_ct;     //  CAN hearbeat. Drive Shaft--odometer

	// DLC set for status byte and four byte payload of int or float
	p->can_msg_speed.dlc = PAYSIZE_U8_FF;  //  Shaft speed, calibrated, response to poll
	p->can_msg_count.dlc = PAYSIZE_U8_S32; //  CAN msg. Drive Shaft--odometer
	p->can_hb_speed.dlc  = PAYSIZE_U8_FF;  //  CAN hearbeat. Drive Shaft--speed
	p->can_hb_count.dlc  = PAYSIZE_U8_S32; //  CAN hearbeat. Drive Shaft--odometer

	// Histogram initializations
	histo_init(&adchisto2, p, 2);
	histo_init(&adchisto3, p, 3);

	return;
}
/* **********************************************************************************
// Send the msgs once and use cangate to see if they are correct.
 ************************************************************************************/
void testfoto(void)
{
}
/* ######################### tim4_tim subinterval in############################### 
 * Enter from EXI0 moderate level interrupt each interval
 * ############################################################################################### */
/* *************************************************************************************************
 * static void adcrpmave(struct ADCTIMBUF *p);
 *	@brief	: Compute and accumulate reciprocals of times between ADC transitions for each photocell
 * @param	: p = pointer to photocell struct with buffer, pointers and counts
 * @return	: Speed (n/delta_t)
 **************************************************************************************************/	
#define TDIFFMAX (ADCTIMSCALE * 256)	// Prevent 'accum' overflow: Limit sys ticks between ADC transitions
#define ZEROCTMAX 32	// Number of cases of zero ADC transitions between 1/64th sec intervals

/* Direction of rotation to be applied to rpm/speed. */
static int32_t rpmsign = 1; // Direction: +1, -1
/*
struct ADCTIMBUF
{
	uint32_t* padd;   // Buffer position to be added
	uint32_t* psub;	// Position to be removed
	uint32_t* p_begin;// Beginning of buffer
	uint32_t* p_end;	// End of buffer
	uint32_t* plast;  // Latest/last added transition time
	uint32_t  rct;		// Running count
	uint32_t  n;		// Number in average
	uint32_t  zeroct;	// Consecutive intervals with no transitions
	uint32_t  tdiff;	// System ticks time diff between transitions
	int32_t   irecip;	// (n/delta_t)
   uint32_t  buf[ADCTIMBUFSIZE]; // 32b sys counter times
};
*/
//#define BONEHEADRPMCOMPUTATION
#ifndef BONEHEADRPMCOMPUTATION
static uint32_t adcrpmave(struct ADCTIMBUF *p)
{
	union U3264
	{
		uint64_t r64;
		uint32_t r32[2];
	}u3264;

	/* Compute number of transitions since last time */
	// Difference between pointers in circular buffer
	p->n = ((uint32_t)p->padd - (uint32_t)p->psub) - 4;
	if (p->n < 0)	p->n += ADCTIMBUFSIZE * sizeof(uint32_t);

	// Note: p->n is 4x the number of transitions 
	if (p->n > 0)
	{ // Here, there are transitions

		// Back up a ptr 
		p->plast = p->padd - 1;
		if (p->plast < p->p_begin) p->plast += ADCTIMBUFSIZE;
		p->tdiff = *(p->plast) - *(p->psub); // Time duration between

		// Speed = number_transitions / time_duration
		// Scale upwards by 32b, by putting count in upper word
		u3264.r32[0] = 0;
		u3264.r32[1] = p->n;
		p->irecip = u3264.r64 / p->tdiff; // long_long/long

		p->psub = p->plast;  // Update "tail" ptr
		p->zeroct = 0;       // Reset no-transition counter
	}		
	else
	{ // Here, no transitions during the inteval
		p->zeroct += 1;     // Count number of consecutive zero cases
		if (p->zeroct >= ZEROCTMAX) // Too many?
		{ // Here, the RPM will assumed to be zero.
			p->zeroct = ZEROCTMAX;
			p->irecip = 0;	
		}
	}
	return p->irecip;
}
#endif
#ifdef BONEHEADRPMCOMPUTATION
// Bonehead approach as a double check on foregoing
static uint32_t adcrpmave(struct ADCTIMBUF *p)
{
	uint32_t t = p->tlast;
	union U3264
	{
		uint64_t r64;
		uint32_t r32[2];
	}u3264;

	p->n = 0;

	while (p->padd != p->psub)
	{
		t = *p->psub;
		p->psub += 1;
		if (p->psub >= p->p_end) p->psub = p->p_begin;
		p->n += 4;	// 4x the count to correspond to the above version
	}
	p->tdiff = (int)((int)t - (int)p->tlast);
	if (p->tdiff > 0)
	{
		p->zeroct = 0;       // Reset no-transition counter
		u3264.r32[0] = 0;
		u3264.r32[1] = p->n;
		p->irecip = u3264.r64 / p->tdiff;
	}
	else
	{ // Here, no transitions during the inteval
		p->zeroct += 1;     // Count number of consecutive zero cases
		if (p->zeroct >= ZEROCTMAX) // Too many?
		{ // Here, the RPM will assumed to be zero.
			p->zeroct = ZEROCTMAX;
			p->irecip = 0;	
		}
	}
	p->tlast = t;
	return p->irecip;
}
#endif
/* *************************************************************************************************
 * static void load_payload_flt(struct CANRCVBUF* pcan, uint8_t status, float fpay);
 *	@brief	: Assemble payload from input of status byte, and a float
 *	@brief	: Assemble payload from input of status byte, and a 32b float
 * @param	: pcan = pointer to CAN msg to loaded
 * @param	: status = status byte for reading
 * @param	: uipay = unsigned int with payload
 **************************************************************************************************/
static void load_payload_fit(struct CANRCVBUF* pcan, uint8_t status, float fpay)
{
	union FUI // Float to byte array conversion
	{
		float f;
		uint8_t uc[4];
	}fui;
	fui.f = fpay;	
	
	pcan->cd.uc[0] = status;
	pcan->cd.uc[1] = fui.uc[0];
	pcan->cd.uc[2] = fui.uc[1];
	pcan->cd.uc[3] = fui.uc[2];
	pcan->cd.uc[4] = fui.uc[3];
	return;
}
/* *************************************************************************************************
 * static void load_payload_int(struct CANRCVBUF* pcan, uint8_t status, uint32_t uipay);
 *	@brief	: Assemble payload from input of status byte, and a unsigned int
 * @param	: pcan = pointer to CAN msg to loaded
 * @param	: status = status byte for reading
 * @param	: uipay = unsigned int with payload
 **************************************************************************************************/
static void load_payload_int(struct CANRCVBUF* pcan, uint8_t status, uint32_t uipay)
{
	union UIB  // uint32_t to byte array cnversion
	{
		uint32_t ui;
		uint8_t uc[4];
	}uib;
	uib.ui = uipay;	
	
	pcan->cd.uc[0] = status;
	pcan->cd.uc[1] = uib.uc[0];
	pcan->cd.uc[2] = uib.uc[1];
	pcan->cd.uc[3] = uib.uc[2];
	pcan->cd.uc[4] = uib.uc[3];
	return;
}
/* *************************************************************************************************
 * void adcsensor_load_pay_hb_speed(float f);
 *	@brief	: Assemble payload from input of status byte, and a unsigned int
 * @param	: f = float to be loaded into CAN msg paylaod
 **************************************************************************************************/
void adcsensor_load_pay_hb_speed(float f)
{
	load_payload_fit(&shaft_f.can_hb_speed , shaft_f.status_speed, f);
	return;
}
/* *************************************************************************************************
 * void adcsensor_rpm_compute(void);
 *	@brief	: Compute rpm based on differences from last call to this routine
 **************************************************************************************************/

/* NOTE: This routine is entered frm the EXI0 low level interrupt when the tim4_tim routine subinterval
   count reaches the SUBINTERVALTRIGGER (presently 14) count.  The allows a little time for computation
   before the expected CAN msg that polls for rpm. */
int adcsensordb[5];

void adcsensor_rpm_compute(void)
{
	int32_t ndiff;
	int32_t bave;

	/* Compute reciprocals of durations between oldest and latest ADC transitions */
	bave  = adcrpmave(&adctim2[0]); // Up transition	Photocell B
	bave += adcrpmave(&adctim3[0]); // Up transition   Photocell A
	bave += adcrpmave(&adctim2[1]); // Down transition Photocell B
	bave += adcrpmave(&adctim3[1]); // Down transition Photocell A

	/* Discern direction: +/-   */
	ndiff = encoder_ctr2 - encoder_ctr2_prev; // Overall quadrature counter
	encoder_ctr2_prev = encoder_ctr2;
	if (ndiff > 0)	     // Counts increasing?
		rpmsign = +1;	  // Plus direction speed
	else if (ndiff < 0) // Counts decreasing?
		rpmsign = -1;    // Negative speed
		
	/* Complete scaling and conversions for CAN msgs */
	shaft_f.nrpm  = bave * rpmsign; // Apply direction to rpm
	shaft_f.drpm  = shaft_f.nrpm;	  // Convert to double for computations
	shaft_f.drpm *= shaft_f.dk1;	  // Apply scale to yield rev per minute
	shaft_f.frpm  = shaft_f.drpm;	  // Convert to float for CAN msga

	/* Additional filtering for heartbeat (slow) msg */
	adcsensor_nrpmhb = shaft_f.nrpm;	// Save for 'main' for further filtering
	iirflag = 1;	// Signal 'main' new data

	/* Load CAN msgs with these latest readings */
	load_payload_fit(&shaft_f.can_msg_speed, shaft_f.status_speed, shaft_f.frpm);

	load_payload_int(&shaft_f.can_msg_count, shaft_f.status_count, (uint32_t)encoder_ctr2);
	load_payload_int(&shaft_f.can_hb_count , shaft_f.status_count, (uint32_t)encoder_ctr2);

// debug
int k = 1;
adcsensordb[0] = adctim3[k].n;
adcsensordb[1] = adctim3[k].tdiff;
adcsensordb[2] = adctim3[k].tlast;
adcsensordb[3] = bave; //adctim3[k].irecip;
adcsensordb[4] = 1;

//adcsensordb[0] = adctim2[0].irecip;
//adcsensordb[1] = adctim2[1].irecip;
//adcsensordb[2] = adctim3[0].irecip;
//adcsensordb[3] = adctim3[1].irecip;

//adcsensordb[0] = adctim2[0].n;
//adcsensordb[1] = adctim2[1].n;
//adcsensordb[2] = adctim3[0].n;
//adcsensordb[3] = adctim3[1].n;

//adcsensordb[0] = adctim2[0].tdiff;
//adcsensordb[1] = adctim2[1].tdiff;
//adcsensordb[2] = adctim3[0].tdiff;
//adcsensordb[3] = adctim3[1].tdiff;
	return;
}

/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA1CH1 -> FSMC  (low priority)

void FSMC_IRQHandler(void)
{
	adc_histo_build_ADC12();	// Add buffer readings to histogram

	/* Call other routines if an address is set up */
	if (dma_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma_ll_ptr)();	// Go do something
	return;
}
/******************************************************************************
 * static void adc_histo_build_ADC12(void);
 * @brief 	: Add ADC2 readings from DMA1 CH1 buffer to histogram counts
 * WARNING: This is called from low priority level interrupt: FSMC_IRQHandler
*******************************************************************************/
/*  DMA1CH1 stores ADC1 & ADC2 upon each completion of ADC1 as two 16b readings.
When the DMA reaches the 1/2 way point it gives an interrupt and 'adc12valbuffflag' is
set to zero.  When it reaches the end that flag is set to one.  That flag is always
the index into the buffer not being filled.

DMA2CH5 stores ADC3 readings as one 16b reading.

After a DMA interrupt the low priority level interrupt is triggered.  This interrupt
calls the following routine to add to the 'bin' counts to make a histogram.  The
histogram bins are double buffered, and 'adc2histo_build' points to the histogram buffer
that is being built.
*/
/* Execution time test:
 Shaft stopped: 1779 - 1878 (cycles at 64E6 per sec)
 Shaft @ approx 1200 rpm: 2068
One would expect the running situation to be longer as the ADC watchdog interrupts
have a higher priority than the interrupt for this routine.
*/

static void adc_histo_build_ADC12(void)
{
	struct ADCHISTO* p = &adchisto2; // Use ptr for convenience
	u32 tmp;
	int dmaidx;

	// Note: DMA for ADC1/2 is 32b, hence union
	union ADC12VAL* pdma_end;
	union ADC12VAL* pdma;

	/* duration = 0 signals skip processing. */
	if (p->dur == 0) return; 

	/* Skip if we are in overrun situation */
	if (p->idx_tmp == p->idx_send) return;	

	/* Pointers to bin count buffer */
	p->ph   = &p->bin[p->idx_build][0];	// begin
	p->pend = &p->bin[p->idx_build][ADCHISTOSIZE]; // end

	/* Pointer to DMA data (note ADC12 pairing with union) */
	dmaidx   =  adc12valbuffflag; // 'buffflag might change on us!
	pdma     = &adc12valbuff[dmaidx][0];
	pdma_end = &adc12valbuff[dmaidx][ADCVALBUFFSIZE];

	/* Increment histogram bins from DMA stored ADC readings. */
	while (pdma < pdma_end)
	{ // 12b ADC collapsed to 64 bins
//		tmp = (pdma->us[1] >> ADCHISTOSIZEN)&63;	// ADC2 into 64 ranges
		tmp = pdma->us[1];
		tmp = tmp / 64;
		*(p->ph + tmp) += 1;		// Increment bin(n) count
		pdma ++;
	}

	/* Count number of DMA buffers used in the histogram. */
	p->ctr += 1;
	if (p->ctr >= p->dur) // Completion?
	{ // Here, designated number of DMA buffers have been accumulated
		p->ctr = 0;
		p->idx_build = p->idx_tmp;
		p->idx_tmp += 1; if (p->idx_tmp >= HISTOBINBUFNUM) p->idx_tmp = 0;
		p->sw -= 1;		    // Count down the number of consectuve histograms
		if (p->sw <= 0) // End of histogramming?    
			p->dur = 0;		 // Signal end of loading data
	}

	return;
}
/*#######################################################################################
 * ISR routine for handling lower priority processing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma2_ll_ptr)(void) = 0;		// DMA2CH5 -> SDIO_IRQHandler (low priority)

void SDIO_IRQHandler(void)
{
	adc_histo_build_ADC3();	// Add buffer readings to histogram

	/* Call other routines if an address is set up */
	if (dma2_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma2_ll_ptr)();	// Go do something
	return;
}
/******************************************************************************
 * static void adc_histo_build_ADC3(void);
 * @brief 	: Add ADC3 readings from DMA2CH4_5 buffer to histogram counts
 * WARNING: This is called from low priority level interrupt: SDIO_IRQHandler
*******************************************************************************/
/*
#define HISTOBINBUFNUM 3 // Number of bin buffers
struct ADCHISTO
{
	u32  bin[HISTOBINBUFNUM][ADCHISTOSIZE];	// Double buffer bin counts
	u32* pbegin;   // Beginning of bin buffer
	u32* pend;     // End of bin buffer
	u32* ph;       // Working bin buffer ptr
	int  idx_build;     // index: adding buffer
	int  idx_send;      // index: sending buffer
	int  ctr;           // Number of ADC buffers in histogram
	u32  dur;           // Number to accumulate
	int  sw;            // Histogram request switch
struct CANRCVBUF can_cmd_histo;	    // CAN msg: Histogram data
struct CANRCVBUF can_cmd_histo_final;// CAN msg: Signal end of sending
};
*/
/* Execution time test:
 Shaft stopped: 1630 - 1733 (cycles at 64E6 per sec)
 Shaft @ approx 1200 rpm: 1829
One would expect the running situation to be longer as the ADC watchdog interrupts
have a higher priority than the interrupt for this routine.
*/
static void adc_histo_build_ADC3(void)
{


	struct ADCHISTO* p = &adchisto3; // Use ptr for convenience
	u32 tmp;
	u16* pdma_end;  // Note: DMA for ADC3 is 16b
	u16* pdma;
	int dmaidx;


	/* duration = 0 signals skip processing. */
	if (p->dur == 0) return; 

	/* Skip if we are in overrun situation */
	if (p->idx_tmp == p->idx_send) return;	

	/* Pointers to bin count buffer */
	p->ph = &p->bin[p->idx_build][0];	// begin at

	/* Pointer to DMA data (note ADC12 pairing with union) */
	dmaidx   =  adc3valbuffflag; // 'buffflag might change on us!
	pdma     = &adc3valbuff[dmaidx][0];
	pdma_end = &adc3valbuff[dmaidx][ADCVALBUFFSIZE];

	/* Increment histogram bins from DMA stored ADC readings. */
	while (pdma != pdma_end)
	{ // 12b ADC collapsed to 64 bins
		tmp = (*pdma >> ADCHISTOSIZEN);	// ADC3 into 64 ranges
		*(p->ph + tmp) += 1;		// Increment bin(n) count
		pdma ++;
	}

	/* Count number of DMA buffers used in the histogram. */
	p->ctr += 1;
	if (p->ctr >= p->dur) // Completion?
	{ // Here, designated number of DMA buffers have been accumulated
		p->ctr = 0;
		p->idx_build = p->idx_tmp;
		p->idx_tmp += 1; if (p->idx_tmp >= HISTOBINBUFNUM) p->idx_tmp = 0;
		p->sw -= 1;		    // Count down the number of consectuve histograms
		if (p->sw <= 0) // End of histogramming?    
			p->dur = 0;		 // Signal end of loading data
	}
	return;
}
/******************************************************************************
 * static uint32_t pay(uint8_t* p);
 * @brief 	: Extract payload bytes to unsigned int
 * @param	: p = pointer to first of the four payload bytes
 * @return	: uint32_t
*******************************************************************************/
static uint32_t pay(uint8_t* p)
{ 
	uint32_t tmp;
	tmp  = *(p+0) <<  0;
	tmp |= *(p+1) <<  8;
	tmp |= *(p+2) << 16;
	tmp |= *(p+3) << 24;
	return tmp;
}
/******************************************************************************
 * static void loadpay(u8* pu, uint32_t* p);
 * @brief 	: Load CAN msg payload bytes from uint32_t
 * @param	: p = pointer 4 byte input
 * @param	: pu = pointer to payload start position
*******************************************************************************/
static void loadpay(u8* pu, uint32_t* p)
{
 	*(pu+0) = *p;
	*(pu+1) = *p >>  8;
	*(pu+2) = *p >> 16;
	*(pu+3) = *p >> 24;	
	return;
}
/******************************************************************************
 * void adc_histo_request(struct CANRCVBUF* p);
 * @brief 	: Handle CAN msg request for histogram
 * @param	: p = pointer to 'struct' with CAN msg
*******************************************************************************/
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
void adc_histo_request(struct CANRCVBUF* p)
{	
	uint8_t adc = p->cd.uc[1]; // Expect 2, 3, or 4
	switch (adc)
	{
	case 2: // ADC2 only
		adchisto2.sw  = p->cd.uc[2];      // Extract number consecutive bins
		adchisto2.dur = pay(&p->cd.uc[3]); // Extract payload with duration
		adchisto2.ctr = 0;	
		adchisto3.dur = 0; // jic	
		break;

	case 3: // ADC3 only
		adchisto3.sw  = p->cd.uc[2];      // Extract number consecutive bins
		adchisto3.dur = pay(&p->cd.uc[3]); // Extract payload with duration		
		adchisto3.ctr = 0;	
		adchisto2.dur = 0; // jic	
		break;

	case 4: // ADC2 and ADC3
		adchisto2.sw  = p->cd.uc[2];      // Extract number consecutive bins
		adchisto2.dur = pay(&p->cd.uc[3]); // Extract payload with duration		
		adchisto2.ctr = 0;	

		adchisto3.sw  = p->cd.uc[2];      // Extract number consecutive bins
		adchisto3.dur = pay(&p->cd.uc[3]); // Extract payload with duration		
		adchisto3.ctr = 0; // jic	
		break;
	}
	/* Save duration count in 'final CAN msg (since dur sets to zero when histo complete */
	loadpay(&adchisto2.can_cmd_histo_final.cd.uc[3], (uint32_t*)&adchisto2.dur);   // Load total count
	loadpay(&adchisto3.can_cmd_histo_final.cd.uc[3], (uint32_t*)&adchisto3.dur);   // Load total count

	adcsensor_foto_h_enable_histogram(); // Enable DMA interrupts

	return;
}

/******************************************************************************
 * void adc_histo_send(struct ADCHISTO* p);
 * @brief 	: Handle CAN msg request for histogram
 * @param	: p = pointer to struct holding ADC bins
*******************************************************************************/
void adc_histo_send(struct ADCHISTO* p)
{	
	int i;

	/* Check if sending has caught up with building */
	if (p->idx_send == p->idx_build) return; // No new data


	uint32_t* pb = &p->bin[p->idx_send][0]; // Convenience and efficiency

	/* Setup and send CAN msg for each hisogram bin */
	for (i = 0; i < ADCHISTOSIZE; i++)
	{ 
		p->can_cmd_histo.cd.uc[2] = i;
		loadpay(&p->can_cmd_histo.cd.uc[3], (uint32_t*)pb);   // Load bin count
		can_driver_put(pctl1,&p->can_cmd_histo,4,0);	// Add/send to CAN driver
		*pb++ = 0;	// Zero the count for the next round
	}	

	/* Send a termination msg */
	can_driver_put(pctl1,&p->can_cmd_histo_final,4,0);	// Add/send to CAN driver

	/* Advance index */
	p->idx_send += 1; if (p->idx_send >= HISTOBINBUFNUM) p->idx_send = 0;

	return;
}

