/* *******************************************************************************
latlongdistance.c
Difference between to lat/long ponts (at same height)
 ********************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/*
Compute the distance between to two lat/long points.
https://stackoverflow.com/questions/507819/pi-and-accuracy-of-a-floating-point-number

pipe input into routine
gcc latlongdistance.c -o latlongdistance -lm && cat latlongdistance.dat | ./latlongdistance
*/

double d2r = 3.14159265358979323846/180.0;	// Deg to radians

//calculate haversine distance for linear distance

double haversine_mi(double lat1, double long1, double lat2, double long2)
{
    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = sin(dlat/2.0) * sin(dlat/2.0) + cos(lat1*d2r) * cos(lat2*d2r) * sin(dlong/2.0) * sin(dlong/2.0);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 3956 * c; 

    return d;
}

double dlat1;
double dlon1;
double dlat2;
double dlon2;

#define LINESIZE 1024
char buf[LINESIZE];

int main(int argc, char *argv[])
{
	double dx;
	double dlat;
	double dlon;
	int sw = 0;
	int i = 0;

	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		sscanf(buf,"%lf %lf",&dlat,&dlon);
		if (sw == 0)
		{
			sw = 1;
			dlat1 = dlat;
			dlon1 = dlon;
			dlat2 = dlat;
			dlon2 = dlon;
		}
		else
		{
			dlat2 = dlat;
			dlon2 = dlon;
		}
		dx = haversine_mi(dlat1,dlon1,dlat2,dlon2);
		printf("%2d %12.7f %12.7f\t",i++,dlat,dlon);
		printf("%12.7f miles\t\t%12.1f feet\n",	dx, dx * 5280);

	}

	return 0;
}
