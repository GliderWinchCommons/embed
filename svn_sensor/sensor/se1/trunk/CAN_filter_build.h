/******************************************************************************
* File Name          : CAN_filter_build.h
* Date First Issued  : 04/06/2018
* Board              :
* Description        : Build a composite list for CAN addresses for CAN hw filter
*******************************************************************************/
/*
This routine is called from the xxx_idx_v_struct.c when it transfers high-flash
parameters to sram. 

Duplicate CAN IDs are rejected
*/

#ifndef __CAN_FILTER_BUILD
#define __CAN_FILTER_BUILD

#define CANFILTERLISTSIZE 32	// Allow for more than hardware can hold

/* **************************************************************************************/
void CAN_filter_build_fifo1(uint32_t canid);
/* @brief	: Add CAN filtering for CAN fifo1
 * @param	: canid = CAN id to be added to list
 * ************************************************************************************** */
void CAN_filter_build_fifo0(uint32_t canid);
/* @brief	: Add CAN filtering for CAN fifo0
 * @param	: canid = CAN id to be added to list
 * ************************************************************************************** */
uint32_t* CAN_filter_build_fifo1_fetch(uint8_t r);
/* @brief	: fetch canid from fifo1 list
 * @param	: r: 1 to re-start from beginning, 0 to continue
 * @return	: pointer to canid, or NULL when list exhausted
 * ************************************************************************************** */
uint32_t* CAN_filter_build_fifo0_fetch(uint8_t r);
/* @brief	: fetch canid from fifo0 list
 * @param	: r: 1 to re-start from beginning, 0 to continue
 * @return	: pointer to canid, or NULL when list exhausted
 * ************************************************************************************** */
int CAN_filter_build_list_add(void);
/* @brief	: Add list previously built to fifos
 * ************************************************************************************** */

#endif

