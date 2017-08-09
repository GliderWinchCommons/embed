/******************************************************************************
* File Name          : encoder_a_printf.h
* Date First Issued  : 08/08/2017
* Board              : Discovery F4
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENCODER_A_PRINTF
#define __ENCODER_A_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "encoder_idx_v_struct.h"

/* ************************************************************************************** */
void encoder_a_printf(struct ENCODERALC* penc);
/* @brief	: Print the values
 * @param	: penc = pointer to struct with the values 
 * ************************************************************************************** */

#endif
