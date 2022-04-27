/* polycal.c */

/*
This requires the gsl library to be installed:
https://www.gnu.org/software/gsl/

# polycompile
gcc $1.c -o $1 polreg_rosetta.c -lm -lgsl -lgslcblas

# polyexecute
LD_LIBRARY_PATH=/home/deh/gsl/gsl-2.7.1/.libs
export LD_LIBRARY_PATH
./$1


Readings input line example:

 3

 Sample command line to compile and execute--

 ./polycompile polycal && ./polyexecute polycal < cal_1_therm_220426.txt
 
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h> 
#include "polifitgsl.h"

double coeff[16];

#define NUMIN 256  // Max number of input points
double tempin[NUMIN];    // Temperature
double adcin[3][NUMIN]; // Copy & paste of IIR line from PC

int degree; // Polynomial degree
uint32_t np;

#define LINESZ 512
char buf[LINESZ]; // Input line 
 
int main(int argc, char **argv)
{
  int i,j;
  int cell;
  uint32_t np = 0; // Number of input points
  uint32_t cn = 0;
  double vratio;
  double tmptemp = -465.0;
  int len;

	/* Read data file from STDIN. */
  	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		len = strlen(buf);
		if ((len > 4) && (len < 12))
		{ // Here, new temperature
			sscanf(&buf[0],"%lf",&tmptemp);
			continue;
		}
		if (strlen(buf) >= 12) // Data versus parameter separation
		{ // Here, assume tos voltage plus 16 adc readings on the line
//printf("IN : %s",&buf[0]);
			sscanf(&buf[0],"%lf %lf %lf",
				&adcin[ 0][np],&adcin[ 1][np],&adcin[ 2][np]);
			tempin[np] = tmptemp;
			np += 1;
			continue;
		}
		if (len > 2)
		{ // Here, it must be the polynomial degree
			sscanf(&buf[0],"%i",&degree);
			if ((degree == 0) || (degree > 5))
			{
				printf ("\n\n DEGREE WARNING %d",degree);
			}
			printf ("\nPolynomial degree %d\n",degree);
			break; // End input here
 		}
	}

	printf ("\nPolynomial degree %d  Number pts %d\n",degree, np);

	/* Dump input data for checking. */	
	printf("\nCell calibration input data");
	for (j = 0; j < np; j++)
	{
		printf ("\n%3d %9.5f",j+1,tempin[j]);
		for (i = 0; i < 3; i++)
			printf ("%9.1f", adcin[i][j]);	
	}


	printf("\n");

	/* Compute polynomial fit for each cell. */
	for (j = 0; j < 3; j++)
	{
		{
			printf ("\n/* Thermistor #%2i */\n", j+1);		
		}
		polynomialfit(np, degree, &adcin[j][0], &tempin[0],coeff);

  		for(i=0; i < degree; i++) 
  		{
	   		printf("p->cabsbms[%2d].coef[%0d] = %10.7Ef;\n",j+16,i,coeff[i]);
	   	}
  	}

	printf ("\n");
  return 0;
}