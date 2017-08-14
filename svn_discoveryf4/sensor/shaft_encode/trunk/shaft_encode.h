/******************************************************************************
* File Name          : shaft_encode.h
* Date               : 07/02/2017
* Board              : F4-Discovery
* Description        : Dual shaft encoder, CAN
*******************************************************************************/

#ifndef __SHAFT_ENCODE_H
#define __SHAFT_ENCODE_H

#include "../../../../svn_common/trunk/common_can.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_SE4	// Sensor board #3 with photosensor, loggable ID

/* USART|UART assignment for xprintf and read/write */
#define UXPRT	3	// Uart number for 'xprintf' messages
#define USTDO	2	// Uart number for gateway (STDOUT_FILE, STDIIN_FILE)

/* Easy way for other routines to access via 'extern'*/
extern struct CAN_CTLBLOCK* pctl0;	// CAN1 control block pointer
extern struct CAN_CTLBLOCK* pctl1;	// CAN2 control block pointer


#endif 

