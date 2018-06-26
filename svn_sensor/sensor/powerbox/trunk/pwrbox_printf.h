/******************************************************************************
* File Name          : pwrbox_printf.h
* Date First Issued  : 05/24/2015,06/13/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWRBOX_PRINTF
#define __PWRBOX_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "pwrbox_idx_v_struct.h"

/* ************************************************************************************** */
void pwrbox_printf(struct PWRBOXLC* pten);
/* @brief	: Print the values
 * @param	: pten = pointer to struct with the values 
 * ************************************************************************************** */

#endif
