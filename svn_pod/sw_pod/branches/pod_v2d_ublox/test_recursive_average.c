/* test_recursive_average.h */

// Compile
// gcc test_recursive_average.c recursive_average.c -o test_recursive_average

#include <stdio.h>
#include "recursive_average.h"

void main (void)
{
	struct RECURSIVE_AVE rave;
	rave.scale = 16;
	
	int64_t ret;
	int i;

	#define N 20
	int x[N];
	#define OFFSET 1372

	recursive_average_reset(&rave);

	for (i = 0; i < N-10; i++)
	{
   		x[i] = (i+1) + OFFSET;
	}
	for (i = 10; i < N; i++)
	{
   		x[i] = -10 + OFFSET;
	}

	double sum = 0;
	double dret;
	for (i = 0; i < N; i++)
	{
		ret = recursive_average(&rave, x[i]);
		dret = ret;
		sum += x[i];
		printf("%3d, %6d, %10.3f, %16ld %16ld, %10.4f",i,x[i], sum/(i+1), ret, ret/(1<<rave.scale), dret/(1<<rave.scale));
		printf(" %6d %6d \n",rave.xbn, rave.xbnp1);
	}

	return;
}
