/******************************************************************************
* File Name          : crc-32_nib.h
* Date First Issued  : 08-29-2022
* Board              : 
* Description        : CRC-32 computation with no nibble table lookup
*******************************************************************************/

#ifndef __CRC_32_NIB
#define __CRC_32_NIB

#include <stdint.h>

/******************************************************************************/
uint32_t crc_32_nib_acc(uint32_t Crc, uint32_t Data);
/* @brief   : Accumulate Crc value for one input word
 * @param   : Crc = previous Crc value
 * @param   : Data = input word
*******************************************************************************/
uint32_t crc_32_nib_calc(uint32_t* pdata, uint32_t len);
/* @brief   : Compute CRC-32
 * @param   : pdata = pointer to input array (words)
 * @param   : len = number of words
 * @param   : CRC-32
*******************************************************************************/

#endif