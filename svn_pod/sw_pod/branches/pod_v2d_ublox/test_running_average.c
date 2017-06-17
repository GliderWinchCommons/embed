/* 
 * test_running_average.h 
 * 06/11/2017
*/

// Compile
// gcc test_running_average.c running_average.c -o test_running_average

#include <stdio.h>
#include "running_average.h"

void main (void)
{
	struct RUNNING_AVE rave;
	
	
	int64_t ret;
	int i;

	#define N 20
	#define OFFSET 48001370
	#define SCALE 16

	int x[N];

	running_average_init(&rave, N, SCALE);

	for (i = 0; i < N; i++)
	{
   		x[i] = (i+1) + OFFSET;
	}

	double sum = 0;
	double dret;
	for (i = 0; i < N; i++)
	{
		ret = running_average(&rave, x[i]);
		dret = ret;
		sum += x[i];
		printf("%3d, %6d, %10.3f, %16ld %16ld, %10.4f",i,x[i], sum/(i+1), ret, ret/(uint64_t)(1<<SCALE), dret/(uint64_t)(1<<SCALE));
		printf("\n");
	}

	return;
}
