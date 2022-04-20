/* polycal.c */

/*
# polycompile
gcc $1.c -o $1 polreg_rosetta.c -lm -lgsl -lgslcblas

# polyexecute
LD_LIBRARY_PATH=/home/deh/gsl/gsl-2.7.1/.libs
export LD_LIBRARY_PATH
./$1

*/


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
 
#include "polifitgsl.h"
 
#define NP 11
double x[] = {0,  1,  2,  3,  4,  5,  6,   7,   8,   9,   10};
double y[] = {1,  6,  17, 34, 57, 86, 121, 162, 209, 262, 321};
 
#define DEGREE 3
double coeff[DEGREE];

#define NUMIN 128  // Max number of input points
double voltin[NUMIN];
double adcin[16][NUMIN];
int degree; // Polynomial degree
uint32_t np;

#define LINESZ 512
char buf[LINESZ]; // Input line 

FILE* fpIn;

/* Resistor calibration */
double volttos[16]; // Voltage at top-of-stack
double voltres[16]; // Voltage of cell tap
double resratio[16];
 
int main(int argc, char **argv)
{
  int i,j;
  int cell;
  uint32_t np = 0; // Number of input points
  uint32_t cn = 0;
  double vratio;



  /* Get voltage divider calibration. */
	if ( (fpIn = fopen (argv[1],"r")) == NULL)
	{
		printf ("\nInput file did not open: %s\n",argv[1]); 
		return -1;
	}

	/* Load table */
	while ( (fgets (&buf[0],LINESZ,fpIn)) != NULL)	// Get a line
	{
		if (strlen(buf) < 21) continue;

		sscanf(buf,"%d %lf %lf %lf",&cell,&volttos[cn],&voltres[cn],&vratio);
		if ((cn+1) != cell)
		{
			printf("\nInput file %s Unexpected cell/line number %d should be %d\n",argv[1],cell,(cn+1));
			return -1;
		}
		resratio[cn] = voltres[cn] / volttos[cn];
		cn += 1;
	}
	if (cn != 16)
	{
		printf("Resistor calibration data count was %d not 16\n",cn);
	}

	printf("\nResistor calibration data table\n");
	for (i = 0; i < cn; i++)
	{
		printf("%2i %10.6f %10.6f %11.8E\n",i+1,volttos[i],voltres[i],resratio[i]);
	}

  	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (strlen(buf) > 12) // Data versus parameter separation
		{
//printf("IN : %s",&buf[0]);
			sscanf(&buf[0],"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ",
				&voltin[np],
				&adcin[ 0][np],&adcin[ 1][np],&adcin[ 2][np],&adcin[ 3][np],
				&adcin[ 4][np],&adcin[ 5][np],&adcin[ 6][np],&adcin[ 7][np],
				&adcin[ 8][np],&adcin[ 9][np],&adcin[10][np],&adcin[11][np],
				&adcin[12][np],&adcin[13][np],&adcin[14][np],&adcin[15][np]);
			np += 1;
		}
		else
		{
			sscanf(&buf[0],"%i",&degree);
			if ((degree == 0) || (degree > 5))
			{
				printf ("\n\n DEGREE WARNING %d",degree);
			}
			printf ("\nPolynomial degree %d\n",degree);
			break; // End input here
 		}
	} 		
	printf("\nCell calibration input data");
	for (j = 0; j < np; j++)
	{
		printf ("\n%2d %9.5f",np,voltin[j]);
		for (i = 0; i < 16; i++)
			printf ("%9.1f", adcin[i][j]);	
	}

	/* Compute polynomial fit for each cell. */
	for (j = 0; j < 16; j++)
	{
		printf ("\n/* Cell #%2i */\n", j+1);		
		polynomialfit(np, degree, &adcin[j][0], voltin,coeff);
  		for(i=0; i < degree; i++) 
  		{
    		printf("p->cabsbms[%2d].coef[%0d] = %10.7Ef;\n",j,i,coeff[i]);
	   	}
  	}

	printf ("\n");
  return 0;
}