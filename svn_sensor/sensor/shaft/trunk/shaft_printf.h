/******************************************************************************
* File Name          : shaft_printf.h
* Date First Issued  : 04/23/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SHAFT_PRINTF
#define __SHAFT_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "shaft_idx_v_struct.h"

/* ************************************************************************************** */
void shaft_printf(struct SHAFTLC* p);
/* @brief	: Print the values
 * @param	: p = pointer to struct with the values 
 * ************************************************************************************** */

#endif
