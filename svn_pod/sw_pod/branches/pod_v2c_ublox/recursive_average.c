/******************************************************************************
* File Name          : recursive_average.c
* Date First Issued  : 06/11/2017
* Board              : 
* Description        : Compute the average recursively
*******************************************************************************/

#include "recursive_average.h"

/******************************************************************************
 * void recursive_average_reset(struct RECURSIVE_AVE* p);
 * @brief	: Reset averaging
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @return	: none; struct is initialized
*******************************************************************************/
void recursive_average_reset(struct RECURSIVE_AVE* p)
{
	p->n = 0;
	p->xbnp1 = 0;
	return;
}
/******************************************************************************
 * int64_t recursive_average(struct RECURSIVE_AVE* p, int x);
 * @brief	: Compute the average recursively
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @param	: x = new reading NOT SCALED
 * @return	: SCALED average; also struct is updated during this computation
*******************************************************************************/
/*
http://math.stackexchange.com/questions/374881/recursive-formula-for-variance
*/
int64_t recursive_average(struct RECURSIVE_AVE* p, int x)
{
/*
	int64_t xnp1;	// x(n+1)	// x(n + 1)
	int64_t xbnp1;	// xbar(n+1)	// xave(n + 1)
	int64_t xbn	// xbar(n)	// xave(n)
	uint32_t n;	// Count (after initialization)
	uint16_t scale;	// Scale factor (bit shift count)
*/
	/* Instead of floating pt, used scaled integers */
	p->xnp1 = x << p->scale; // x(n+1) = x	

	if (p->n != 0)
        {
 		/* New (n+1) average (xbar), i.e. xbar(n+1) */
		p->xbnp1 = p->xbn + ((p->xnp1 - p->xbn) / (p->n+1));

		p->xbn = p->xbnp1;	// Update
	}	
	else
	{
		p->xbn = p->xnp1;
	}
	
	p->n += 1;
	return p->xbn;
	
}
