/******************************************************************************
* File Name          : running_average.h
* Date First Issued  : 06/11/2017
* Board              : 
* Description        : Compute running average
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RUNNING_AVERAGE
#define __RUNNING_AVERAGE

#include <stdint.h>



struct RUNNING_AVE
{
	int *p0;	// Pointer to beginning of array
	int *p;		// Pointer to element		
	int64_t sum;	// Running sum
	uint16_t N;	// Number in array
	uint16_t scale;	// Scale factor (bit shift count)
};

/******************************************************************************/
void recursive_average_reset(struct RECURSIVE_AVE* p);
/* @brief	: Reset averaging * @param	: p = pointer to struct with stuff for the variable of interest
 * @return	: none; struct is initialized
*******************************************************************************/
int64_t recursive_average(struct RECURSIVE_AVE* p, int x);
/* @brief	: Compute the average recursively
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @param	: x = new reading NOT SCALED
 * @return	: SCALED average; also struct is updated during this computation
*******************************************************************************/


#endif
