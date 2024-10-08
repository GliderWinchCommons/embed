/******************************************************************************
* File Name          : ad7799_ten_comm.h
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for operating AD7799_1 for winch tension
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AD7799_COMM_TEN2
#define __AD7799_COMM_TEN2

#include <stdint.h>

#define NUMAD7799	2	// Number of AD7799s

/* These 'unions' are used to re-order the bytes into the correct order for mult-byte register reading.
The AD7799 sends the data MSB first, so each byte needs to be stored in reverse order for this machine. */
union SHORTCHAR
{
	unsigned short us;
	uint8_t uc[2];
};
union INTCHAR
{
	int32_t	n;	// 32 bit signed for final result
	uint8_t	uc[4];	// Three bytes from AD7799 are stored here, then fixed up
	int8_t	c[4];	// Signed chars
	int16_t	s[2];	// Signed short for extending hi-order byte after re-ordering
};

/* Macros that help getting the selection codes right (for bozos like the author) */
#define AD7799_RD_MODE_REG		(ad7799_ten_rd_mode_reg(n)) 		// Register selection code: 001
#define AD7799_RD_CONFIGURATION_REG	(ad7799_ten_rd_configuration_reg(n)) 	// Register selection code: 010
#define AD7799_RD_DATA_REG		(ad7799_ten_rd_data_reg(n)) 		// Register selection code: 011
#define AD7799_RD_ID_REG		(ad7799_ten_rd_ID_reg (n)) 		// Register selection code: 100
#define AD7799_RD_IO_REG		(ad7799_ten_rd_IO_reg (n)) 		// Register selection code: 101
#define AD7799_RD_OFFSET_REG		(ad7799_ten_rd_offset_reg (n)) 		// Register selection code: 110
#define AD7799_RD_FULLSCALE_REG		(ad7799_ten_rd_fullscale_reg(n)) 	// Register selection code: 111

#define AD7799_SIGNEXTEND_24BIT_REG	(ad7799_ten_24bit.s[1] = ad7799_ten_24bit.c[2])	// Sign extend the high order byte

struct AD7799REGS
{
	int16_t mode_reg;	// Latest register reading is saved in these
	int16_t config_reg;
	int32_t data_reg;
	uint8_t id_reg;
	uint8_t io_reg;
	uint8_t status_reg;
	uint32_t offset_reg;
	uint32_t fullscale_reg;
	int32_t lgr;		// Last data_reg reading (offset adjusted)
};

/* ------------- Configuration register --------------------------------------*/
/* Channel selection  ("$" = reset setting) */
#define AD7799_CH_1	0x0	//$AIN1(+) - AIN1(-)  Calibration pair 0
#define AD7799_CH_2	0x1	// AIN1(+) - AIN1(-)  Calibration pair 1
#define AD7799_CH_3	0x2	// AIN1(+) - AIN1(-)  Calibration pair 2
#define AD7799_CH_1X	0x3	// AIN1(-) - AIN1(-)  Calibration pair 0
#define AD7799_CH_AV	0x7	// Auto select gain = 1/6, and internal 1.17v ref
/* Buffer selection */
#define AD7799_BUF_IN	(1<<4)	// Select buffer amp (only for gains 1x, 2x)
/* Reference detection function */
#define AD7799_REF_DET	(1<<5)	// Reference detection ON
/* Gain selection */
#define AD7799_1NO	0	// gain 1   max input = 2.5v  (No in-amp used)
#define AD7799_2NO	(1<<8)	// gain 2   max input = 1.25v (No in-amp used)
#define AD7799_4	(2<<8)	// gain 4   max input = 625mv
#define AD7799_8	(3<<8)	// gain 8   max input = 312.5mv
#define AD7799_16	(4<<8)	// gain 16  max input = 156.2mv
#define AD7799_32	(5<<8)	// gain 32  max input = 78.125mv
#define AD7799_64	(6<<8)	// gain 64  max input = 39.06mv
#define AD7799_128	(7<<8)	// gain 128 max input = 19.53mv
/* Unipolar/bipolar */
#define AD7799_UNI	(1<<12)	// zero differential = 0x000000; full scale == 0xffffff
/* Burn out current detection */
#define AD7799_BOUT	(1<<13)	// 100nA current sources in signal path enabled

/* --------------- Mode register ---------------------------------------------*/
#define AD7799_CONT		0	// Continuous conversion
#define AD7799_SINGLE		(1<<13)	// Single conversion then power down
#define AD7799_IDLE		(2<<13)	// Idle: Modulator clocks run, modulator and ADC filter held reset
#define AD7799_PWRDN		(3<<13)	// Power down: circuitry powered down including burnout currents
#define AD7799_ZEROCALIB	(4<<13)	// Internal calibrate: short input, set offset register, then place in idle
#define AD7799_FSCALIB		(5<<13)	// Full scale calibrate: connect to input, calib, place in idle (not applicable to 128 gain)
#define AD7799_SYSZEROCALIB	(6<<13)	// System zero-scale calibrate: measure with input zero (no load on load cell) & set register
#define AD7799_SYSFSCALIB	(7<<13)	// System full-scale calibrate: measure will full scale load & set register
/* Output FET */
#define AD7799_PSW		(1<<12)	// Open-drain FET ON.
/* Conversion update rate ( 'p' in the following is for a decimal point) */
//             fADC (Hz)       tSETTLE (ms)   Rejection @ 50 Hz/60 Hz */
#define AD7799_Reserved	0	// ...
#define AD7799_470SPS	1 	//   4 (ms)
#define AD7799_242SPS	2	//   8
#define AD7799_123SPS	3	//  16
#define AD7799_62SPS	4	//  32
#define AD7799_50SPS	5	//  40
#define AD7799_39SPS	6	//  48
#define AD7799_33p2SPS	7	//  60
#define AD7799_19p6SPS	8	// 101          90 dB (60 Hz only)
#define AD7799_16p7SPS	9	// 120          80 dB (50 Hz only)
#define AD7799_16p7bSPS	10	// 120          65 dB
#define AD7799_12p5SPS	11	// 160          66 dB
#define AD7799_10SPS	12	// 200          69 dB
#define AD7799_8p33SPS	13	// 240          70 dB
#define AD7799_6p25SPS	14	// 320          72 dB
#define AD7799_4p17SPS	15	// 480          74 dB

/* ----------------- Status register -----------------------------------------*/
#define AD7799_NRDY	(1<<7)	// Not Ready: goes low when data written to data register
#define AD7799_ERR	(1<<6)	// Data reg has been clamped to zero. Overrange, underange.  Cleared by start conversion
#define AD7799_NREF	(1<<5)	// No Reference Bit.  Reference voltage is below limit.
#define AD7799_CHSET	(0x07)	// Channel setting (see configuration register)

/******************************************************************************/
int32_t ad7799_ten_1_initA (uint8_t ucSPS);
/* @brief	: Initialize AD7799_1 registers for strain gauge
 * @param	: ucSPS: Sampling rate code (see ad7799_ten_comm.h file)
 * @return	: 0 = success; -1 = failed (timed out)
*******************************************************************************/
int32_t ad7799_ten_1_initB (uint8_t ucSPS);
/* @brief	: Initialize AD7799_1 registers for thermistor bridge
 * @param	: ucSPS: Sampling rate code (see ad7799_ten_comm.h file)
 * @return	: 0 = success; -1 = failed (timed out)
*******************************************************************************/
void ad7799_ten_wr_comm_rd24b (uint8_t ucX, uint32_t n);
/* @brief	: Write comm register, then read 24b data register (set/exit continuous mode)
 * @param	: uc = Code
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1;
 * @return	: ad7799_ten_comm_busy = 0 when operation completes 
*******************************************************************************/
void ad7799_ten_rd_8bit_reg (uint8_t ucX, uint8_t* p);
/* @brief	: Read an 8 bit register from ad7799
 * @param	: ucX = register code
 * @return	: ad7799_ten_comm_busy = 0 when operation complete 
*******************************************************************************/
void ad7799_ten_rd_status_reg (uint32_t n);
/* @brief	: Read status register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_IO_reg (uint32_t n);
/* @brief	: Read IO register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_ID_reg (uint32_t n);
/* @brief	: Read ID register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_configuration_reg (uint32_t n);
/* @brief	: Read 16 bit configuration register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_16bit' in correct byte order
*******************************************************************************/
void ad7799_ten_wr_configuration_reg (unsigned short usX);
/* @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit configuration register to ad7799_ten
 * @return	: only hope that it worked.
*******************************************************************************/
void ad7799_ten_rd_mode_reg (uint32_t n);
/* @brief	: Read 16 bit mode register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_16bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_offset_reg (uint32_t n);
/* @brief	: Read 24 bit mode register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_24bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_fullscale_reg (uint32_t n);
/* @brief	: Read 24 bit mode register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_24bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_data_reg (uint32_t n);
/* @brief	: Read 24 bit mode register from ad7799_ten
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_24bit' in correct byte order
*******************************************************************************/
void ad7799_ten_wr_mode_reg (unsigned short usX);
/* @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit mode register to ad7799_ten
 * @return	: only hope that it worked.
*******************************************************************************/
void ad7799_ten_set_continous_rd (uint32_t n);
/* @brief	: Writes 0x5c to the comm register to set part in continuous read mode
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 *		: Needs to be followed by a 24b reg read
*******************************************************************************/
void ad7799_ten_exit_continous_rd (uint32_t n);
/* @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @brief	: Writes 0x58 to the comm register to exit continuous read mode
*******************************************************************************/
void ad7799_ten_reset (void);
/* @brief	: Send 32 consecutive 1's to reset the digital interface
 * @return	: None now 
*******************************************************************************/
void ad7799_ten_rd_16bit_reg (uint8_t uc, int16_t* p);
/* @brief	: Read an 16 bit register from ad7799_ten
 * @param	: uc = register code
 * @param	: p = pointer to where the result is to be copied
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_24bit_reg (uint8_t uc, uint32_t* p);
/* @brief	: Read an 24 bit register from ad7799_ten
 * @param	: uc = register selection code
 * @param	: p = pointer to where result is copied
 * @return	: ad7799_ten_comm_busy set to 0 when operation sequence is complete
*******************************************************************************/
void ad7799_ten_signextend_24bit_reg (void);
/* @brief	: Convert 3 byte readings to 4 byte signed
*******************************************************************************/
void ad7799_ten_wr_16bit_reg (uint8_t);
/* @brief	: Write an 16 bit register from ad7799_ten
 * @param	: register code
 * @return	: 'ad7799_ten_comm_busy' flag is set to zero when sequence completes
*******************************************************************************/
void ad7799_ten_wr_offset_reg (int32_t offset);
/* @brief	: Write offset register with some program generated value
 * @param	: offset = value to write into offset register
 * @return	: Operation complete when (ad7799_ten_comm_busy==0) 
*******************************************************************************/
void ad7799_ten_wr_24bit_reg (uint8_t uc, int32_t n);
/* @brief	: Write an 16 bit register from ad7799_ten
 * @param	: uc = register code
 * @param	: n = value to be written
 * @return	: 'ad7799_ten_comm_busy' flag is set to zero when sequence completes
*******************************************************************************/

extern volatile uint8_t ad7799_ten_comm_busy; // 0 = not busy with a sequence of steps, not zero = busy
extern int32_t ad7799_ten_flag;	// Flag for flashing LED

/* Readings and registers for the AD7799. */
extern struct AD7799REGS ad7799regs[NUMAD7799];

#endif 
