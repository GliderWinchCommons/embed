/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : se1.h
* Author             : deh
* Date First Issued  : 06/06/2013
* Board              : RxT6
* Description        : Sensor board with pressure, rpm, throttle pot, thermistor
*******************************************************************************/

#ifndef __SE1
#define __SE1

#include "../../../../svn_common/trunk/common_can.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_SE1	// Sensor board #1 with photosensor, loggable ID

/* Easy way for other routines to access via 'extern'*/
extern struct CAN_CTLBLOCK* pctl1;
extern struct CAN_CTLBLOCK* pctl0;	// Some routines use 0 for CAN1 others use 1 (sigh)


#endif 

