/******************************************************************************
* File Name          : recursive_variance_flt.h
* Date First Issued  : 08/04/2017
* Board              : 
* Description        : Compute the variance recursively
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RECURSIVE_VARIANCE_FLT
#define __RECURSIVE_VARIANCE_FLT

#include <stdint.h>

struct RECURSIVE_FLT
{
	float xnp1;	// x(n+1)	// x(n + 1)
	float xn;	// x(n)		// x(n)
	float xbnp1;	// xbar(n+1)	// xave(n + 1)
	float xbn;	// xbar(n)	// xave(n)
	float snp1;	// sigma(n+1)	// variance(n + 1)
	float sn;	// sigma(n)	// variance(n)
	uint32_t n;	// Count (after initialization)
	uint32_t oto;	// Startup countdown counter
};

/******************************************************************************/
void recursive_variance_flt(struct RECURSIVE_FLT* p, float x);
/* @brief	: Compute the variance recursively
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @param	: x = new reading
 * @return	: struct is updated during this computation
*******************************************************************************/
void recursive_variance_reset_flt(struct RECURSIVE_FLT* p);
/* @brief	: Reset averaging
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @return	: struct is updated during this computation
*******************************************************************************/


#endif
