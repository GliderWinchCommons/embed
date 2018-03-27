/******************************************************************************
* File Name          : eng_rpm_printf.h
* Date First Issued  : 03/26/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENG_RPM_PRINTF
#define __ENG_RPM_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "engine_idx_v_struct.h"

/* ************************************************************************************** */
void eng_rpm_printf(struct ENGRPMLC* p);
/* @brief	: Print the values
 * @param	: pten = pointer to struct with the values 
 * ************************************************************************************** */

#endif
