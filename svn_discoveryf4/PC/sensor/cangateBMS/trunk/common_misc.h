/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : common_misc.h
* Hackerees          : deh
* Date First Issued  : 12/27/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : includes "common" for all sensor programs
*******************************************************************************/

#ifndef __COMMON_MISC_SENSOR
#define __COMMON_MISC_SENSOR/* This struct is used to return a pointer and count from a "get packet" routine */

/* #include "usartallproto.h" */
#include "common_can.h"

struct PKT_PTR
{
	char*		ptr;	// Pointer to packet
	unsigned int	ct;	// Byte count of packet
};

/* Different ways of dealing with an int */
union UI_I_C
{
	u32			u32;
	s32			s32;
	unsigned long		ul;
	signed long		sl;
	unsigned int		ui;
	signed int		n;
	unsigned short		us[2];
	signed short		s[2];
	unsigned char		uc[4];
	signed char		c[4];
};


/* Use this for passing two 32b "things" */
struct TWO32
{
	union UI_I_C	a;
	union UI_I_C	b;
};

#endif 

