/* polycal.c */

/*
# polycompile
gcc $1.c -o $1 polreg_rosetta.c -lm -lgsl -lgslcblas

# polyexecute
LD_LIBRARY_PATH=/home/deh/gsl/gsl-2.7.1/.libs
export LD_LIBRARY_PATH
./$1


Readings input line example (last line is polynomial degree)--
18.5056     15098.0 14413.9 14390.8 14393.1 14401.0 14405.1 14414.1 14382.5 14405.2 14416.5 14401.3 14393.1 14391.2 14385.9 14390.5 14382.7
22.6908     18291.0 17686.0 17657.3 17659.6 17669.8 17676.6 17686.8 17645.7 17675.1 17688.4 17668.6 17661.4 17657.0 17649.9 17657.2 17647.3 
... snipped ...
59.522      46263.9 46505.3 46439.0 46448.8 46486.0 46488.8 46517.2 46410.8 46502.5 46539.0 46466.2 46448.0 46439.4 46410.9 46435.5 46436.9
58.492      45464.0 45706.4 45638.8 45652.9 45684.2 45692.1 45720.8 45615.1 45697.2 45743.6 45670.7 45647.7 45635.9 45621.1 45637.8 45636.5 
 3
 
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

#define NUMIN 128  // Max number of input points
double voltin[NUMIN];    // TOS voltage for readings
double adcin[16][NUMIN]; // Copy & paste of IIR line from PC
double volta[16][NUMIN]; // Adjusted for resistor calibration tap volts

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

  /* Get voltage divider calibration from file on command line. */
	if ( (fpIn = fopen (argv[1],"r")) == NULL)
	{
		printf ("\nInput file did not open: %s\n",argv[1]); 
		return -1;
	}

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

	// Bogus check
	if (cn != 16)
	{
		printf("Resistor calibration data count was %d not 16\n",cn);
	}

	// Output for checking
	printf("\nResistor calibration data table\n");
	for (i = 0; i < cn; i++)
	{
		printf("%2i %10.6f %10.6f %11.8E\n",i+1,volttos[i],voltres[i],resratio[i]);
	}

	/* Read data file from STDIN. */
  	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (strlen(buf) > 12) // Data versus parameter separation
		{ // Here, assume tos voltage plus 16 adc readings on the line
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
		{ // Here, it must not be a line with readings.
			sscanf(&buf[0],"%i",&degree);
			if ((degree == 0) || (degree > 5))
			{
				printf ("\n\n DEGREE WARNING %d",degree);
			}
			printf ("\nPolynomial degree %d\n",degree);
			break; // End input here
 		}
	}

	/* Dump input data for checking. */	
	printf("\nCell calibration input data");
	for (j = 0; j < np; j++)
	{
		printf ("\n%2d %9.5f",j+1,voltin[j]);
		for (i = 0; i < 16; i++)
			printf ("%9.1f", adcin[i][j]);	
	}

	/* Apply resistor calibration and set cell voltages. */
	printf("\n\n Resistor calibration adjusted voltage\n"
		"       TOS   TOS/16");
	for (j = 0; j < 16; j++) printf("%9d", j+1);

	
	for (i = 0; i < np; i++)
	{
		for (j = 0; j < 16; j++)
		{
			volta[j][i] = voltin[i] * resratio[j];
		}
	}

	/* Output for checking. */
	for (i = 0; i < np; i++)
	{
		printf ("\n%10.4f%9.5f:", voltin[i], voltin[i]/16);
		for (j = 0; j < 16; j++)
		{
			printf("%9.5f",volta[j][i]);
		}
	}
	printf("\n");

	/* Compute polynomial fit for each cell. */
	for (j = 0; j < 16; j++)
	{
		printf ("\n/* Cell #%2i */\n", j+1);		
		polynomialfit(np, degree, &adcin[j][0], &volta[j][0],coeff);
  		for(i=0; i < degree; i++) 
  		{
    		printf("p->cabsbms[%2d].coef[%0d] = %10.7Ef;\n",j,i,coeff[i]);
	   	}
  	}

	printf ("\n");
  return 0;
}