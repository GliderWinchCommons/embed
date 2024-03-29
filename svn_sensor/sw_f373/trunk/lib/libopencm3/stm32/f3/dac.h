/** @defgroup STM32F_dac_defines DAC Defines

@brief <b>libopencm3 Defined Constants and Types for the STM32F Digital to Analog Converter </b>

@ingroup STM32F_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2012 Felix Held <felix-libopencm3@felixheld.de>
@author @htmlonly &copy; @endhtmlonly 2012 Ken Sarkies

@date 18 August 2012

LGPL License Terms @ref lgpl_license
 */

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2012 Felix Held <felix-libopencm3@felixheld.de>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@{*/

/* NOTE-- DAC2 Channel 2 is not implemented on the F373, only DAC1 CHannel 1 implemented on the F372 */

#ifndef LIBOPENCM3_DAC_H
#define LIBOPENCM3_DAC_H

#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/common.h>

/* DAC registers base adresses (for convenience) */
#define DAC1				DAC1_BASE	
#define DAC2				DAC2_BASE

/* --- DAC registers p 251 ------------------------------------------------------- */

/* DAC control register (DAC_CR) p 265 */
#define DAC1_CR				MMIO32(DAC1_BASE + 0x00)	
#define DAC2_CR				MMIO32(DAC2_BASE + 0x00)

/* DAC software trigger register (DAC_SWTRIGR)  p 268 */
#define DAC1_SWTRIGR			MMIO32(DAC1_BASE + 0x04)
#define DAC2_SWTRIGR			MMIO32(DAC2_BASE + 0x04)

/* DAC channel1 12-bit right-aligned data holding register (DAC_DHR12R1) p 268 */
#define DAC1_DHR12R1			MMIO32(DAC1_BASE + 0x08)
#define DAC2_DHR12R1			MMIO32(DAC2_BASE + 0x08)

/* DAC channel1 12-bit left aligned data holding register (DAC_DHR12L1) p 269 */
#define DAC1_DHR12L1			MMIO32(DAC1_BASE + 0x0C)
#define DAC2_DHR12L1			MMIO32(DAC2_BASE + 0x0C)

/* DAC channel1 8-bit right aligned data holding register (DAC_DHR8R1) p 269 */
#define DAC1_DHR8R1			MMIO32(DAC1_BASE + 0x10)
#define DAC2_DHR8R1			MMIO32(DAC2_BASE + 0x10)

/* DAC channel2 12-bit right aligned data holding register(DAC_DHR12R2) p 269, 270 */
#define DAC1_DHR12R2			MMIO32(DAC1_BASE + 0x14)

/* DAC channel2 12-bit left aligned data holding register (DAC_DHR12L2) p 270 */
#define DAC1_DHR12L2			MMIO32(DAC1_BASE + 0x18)

/* DAC channel2 8-bit right-aligned data holding register (DAC_DHR8R2) p 270 */
#define DAC1_DHR8R2			MMIO32(DAC1_BASE + 0x1C)

/* Dual DAC 12-bit right-aligned data holding register (DAC_DHR12RD) p 271 */
#define DAC1_DHR12RD			MMIO32(DAC_BASE + 0x20)

/* DUAL DAC 12-bit left aligned data holding register (DAC_DHR12LD) p 271 */
#define DAC1_DHR12LD			MMIO32(DAC_BASE + 0x24)

/* DUAL DAC 8-bit right aligned data holding register (DAC_DHR8RD) p 272 */
#define DAC1_DHR8RD			MMIO32(DAC1_BASE + 0x28)
#define DAC2_DHR8RD			MMIO32(DAC2_BASE + 0x28)

/* DAC channel1 data output register (DAC_DOR1) p 271 */
#define DAC1_DOR1			MMIO32(DAC1_BASE + 0x2C)
#define DAC2_DOR1			MMIO32(DAC2_BASE + 0x2C)

/* DAC channel2 data output register (DAC_DOR2) p 272, 273 */
#define DAC1_DOR2			MMIO32(DAC1_BASE + 0x30)
#define DAC2_DOR2			MMIO32(DAC2_BASE + 0x30)

/* DAC status register (DAC_SR) p 273 */
#define DAC1_SR				MMIO32(DAC1_BASE + 0x34)
#define DAC2_SR				MMIO32(DAC2_BASE + 0x34)


/* --- DAC_CR values p 265 ------------------------------------------------------- */

/* DMAUDRIE2: DAC channel2 DMA underrun interrupt enable */
/* doesn't exist in most members of the STM32F1 family */
#define DAC_CR_DMAUDRIE2		(1 << 29)

/* DMAEN2: DAC channel2 DMA enable */
#define DAC_CR_DMAEN2			(1 << 28)

/* MAMP2[3:0]: DAC channel2 mask/amplitude selector */
/* DAC_CR_MAMP2_n:
 * Unmask bits [(n-1)..0] of LFSR/Triangle Amplitude equal to (2**n)-1
 */
#define DAC_CR_MAMP2_SHIFT		24
/** @defgroup dac_mamp2 DAC Channel 2 LFSR Mask and Triangle Wave Amplitude values
@ingroup STM32F_dac_defines

Unmask bits [(n-1)..0] of LFSR/Triangle Amplitude equal to (2**(n)-1
@{*/
#define DAC_CR_MAMP2_1			(0x0 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_2			(0x1 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_3			(0x2 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_4			(0x3 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_5			(0x4 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_6			(0x5 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_7			(0x6 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_8			(0x7 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_9			(0x8 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_10			(0x9 << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_11			(0xA << DAC_CR_MAMP2_SHIFT)
#define DAC_CR_MAMP2_12			(0xB << DAC_CR_MAMP2_SHIFT)
/**@}*/

/* WAVE2[1:0]: DAC channel2 noise/triangle wave generation enable */
/* Legend:
 * DIS: wave generation disabled
 * NOISE: Noise wave generation enabled
 * TRI: Triangle wave generation enabled
 *
 * Note: only used if bit TEN2 = 1 (DAC channel2 trigger enabled)
 */
#define DAC_CR_WAVE2_SHIFT		22
#define DAC_CR_WAVE2_DIS		(0x3 << DAC_CR_WAVE2_SHIFT)
/** @defgroup dac_wave2_en DAC Channel 2 Waveform Generation Enable
@ingroup STM32F_dac_defines

@li NOISE: Noise wave generation enabled
@li TRI: Triangle wave generation enabled

@note: only used if bit TEN2 is set (DAC channel2 trigger enabled)
@{*/
#define DAC_CR_WAVE2_NOISE		(0x1 << DAC_CR_WAVE2_SHIFT)
#define DAC_CR_WAVE2_TRI		(0x2 << DAC_CR_WAVE2_SHIFT)
/**@}*/

/* TSEL2[2:0]: DAC channel2 trigger selection */
/* Legend:
 *
 * T6: Timer 6 TRGO event
 * T3: Timer 3 TRGO event
 * T8: Timer 8 TRGO event
 * T7: Timer 7 TRGO event
 * T5: Timer 5 TRGO event
 * T15: Timer 15 TRGO event
 * T2: Timer 2 TRGO event
 * T4: Timer 4 TRGO event
 * E9: External line9
 * SW: Software trigger
 *
 * Note: only used if bit TEN2 = 1 (DAC channel2 trigger enabled)
 * Note: T3 == T8; T5 == T15; not both present on one device
 * Note: this is *not* valid for the STM32L1 family
 */
#define DAC_CR_TSEL2_SHIFT		19
/** @defgroup dac_trig2_sel DAC Channel 2 Trigger Source Selection
@ingroup STM32F_dac_defines

@li T6: Timer 6 TRGO event
@li T3: Timer 3 TRGO event
@li T8: Timer 8 TRGO event
@li T7: Timer 7 TRGO event
@li T5: Timer 5 TRGO event
@li T15: Timer 15 TRGO event
@li T2: Timer 2 TRGO event
@li T4: Timer 4 TRGO event
@li E9: External line9
@li SW: Software trigger

@note: Refer to the timer documentation for details of the TRGO event.
@note: T3 replaced by T8 and T5 replaced by T15 in some devices.
@note: this is <b>not</b> valid for the STM32L1 family.
@note: only used if bit TEN2 is set (DAC channel 2 trigger enabled)
@{*/
#define DAC_CR_TSEL2_T6			(0x0 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T3			(0x1 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T8			(0x1 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T7			(0x2 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T5			(0x3 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T15		(0x3 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T2			(0x4 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_T4			(0x5 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_E9			(0x6 << DAC_CR_TSEL2_SHIFT)
#define DAC_CR_TSEL2_SW			(0x7 << DAC_CR_TSEL2_SHIFT)
/**@}*/

/* TEN2: DAC channel2 trigger enable */
#define DAC_CR_TEN2			(1 << 18)

/* BOFF2: DAC channel2 output buffer disable */
#define DAC_CR_BOFF2			(1 << 17)

/* EN2: DAC channel2 enable */
#define DAC_CR_EN2			(1 << 16)

/* DMAUDRIE1: DAC channel1 DMA underrun interrupt enable */
/* doesn't exist in most members of the STM32F1 family */
#define DAC_CR_DMAUDRIE1	(1 << 13)

/* DMAEN1: DAC channel1 DMA enable */
#define DAC_CR_DMAEN1			(1 << 12)

/* MAMP1[3:0]: DAC channel1 mask/amplitude selector */
/* DAC_CR_MAMP1_n:
 * Unmask bits [(n-1)..0] of LFSR/Triangle Amplitude equal to (2**n)-1
 */
#define DAC_CR_MAMP1_SHIFT		8
/** @defgroup dac_mamp1 DAC Channel 1 LFSR Mask and Triangle Wave Amplitude values
@ingroup STM32F_dac_defines

Unmask bits [(n-1)..0] of LFSR/Triangle Amplitude equal to (2**(n+1)-1
@{*/
#define DAC_CR_MAMP1_1			(0x0 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_2			(0x1 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_3			(0x2 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_4			(0x3 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_5			(0x4 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_6			(0x5 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_7			(0x6 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_8			(0x7 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_9			(0x8 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_10			(0x9 << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_11			(0xA << DAC_CR_MAMP1_SHIFT)
#define DAC_CR_MAMP1_12			(0xB << DAC_CR_MAMP1_SHIFT)
/**@}*/

/* WAVE1[1:0]: DAC channel1 noise/triangle wave generation enable */
/* Legend:
 * DIS: wave generation disabled
 * NOISE: Noise wave generation enabled
 * TRI: Triangle wave generation enabled
 *
 * Note: only used if bit TEN1 = 1 (DAC channel1 trigger enabled)
 */
#define DAC_CR_WAVE1_SHIFT		6
#define DAC_CR_WAVE1_DIS		(0x3 << DAC_CR_WAVE1_SHIFT)
/** @defgroup dac_wave1_en DAC Channel 1 Waveform Generation Enable
@ingroup STM32F_dac_defines

@li DIS: wave generation disabled
@li NOISE: Noise wave generation enabled
@li TRI: Triangle wave generation enabled

@note: only used if bit TEN2 = 1 (DAC channel2 trigger enabled)
@{*/
#define DAC_CR_WAVE1_NOISE		(0x1 << DAC_CR_WAVE1_SHIFT)
#define DAC_CR_WAVE1_TRI		(0x2 << DAC_CR_WAVE1_SHIFT)
/**@}*/

/* TSEL1[2:0]: DAC channel1 trigger selection */
/* Legend:
 *
 * T6: Timer 6 TRGO event
 * T3: Timer 3 TRGO event in connectivity line devices
 * T8: Timer 8 TRGO event in high-density and XL-density devices
 * T7: Timer 7 TRGO event
 * T5: Timer 5 TRGO event
 * T15: Timer 15 TRGO event
 * T2: Timer 2 TRGO event
 * T4: Timer 4 TRGO event
 * E9: External line9
 * SW: Software trigger
 *
 * Note: only used if bit TEN1 = 1 (DAC channel1 trigger enabled)
 * Note: T3 == T8; T5 == T15; not both present on one device
 * Note: this is *not* valid for the STM32L1 family
 */
#define DAC_CR_TSEL1_SHIFT		3
/** @defgroup dac_trig1_sel DAC Channel 1 Trigger Source Selection
@ingroup STM32F_dac_defines

@li T6: Timer 6 TRGO event
@li T3: Timer 3 TRGO event
@li T8: Timer 8 TRGO event
@li T7: Timer 7 TRGO event
@li T5: Timer 5 TRGO event
@li T15: Timer 15 TRGO event
@li T2: Timer 2 TRGO event
@li T4: Timer 4 TRGO event
@li E9: External line 9
@li SW: Software trigger

@note: Refer to the timer documentation for details of the TRGO event.
@note: T3 replaced by T8 and T5 replaced by T15 in some devices.
@note: this is <b>not</b> valid for the STM32L1 family.
@note: only used if bit TEN2 is set (DAC channel 1 trigger enabled).
@{*/
#define DAC_CR_TSEL1_T6			(0x0 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T3			(0x1 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T8			(0x1 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T7			(0x2 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T5			(0x3 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T15		(0x3 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T2			(0x4 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_T4			(0x5 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_E9			(0x6 << DAC_CR_TSEL1_SHIFT)
#define DAC_CR_TSEL1_SW			(0x7 << DAC_CR_TSEL1_SHIFT)
/**@}*/

/* TEN1: DAC channel1 trigger enable */
#define DAC_CR_TEN1			(1 << 2)

/* BOFF1: DAC channel1 output buffer disable */
#define DAC_CR_BOFF1			(1 << 1)

/* EN1: DAC channel1 enable */
#define DAC_CR_EN1			(1 << 0)


/* --- DAC_SWTRIGR values p 268 -------------------------------------------------- */

/* SWTRIG2: DAC channel2 software trigger */
#define DAC_SWTRIGR_SWTRIG2		(1 << 1)

/* SWTRIG1: DAC channel1 software trigger */
#define DAC_SWTRIGR_SWTRIG1		(1 << 0)


/* --- DAC_DHR12R1 values p 268 -------------------------------------------------- */
#define DAC_DHR12R1_DACC1DHR_LSB	(1 << 0)
#define DAC_DHR12R1_DACC1DHR_MSK	(0x0FFF << 0)


/* --- DAC_DHR12L1 values p 269 -------------------------------------------------- */
#define DAC_DHR12L1_DACC1DHR_LSB	(1 << 4)
#define DAC_DHR12L1_DACC1DHR_MSK	(0x0FFF << 4)


/* --- DAC_DHR8R1 values p 269 --------------------------------------------------- */
#define DAC_DHR8R1_DACC1DHR_LSB		(1 << 0)
#define DAC_DHR8R1_DACC1DHR_MSK		(0x00FF << 0)


/* --- DAC_DHR12R2 values p 269 -------------------------------------------------- */
#define DAC_DHR12R2_DACC2DHR_LSB	(1 << 0)
#define DAC_DHR12R2_DACC2DHR_MSK	(0x00FFF << 0)


/* --- DAC_DHR12L2 values p 270 -------------------------------------------------- */
#define DAC_DHR12L2_DACC2DHR_LSB	(1 << 4)
#define DAC_DHR12L2_DACC2DHR_MSK	(0x0FFF << 4)


/* --- DAC_DHR8R2 values p 270 --------------------------------------------------- */
#define DAC_DHR8R2_DACC2DHR_LSB		(1 << 0)
#define DAC_DHR8R2_DACC2DHR_MSK		(0x00FF << 0)


/* --- DAC_DHR12RD values p 271 -------------------------------------------------- */
#define DAC_DHR12RD_DACC2DHR_LSB	(1 << 16)
#define DAC_DHR12RD_DACC2DHR_MSK	(0x0FFF << 16)
#define DAC_DHR12RD_DACC1DHR_LSB	(1 << 0)
#define DAC_DHR12RD_DACC1DHR_MSK	(0x0FFF << 0)


/* --- DAC_DHR12LD values p 271 -------------------------------------------------- */
#define DAC_DHR12LD_DACC2DHR_LSB	(1 << 16)
#define DAC_DHR12LD_DACC2DHR_MSK	(0x0FFF << 20)
#define DAC_DHR12LD_DACC1DHR_LSB	(1 << 0)
#define DAC_DHR12LD_DACC1DHR_MSK	(0x0FFF << 4)


/* --- DAC_DHR8RD values p 272 --------------------------------------------------- */
#define DAC_DHR8RD_DACC2DHR_LSB		(1 << 8)
#define DAC_DHR8RD_DACC2DHR_MSK		(0x00FF << 8)
#define DAC_DHR8RD_DACC1DHR_LSB		(1 << 0)
#define DAC_DHR8RD_DACC1DHR_MSK		(0x00FF << 0)


/* --- DAC_DOR1 values  p 272 ----------------------------------------------------- */
#define DAC_DOR1_DACC1DOR_LSB		(1 << 0)
#define DAC_DOR1_DACC1DOR_MSK		(0x0FFF << 0)


/* --- DAC_DOR2 values p 272, 273 ------------------------------------------------- */
#define DAC_DOR2_DACC2DOR_LSB		(1 << 0)
#define DAC_DOR2_DACC2DOR_MSK		(0x0FFF << 0)

/* --- DAC_SR values p 273 -------------------------------------------------------- */
#define DAC_SR_DMAUDR2			(1 << 29) 	// DAC channel2 DMA underrun flag
#define DAC_SR_DMAUDR1			(1 << 13)	// DAC channel1 DMA underrun flag

/** DAC channel identifier */
typedef enum {
	CHANNEL_1, CHANNEL_2, CHANNEL_D
} data_channel;

/** DAC data size (8/12 bits), alignment (right/left) */
typedef enum {
	RIGHT8, RIGHT12, LEFT12
} data_align;

/* --- Function prototypes ------------------------------------------------- */

BEGIN_DECLS

void dac_enable(data_channel dac_channel);
void dac_disable(data_channel dac_channel);
void dac_buffer_enable(data_channel dac_channel);
void dac_buffer_disable(data_channel dac_channel);
void dac_dma_enable(data_channel dac_channel);
void dac_dma_disable(data_channel dac_channel);
void dac_trigger_enable(data_channel dac_channel);
void dac_trigger_disable(data_channel dac_channel);
void dac_set_trigger_source(u32 dac_trig_src);
void dac_set_waveform_generation(u32 dac_wave_ens);
void dac_disable_waveform_generation(data_channel dac_channel);
void dac_set_waveform_characteristics(u32 dac_mamp);
void dac_load_data_buffer_single(u32 dac_data, data_align dac_data_format, data_channel dac_channel);
void dac_load_data_buffer_dual(u32 dac_data1, u32 dac_data2, data_align dac_data_format);
void dac_software_trigger(data_channel dac_channel);

END_DECLS

#endif
/**@}*/

