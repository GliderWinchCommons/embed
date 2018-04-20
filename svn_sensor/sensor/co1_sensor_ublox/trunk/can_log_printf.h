/******************************************************************************
* File Name          : can_log_printf.h
* Date First Issued  : 04/22/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_LOG_PRINTF
#define __CAN_LOG_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "logger_idx_v_struct.h"

/* ************************************************************************************** */
void can_log_printf(struct LOGGERLC* p);
/* @brief	: Print the values
 * @param	: pten = pointer to struct with the values 
 * ************************************************************************************** */

#endif
