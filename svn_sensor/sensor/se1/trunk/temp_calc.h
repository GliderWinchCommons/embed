/******************************************************************************
* File Name          : temp_calc2.h
* Date First Issued  : 04/19/2014,04/04/2018
* Board              : 
* Description        : Computer deg C from filtered thermistor reading
*******************************************************************************/

#ifndef __TEMP_CALC2
#define __TEMP_CALC2

#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"

 // Thermistor parameters for converting ADC readings to temperature
  struct THERMPARAM
 {   //                   	   default values    description
	float B;       //	3380.0 // Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;      //	10.0	 // Series resistor, fixed (K ohms)
	float R0;      //	10.0	 // Thermistor room temp resistance (K ohms)
	float TREF;    //	298.0	 // Reference temp for thermistor
	float offset;  // 0.0	 // Therm temp correction offset	1.0 Therm correction scale
	float scale;   // 1.0	 // Therm temp correction scale	1.0 Therm correction scale

/* **************************************************************************************/
 double temp_calc(struct THERMPARAM *p uint32_t adc);
/* @brief	: Compute temperature
 * @param	: p = pointer to struct of constants/parameters
 * @param	: adc = adc reading
 * @return	: Degrees Centigrade
 * ************************************************************************************** */


#endif 

