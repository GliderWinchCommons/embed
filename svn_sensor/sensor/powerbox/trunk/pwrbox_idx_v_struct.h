/******************************************************************************
* File Name          : pwrbox_idx_v_struct.h
* Date First Issued  : 07/15/2015,06/13/2018
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
These structs serve all 'Tension_a" function types (which is the one used with
AD7799 on the POD board).
*/

#include <stdint.h>
#include "common_can.h"
#include "iir_filter_l.h"

#ifndef __PWRBOX_IDX_V_STRUCT
#define __PWRBOX_IDX_V_STRUCT

#define NIIR	4	// Number of IIR filters

/*
// IIR filter (int) parameters
struct IIR_L_PARAM
{
	int32_t k;		// Filter parameter for setting time constant
	int32_t scale;		// Scaling improve spare bits with integer math
};
*/

/* The parameter list supplies the CAN IDs for the hardware filter setup. */
#define CANFILTMAX	8	// Max number of CAN IDs in parameter list

#define NUMADCPARAM	8	// Number of ADC for which we use parameters

 // Thermistor parameters for converting ADC readings to temperature
  struct THERMPARAM
 {   //                   	   default values    description
	float B;		//	3380.0	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;		//	10.0	// Series resistor, fixed (K ohms)
	float R0;		//	10.0	// Thermistor room temp resistance (K ohms)
	float TREF;		//	298.0	// Reference temp for thermistor
	float offset;		//      0.0	// Therm temp correction offset	1.0 Therm correction scale
	float scale;		//      1.0	// Therm temp correction scale	1.0 Therm correction scale
 };

struct ADCCALPWRBOX
{
	double	offset;	//	Offset
	double	scale;	//	Scale
};
 
// Naming convention--"cid" - CAN ID
 struct PWRBOXLC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	uint32_t hbct;			// Heartbeat ct: ticks between sending msgs
	struct ADCCALPWRBOX adc[NUMADCPARAM];	// ADC measurements
	struct IIR_L_PARAM iir[NIIR];	   // IIR Filter for IIR filters 
	uint32_t cid_heartbeat; // CANID-Heartbeat msg
	uint32_t cid_pwr_msg;   // CANID-voltage msg
	uint32_t cid_pwr_alarm; // CANID-low voltage alarm msg
   uint32_t alarm_rate;    // Time between alarm msgs (ms)
   float  alarm_thres;     // Voltage threshold for low volt alarm msgs    
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
 };
 
/* **************************************************************************************/
int pwrbox_idx_v_struct_copy(struct PWRBOXLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
#endif
