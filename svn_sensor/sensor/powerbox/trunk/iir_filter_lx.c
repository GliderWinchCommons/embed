/******************************************************************************
* File Name          : iir_filter_lx.c
* Date First Issued  : 05/18/2016
* Board              : --
* Description        : Single pole IIR filter using int32_t
*******************************************************************************/
/*
Simple single pole IIR stage.

For pass-thru, where the output = input, i.e no filtering, set k=1.

*/

#include "iir_filter_lx.h"
/******************************************************************************
 * static void iir_filter_lx_init(struct IIRFILTERL* pfil, int32_t* pval, int32_t scale);
 * @brief	: Set initial value
 * @param	: pfil = pointer to struct with filter stuff
 * @param	: pval = pointer to reading
*******************************************************************************/
static void iir_filter_lx_init(struct IIRFILTERL* pfil, int32_t* pval)
{
	/* Crazy results if scale or k is bogus. */
	if (pfil->pprm->scale <= 0) pfil->pprm->scale = 1;
	if (pfil->pprm->k <= 0) pfil->pprm->k = 1;

	/* Set initial value with the first reading. */
	pfil->z = (*pval) * (pfil->pprm->scale);
	return;
}
/******************************************************************************
 * void iir_filter_lx_do(struct IIRFILTERL* pfil, int32_t* pval);
 * @brief	: Pass an input value through the filter.
 * @param	: pfil = pointer to struct with filter stuff
 * @param	: pval = pointer to reading
*******************************************************************************/
/*
NOTE: It is expected that pfil-z has been initialized.
*/
void iir_filter_lx_do(struct IIRFILTERL* pfil, int32_t* pval)
{
	/* First time with reading. */
	if (pfil->sw == 0)
	{
		pfil->sw = 1;
		iir_filter_lx_init(pfil, pval);
	}

// NOTE: 'lx' version skips this conversion
	/* Filter computation */
   pfil->z = pfil->z + ( (*pval) * (pfil->pprm->scale) - pfil->z) / (pfil->pprm->k); 
//	pfil->d_out = pfil->z / (pfil->pprm->scale);
	return;

}
/******************************************************************************
 * void iir_filter_lx_double(struct IIRFILTERL* pfil);
 * @brief	: Convert unscaled int to scaled double
 * @param	: pfil = pointer to struct with filter stuff
*******************************************************************************/
void iir_filter_lx_double(struct IIRFILTERL* pfil)
{
	pfil->d_out = pfil->z;	// int to double
	pfil->d_out = pfil->z/(pfil->pprm->scale); // Scale
	return;
}
/******************************************************************************
 * void iir_filter_lx_float(struct IIRFILTERL* pfil);
 * @brief	: Convert unscaled int to scaled float
 * @param	: pfil = pointer to struct with filter stuff
*******************************************************************************/
void iir_filter_lx_float(struct IIRFILTERL* pfil)
{
	pfil->f_out = pfil->z;	// int to float
	pfil->f_out = pfil->f_out/(pfil->pprm->scale); // Scale
	return;
}

