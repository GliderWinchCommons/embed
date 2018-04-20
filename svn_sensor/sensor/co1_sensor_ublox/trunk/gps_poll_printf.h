/******************************************************************************
* File Name          : eng_man_printf.h
* Date First Issued  : 03/26/2018
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS_POLL_PRINTF
#define __GPS_POLL_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "gps_idx_v_struct.h"

/* ************************************************************************************** */
void gps_poll_printf(struct GPSLC* p);
/* @brief	: Print the values
 * @param	: pten = pointer to struct with the values 
 * ************************************************************************************** */

#endif
