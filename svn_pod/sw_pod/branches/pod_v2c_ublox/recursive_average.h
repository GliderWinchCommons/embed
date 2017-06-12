/******************************************************************************
* File Name          : recursive_average.h
* Date First Issued  : 06/11/2017
* Board              : 
* Description        : Compute the average recursively
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RECURSIVE_VAR
#define __RECURSIVE_VAR

#include <stdint.h>

struct RECURSIVE_AVE
{
	int64_t xnp1;	// x(n+1)	// x(n + 1)
	int64_t xbnp1;	// xbar(n+1)	// xave(n + 1)
	int64_t xbn;	// xbar(n)	// xave(n)
	uint32_t n;	// Count
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
