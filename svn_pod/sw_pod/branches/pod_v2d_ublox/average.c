/******************************************************************************
* File Name          : running_average.c
* Date First Issued  : 06/11/2017
* Board              : 
* Description        : Compute the average recursively
*******************************************************************************/

#include "recursive_average.h"

/******************************************************************************
 * struct RUNNING_AVE* p running_average_init(struct RUNNING_AVE* p, int n);
 * @brief	: Get memory and init for averaging
 * @param	: n = number of elements in the average
 * @return	: pointer malloc array
*******************************************************************************/
struct RUNNING_AVE* p running_average_init(struct RUNNING_AVE* p, int n)
{
	p->N = n;	// Number to average
	p->sum = 0; 	// Sum runs forever so start at zero
	p->p0 = calloc(p->N, size_of(int));

	return p;
}
/******************************************************************************
 * int64_t recursive_average(struct RUNNING_AVE* p, int x);
 * @brief	: Compute the average recursively
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @param	: x = new reading NOT SCALED
 * @return	: SCALED average; also struct is updated during this computation
*******************************************************************************/
int64_t running_average(struct RUNNING_AVE* p, int x)
{
/*
	int *p0;	// Pointer to beginning of array
	int *p;		// Pointer to element		
	int64_t sum;	// Running sum
	uint16_t N;	// Number in array
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
