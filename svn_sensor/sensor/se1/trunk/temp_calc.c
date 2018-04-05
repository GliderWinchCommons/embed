/******************************************************************************
* File Name          : temp_calc2.c
* Date First Issued  : 04/19/2014,04/04/2018
* Board              : 
* Description        : Computer deg C from filtered thermistor reading
*******************************************************************************/

#include <math.h>
#include "adcsensor_eng.h"
#include "temp_calc.h"

/* **************************************************************************************
 * double temp_calc(struct THERMPARAM *p uint32_t adc);
 * @brief	: Compute temperature
 * @param	: p = pointer to struct of constants/parameters
 * @param	: adc = adc reading
 * @return	: Degrees Centigrade
 * ************************************************************************************** */
double temp_calc2(struct THERMPARAM *p uint32_t adc)
{
	double dreading;	// Input reading
	double r;	      // Thermistor resistance
	double ratio;     // R/Ro
	double Oh;
	double T;

double B	 = p->therm.B;  // Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
double RS = p->therm.RS; // Series resistor, fixed (K ohms)
double RO = p->therm.R0;       // Thermistor room temp resistance (K ohms)
double TREF	= p->therm.TREF;// Reference temp for thermistor

	dreading = adc; // Convert ADC reading to double

	/* Compute thermistor resistance from adc reading */
	r = (dreading * p->therm.RS) / (4095.9 - reading);

	/* Ratio of thermistor resistance to thermistor reference resistance */
	ratio = r / RO;	

	/* Compute formula */
	Oh = log(ratio) + (B / TREF);
	T = B / Oh;
	T = (T - 273.0) * 100.0; // Convert Kelvin to Centigrade and scale upwards
				
	adc_calib_temp = T;	// Convert to fixed pt (var defined in 'adcsensor_eng.c')
	adc_temp_flag = 2;	// Show adcsensor.c a value is ready to send on CAN bus.

	return T;
}

