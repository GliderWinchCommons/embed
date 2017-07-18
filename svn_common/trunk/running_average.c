/******************************************************************************
* File Name          : running_average.c
* Date First Issued  : 06/11/2017
* Board              : 
* Description        : Compute the average recursively
*******************************************************************************/

#include "running_average.h"

/******************************************************************************
 * int64_t* running_average_init(struct RUNNING_AVE* p, uint16_t n, uint16_t scale);
 * @brief	: Get memory and init for averaging
 * @param	: p = pointer to struct with pointers etc.
 * @param	: n = number of elements in the average
 * @param	: scale = bit shift count for scaling
 * @return	: pointer malloc'ed array
*******************************************************************************/
int64_t* running_average_init(struct RUNNING_AVE* p, uint16_t n, uint16_t scale)
{
	p->N = n;		// Number to average
	p->n = 1;		// Working count
	p->sum = 0; 		// Always start with zero
	p->p0 = calloc(n, sizeof(int64_t)); // Get array 
	p->pn = p->p0;		// Working pointer init
	p->pend = p->p0 + p->N;	// End of array pointer
	p->scale = scale;

	return p->p0;	// If this is NULL calloc didn't work
}
/******************************************************************************
 * int64_t running_average_reset(struct RUNNING_AVE* p, int32_t x);
 * @brief	: Reset average to some initial value (e.g. zero!)
 * @param	: p = pointer to struct with pointers etc.
 * @param	: x = initial reading NOT SCALED
 * @return	: SCALED average;
*******************************************************************************/
int64_t running_average_reset(struct RUNNING_AVE* p, int32_t x)
{
	int i;
	int64_t tmp = x;
	p->pn = p->p0;
	p->n = 1;
	for (i = 0; i < p->N; i++)
	{
		*p->pn = tmp;
	}
	p->sum = p->N * tmp;
	return (tmp << p->scale);
}
/******************************************************************************
 * int64_t running_average(struct RUNNING_AVE* p, int32_t x);
 * @brief	: Compute the running average
 * @param	: p = pointer to struct with pointers etc.
 * @param	: x = new reading NOT SCALED
 * @return	: SCALED average;
*******************************************************************************/
int64_t running_average(struct RUNNING_AVE* p, int32_t x)
{
/*
	int64_t *p0;	// Pointer to beginning of array
	int64_t *pend;	// p0 + n
	int64_t *pn;	// Pointer to element		
	int64_t sum;	// Running sum
	uint16_t n;	// Working count
	uint16_t N;	// Number in array
	uint16_t scale;	// Scale factor (bit shift count)

*/
	int64_t new = (int64_t)x << p->scale;	// Scale up new value
	int64_t tmp;
	
	p->sum  -= *(p->pn);	// Remove oldest value from sum
	p->sum  += new;		// Add new value to sum
	*(p->pn) = new;		// Save new value
	p->pn++;		// Step to next array position
	if (p->pn >= p->pend) p->pn = p->p0; // Wrap around

	/* Take care of startup */
	if (p->n >= p->N)
	{ // Here, Array has been filled
		return p->sum / p->N; // Return scaled average
	}
	else
	{ // Here array has not been filled
		tmp = p->sum / p->n; // Return scaled average
		p->n += 1;
		return tmp;
	}	
}

