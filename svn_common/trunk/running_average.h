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
#include <stdlib.h>

struct RUNNING_AVE
{
	int64_t *p0;	// Pointer to beginning of array
	int64_t *pend;	// p0 + n
	int64_t *pn;	// Pointer to element		
	int64_t sum;	// Running sum
	uint16_t n;	// Working count
	uint16_t N;	// Number in array
	uint16_t scale;	// Scale factor (bit shift count)
};

/******************************************************************************/
int64_t* running_average_init(struct RUNNING_AVE* p, uint16_t n, uint16_t scale);
/* @brief	: Get memory and init for averaging
 * @param	: p = pointer to struct with pointers etc.
 * @param	: n = number of elements in the average
 * @param	: scale = bit shift count for scaling
 * @return	: pointer malloc'ed array
*******************************************************************************/
int64_t running_average_reset(struct RUNNING_AVE* p, int32_t x);
/* @brief	: Reset average to some initial value (e.g. zero!)
 * @param	: p = pointer to struct with pointers etc.
 * @param	: x = initial reading NOT SCALED
 * @return	: SCALED average;
*******************************************************************************/
int64_t running_average(struct RUNNING_AVE* p, int32_t x);
/* @brief	: Compute the running average
 * @param	: p = pointer to struct with pointers etc.
 * @param	: x = new reading NOT SCALED
 * @return	: SCALED average;
*******************************************************************************/

#endif
