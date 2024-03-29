/******************************************************************************
* File Name          : canwinch_setup_F4_discovery.c
* Date First Issued  : 06-15-2015
* Board              : F4 discovery
* Description        : Setup initializtion of CAN1 and/or CAN2
*******************************************************************************/
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_driver_port.h"
#include "common_can.h"
#include "clockspecifysetup.h"
//#include "libusartstm32f4/nvicdirect.h"
#include "libopencm3/stm32/nvic.h"

/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc: 0 = internal 16 MHz rc; 1 = external xtal controlled; 2 = ext input; 3 ext remapped xtal; 4 ext input */ \
1,			/* Source for main PLL & audio PLLI2S: 0 = HSI, 1 = HSE selected */ \
APBX_4,			/* APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 42 MHz */ \
APBX_2,			/* APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 84 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2 and HCLK) */ \
8000000,		/* External Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
7,			/* Q (PLL) divider: USB OTG FS, SDIO, random number gen. USB OTG FS clock freq = VCO freq / PLLQ with 2 ≤ PLLQ ≤ 15 */ \
PLLP_2,			/* P Main PLL divider: PLL output clock frequency = VCO frequency / PLLP with PLLP = 2, 4, 6, or 8 */ \
84,			/* N Main PLL multiplier: VCO output frequency = VCO input frequency × PLLN with 64 ≤ PLLN ≤ 432	 */ \
2			/* M VCO input frequency = PLL input clock frequency / PLLM with 2 ≤ PLLM ≤ 63 */
};
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
/* Parameters for setting up CAN--DISCOVERY F4 board. */
// CAN params: Based on 168 MHz clock|42 MHz APB1/pclk1_freq
// (1 + tbs1 + tbs2) = 6, 7, 21 (to make even 1E6, 5E5,...baud)
// (1 + 5 + 1) gives sample pt: 6/7 = 85.7% pt (CANOpen spec = 87.5%)
// pclk1_freq / (1 + tbs1 + tbs2) = power of 2.
static const struct CAN_PARAMS2 params1 = { \
CANWINCH_BAUDRATE,	// CAN baudrate for winch (ref: 'svn_common/trunk/common_can.h')
1,		/* CAN number (1 = CAN1, 2 = CAN2) */
3,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
1,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
1,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 5)
5,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 15)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
0,		// ttcm: CAN_MCR[7] Time triggered communication mode
1,		// abom: CAN_MCR[6] Automatic bus-off management
0,		// awum: CAN_MCR[5] Auto WakeUp Mode
0,		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once)
{ NVIC_CAN1_TX_IRQ,  NVIC_USB_HP_CAN_TX_IRQ_PRIORITY},	/* TX  interrupt (number & interrupt priority) */
{ NVIC_CAN1_RX0_IRQ, NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY},	/* RX0 interrupt (number & interrupt priority) */
{ NVIC_CAN1_RX1_IRQ, NVIC_CAN_RX1_IRQ_PRIORITY},			/* RX1 interrupt (number & interrupt priority) */
};
/* Parameters for setting up CAN2, based on 168Hz clock */
static const struct CAN_PARAMS2 params2 = { \
CANWINCH_BAUDRATE_CAN2,	// CAN baudrate for winch (ref: 'svn_common/trunk/common_can.h')
2,		/* CAN number (1 = CAN1, 2 = CAN2) */
5,		// port: port: 4 = PB 8|9; 5 = PB 12|13;  ( <4 not valid; >5 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
1,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
1,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 5)
5,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 15)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
0,		// ttcm: CAN_MCR[7] Time triggered communication mode
1,		// abom: CAN_MCR[6] Automatic bus-off management
0,		// awum: CAN_MCR[5] Auto WakeUp Mode
0,		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once)
{ NVIC_CAN2_TX_IRQ,  NVIC_CAN2_TX_IRQ_PRIORITY},	/* TX  interrupt (number & interrupt priority) */
{ NVIC_CAN2_RX0_IRQ, NVIC_CAN2_RX0_IRQ_PRIORITY},	/* RX0 interrupt (number & interrupt priority) */
{ NVIC_CAN2_RX1_IRQ, NVIC_CAN2_RX1_IRQ_PRIORITY},	/* RX1 interrupt (number & interrupt priority) */
};
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
/* Hardware filters */
// These are default filters that place a small block of the highest priority (11b) msgs in FIFO1 and the 
//   reset go into FIFO0.
// Designate a high priority block of msgs that go to FIFO1
//                                             CAN ID               MASK                   FIFO_SCALE_MODE      BANK NUM  ODD/EVEN
static const struct CANFILTERPARAM fb0_CAN1 = {CANWINCH_FIFO1_ID1, CANWINCH_FIFO1_MSK1, CANFILT_FIFO1_32b_MSK_ID,  0,    CANFILT_EVEN};
static const struct CANFILTERPARAM fb1_CAN1 = {CANWINCH_FIFO0_ID1, CANWINCH_FIFO0_MSK1, CANFILT_FIFO0_32b_MSK_ID,  1,    CANFILT_EVEN};
// CAN2 filters
static const struct CANFILTERPARAM fb0_CAN2 = {CANWINCH_FIFO1_ID1, CANWINCH_FIFO1_MSK1, CANFILT_FIFO1_32b_MSK_ID,  0+14, CANFILT_EVEN};
static const struct CANFILTERPARAM fb1_CAN2 = {CANWINCH_FIFO0_ID1, CANWINCH_FIFO0_MSK1, CANFILT_FIFO0_32b_MSK_ID,  1+14, CANFILT_EVEN};
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/******************************************************************************
 * struct CLOCKS* canwinch_setup_F4_discovery_clocks(void);
 * @brief 	: Supply 'clock' params that match CAN setup params
 * @return	:  pointer to struct
*******************************************************************************/
struct CLOCKS* canwinch_setup_F4_discovery_clocks(void)
{
	return (struct CLOCKS*)&clocks;	// cast to stop 'const' warning
}
/******************************************************************************
* static int canwinch_setup_F4_discovery_filter(u32 num);
* @brief	: Setup the filters 
* @param	: num = 1 or 2 for CAN1 or CAN2
* @return	: 0 = success; -1 for error
*******************************************************************************/
static int canwinch_setup_F4_discovery_filter(u32 num)
{
	/* Insert new filters and activate. */
	int ret;
	if (num == 1)
	{
		ret = can_driver_filter_insert((struct CANFILTERPARAM*)&fb0_CAN1);
		if (ret != 0) return ret;
		ret = can_driver_filter_insert((struct CANFILTERPARAM*)&fb1_CAN1);
	}
	else
	{
		ret = can_driver_filter_insert((struct CANFILTERPARAM*)&fb0_CAN2);
		if (ret != 0) return ret;
		ret = can_driver_filter_insert((struct CANFILTERPARAM*)&fb1_CAN2);

	}
	return ret;
}
/******************************************************************************
 * struct CAN_CTLBLOCK* canwinch_setup_F4_discovery(const struct CAN_INIT pinit, u32 cannum);
 * @brief 	: Provide CAN1 initialization parameters: winch app, F103, pod board
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @param	: cannum = 1 or 2 for CAN1 or CAN2
 * @return	: Pointer to control block for this CAN
 *		:  Pointer->ret = return code
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -3 RX0 get buffer failed
 *		:  -4 RX1 get buffer failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
 *		: -12 port pin setup CAN1
 *		: -13 port pin setup CAN2
*******************************************************************************/
struct CAN_CTLBLOCK* canwinch_setup_F4_discovery(const struct CAN_INIT* pinit, u32 cannum)
{
	int ret;
	struct CAN_CTLBLOCK* pctl = NULL;

	switch (cannum)	
	{
	case 1: // CAN1 intialize
		pctl = can_driver_init((struct CAN_PARAMS2*)&params1, pinit);
		break;
	case 2: // CAN2 initialize
		pctl = can_driver_init((struct CAN_PARAMS2*)&params2, pinit);
		break;
	default: // Bad CAN number
		return pctl;
	}

	/* Early CAN init failure returns NULL. */
	if (pctl == NULL) return pctl;

	/* Check for more init problems. */
	if (pctl->ret != 0) return pctl;

	/* Here, CAN init OK.  Setup hardware msg filtering */
	ret = canwinch_setup_F4_discovery_filter(cannum);
	if (ret != 0) pctl->ret = (ret - 7); // Set ret to -8 or -9

	return pctl;
	
}

