/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : SYSTICK_24bitdiff.c
* Hackor             : deh
* Date First Issued  : 10/21/2010
* Description        : SYSTICK setup and related
*******************************************************************************/
#include "../libopenstm32/systick.h"
#include "../libmiscstm32/systick1.h"

/*******************************************************************************
 * u32 SYSTICK_24bitdiff(u32 new_systick, u32 old_systick);
 * @brief	: Gets 32 bit systick count and returns difference from old
 * @param	: A previous systick reading
 * @return	: Return difference with wrap-around adjustment
 *******************************************************************************/
u32 SYSTICK_24bitdiff(u32 new_systick, u32 old_systick)
{
	s32 temp = (old_systick - new_systick);

	if (temp > 0) return temp;	// No wrap.
	return (temp + STK_LOAD +1);	// Adjust for wrap=around
}

