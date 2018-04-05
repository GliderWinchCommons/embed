/******************************************************************************
* File Name          : temp_calc_param.h
* Date First Issued  : 02/14/2015
* Board              : ...
* Description        : Computer deg C from filtered thermistor reading (double)
*******************************************************************************/

#ifndef __TEMP_CALC_PARAM
#define __TEMP_CALC_PARAM

#include "common_misc.h"
#include "common_can.h"
// 3/27/2018  #include "../../../../sensor/tension/trunk/tension_idx_v_struct.h"
 // Thermistor parameters for converting ADC readings to temperature
  struct THERMPARAM
 {   //                   	   default values    description
	float B;       //	3380.0 // Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;      //	10.0	 // Series resistor, fixed (K ohms)
	float R0;      //	10.0	 // Thermistor room temp resistance (K ohms)
	float TREF;    //	298.0	 // Reference temp for thermistor
	float offset;  // 0.0	 // Therm temp correction offset	1.0 Therm correction scale
	float scale;   // 1.0	 // Therm temp correction scale	1.0 Therm correction scale
 };
  struct THERMPARAMDBL
 {   //                   	   default values    description
	double B;       //	3380.0 // Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	double RS;      //	10.0	 // Series resistor, fixed (K ohms)
	double R0;      //	10.0	 // Thermistor room temp resistance (K ohms)
	double TREF;    //	298.0	 // Reference temp for thermistor
	double offset;  // 0.0	 // Therm temp correction offset	1.0 Therm correction scale
	double scale;   // 1.0	 // Therm temp correction scale	1.0 Therm correction scale
 };
/* **************************************************************************************/
 void temp_calc_param_dbl_init(struct THERMPARAMDBL *pd, struct THERMPARAM *pf);
/* @brief	: Convert parameters from float to double 
 * @param	: doubles
 * @param	: floats
 * **************************************************************************************/
double temp_calc_param_dbl(int adcreading, struct THERMPARAMDBL *p);
/* @brief	: Compute temperature from adc reading ('d' = double precision)
 * @param	; adcreading,  scaled (0 - 4095) if filtered/averaged
 * @return	: Degrees Centigrade
 * ************************************************************************************** */


#endif 

