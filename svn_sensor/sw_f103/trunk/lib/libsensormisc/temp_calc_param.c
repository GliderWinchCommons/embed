/******************************************************************************
* File Name          : temp_calc_param.c
* Date First Issued  : 02/14/2015
* Board              : ...
* Description        : Computer deg C from filtered thermistor reading (double)
*******************************************************************************/
/*
04/19/2014 - temp_calc.c
02/15/2015 - temp_calc_param.c hacked '_calc.c to pass thermistor parameters

NOTE: This routine (and the .h) has to be in with the main program so that that the
tmpstruch.h that is generated on the fly by the compile script is available.

*/
#include <math.h>
#include "temp_calc_param.h"

// B	3380.0	 // Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
// RS	10.0	 // Series resistor, fixed (K ohms)
// RO	10.0	 // Thermistor room temp resistance (K ohms)
// TREF	298.0	 // Reference temp for thermistor

/* **************************************************************************************
 * void temp_calc_param_dbl_init(struct THERMPARAMDBL *pd, struct THERMPARAM *pf);
 * @brief	: Convert parameters from float to double 
 * @param	: doubles
 * @param	: floats
 * ************************************************************************************** */
void temp_calc_param_dbl_init(struct THERMPARAMDBL *pd, struct THERMPARAM *pf)
{
	pd->B    = pf->B;
	pd->RS   = pf->RS;
	pd->R0   = pf->R0;
	pd->TREF = pf->TREF;
	return;
}
/* **************************************************************************************
 * double temp_calc_param_dbl(int adcreading, struct THERMPARAMDBL *p);
 * @brief	: Compute temperature from adc reading ('d' = double precision)
 * @param	; adcreading,  scaled (0 - 4095) if filtered/averaged
 * @return	: Degrees Centigrade
 * ************************************************************************************** */
double temp_calc_param_dbl(int adcreading, struct THERMPARAMDBL *p)
{
	double r;	// Thermistor resistance
	double ratio;	// R/Ro
	double Oh;
	double T;

	/* Compute thermistor resistance from adc reading */
	r = (adcreading * p->RS) / (4095.9 - adcreading);

	/* Ratio of thermistor resistance to thermistor reference resistance */
	ratio = r / p->R0;	

	/* Compute formula */
	Oh = log(ratio) + (p->B / p->TREF);
	T = p->B / Oh;
	T = (T - 273.0); // Convert Kelvin to Centigrade
	return T;
}

