/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_time_linux.c
* Hackeroos          : deh
* Date First Issued  : 10/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert gps|rtc time ticks to linux time format
*******************************************************************************/

#include <time.h>
#include "gps_time_convert.h"


static int ascii_bin(char * p);

/******************************************************************************
 * time_t gps_time_linux_init(struct TIMESTAMPG * pts);
 * @brief	: Convert gps|rtc time ticks to linux time format
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = Extracted RTC_CNT following EOL
 ******************************************************************************/
time_t gps_time_linux_init(struct TIMESTAMPG * pts)
{
	struct tm t;

	/* Fill tm struct with values extracted from gps */
	t.tm_sec =	 ascii_bin(&pts->g[ 4]);
	t.tm_min = 	 ascii_bin(&pts->g[ 2]);
	t.tm_hour =	 ascii_bin(&pts->g[ 0]);
	t.tm_mday =	 ascii_bin(&pts->g[ 6]);
	t.tm_mon =	 ascii_bin(&pts->g[ 8]) - 1;
	t.tm_year =	 ascii_bin(&pts->g[10]) + (2000 - 1900);
	t.tm_isdst = 0;		// Be sure daylight time is off
	return mktime (&t);	// Convert to time_t format
}
/******************************************************************************
 * static void ascii_bin(char * p);
 * @brief	: Convert two ascii to binary
 * @param	: pointer to high order ascii
 * @return	: binary number 0 - 99
 ******************************************************************************/

static int ascii_bin(char * p)
{
	int temp;
	temp = *p++ - '0';
	return (10 * temp) + *p - '0';
}
