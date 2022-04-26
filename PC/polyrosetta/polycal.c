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
Type Multimeter into 1st entry, cut & paste cells, thermistors, and tos from minicom output. Delete the 3 thermistor readings.
(last line is polynomial degree)
TOP V     1       2       3       4       5       6       7       8       9       10      11      12      13      14      15      16        TOS
31.349  24525.5 24467.5 24427.1 24431.3 24446.5 24453.4 24468.6 24411.6 24454.8 24473.4 24441.6 24432.5 24428.9 24412.6 24426.4 24419.6    24752.5
14.1285 11092.1 11009.3 10992.4 10993.9 10999.8 11002.8 11011.4 10985.5 11001.9 11012.4 10997.7 10993.5 10992.7 10986.9 10990.7 11105.0    11148.8
... snip ...
60.506  47406.1 47274.1 47208.3 47217.8 47252.2 47256.0 47285.7 47175.8 47264.0 47313.7 47236.6 47215.9 47204.6 47176.8 47203.2 47209.1    47826.7
52.625  41238.5 41128.1 41065.1 41069.0 41103.8 41110.1 41133.1 41038.4 41117.5 41161.8 41088.6 41072.1 41070.1 41040.1 41065.3 41064.0    41605.5
 3

 Sample command line to compile and execute--

 ./polycompile polycal && ./polyexecute polycal cal_voltdivider.txt < cal_1_220425.txt
 
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
double adcin[17][NUMIN]; // Copy & paste of IIR line from PC
double volta[17][NUMIN]; // Adjusted for resistor calibration tap volts

int degree; // Polynomial degree
uint32_t np;

#define LINESZ 512
char buf[LINESZ]; // Input line 

FILE* fpIn;

/* Resistor calibration */
double volttos[17]; // Voltage at top-of-stack
double voltres[17]; // Voltage of cell tap
double resratio[17];
 
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

	/* Dummy resistor ratio for top-of-stack. */
	resratio[cn] = 1; 
	voltres[cn] = 1;
	volttos[cn] = 1;
	cn += 1;

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
			sscanf(&buf[0],"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ",
				&voltin[np],
				&adcin[ 0][np],&adcin[ 1][np],&adcin[ 2][np],&adcin[ 3][np],
				&adcin[ 4][np],&adcin[ 5][np],&adcin[ 6][np],&adcin[ 7][np],
				&adcin[ 8][np],&adcin[ 9][np],&adcin[10][np],&adcin[11][np],
				&adcin[12][np],&adcin[13][np],&adcin[14][np],&adcin[15][np],
				&adcin[16][np]);
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
	for (j = 0; j < 17; j++) printf("%9d", j+1);

	
	for (i = 0; i < np; i++)
	{
		for (j = 0; j < 17; j++)
		{
			volta[j][i] = voltin[i] * resratio[j];
		}
	}

	/* Output for checking. */
	for (i = 0; i < np; i++)
	{
		printf ("\n%10.4f%9.5f:", voltin[i], voltin[i]/16);
		for (j = 0; j < 17; j++)
		{
			printf("%9.5f",volta[j][i]);
		}
	}
	printf("\n");

	/* Compute polynomial fit for each cell. */
	for (j = 0; j < 17; j++)
	{
		if (j == 16)
		{
			printf ("\n/* Top-of-stack */\n");		
		}
		else
		{
			printf ("\n/* Cell #%2i */\n", j+1);		
		}
		polynomialfit(np, degree, &adcin[j][0], &volta[j][0],coeff);

  		for(i=0; i < degree; i++) 
  		{
  			if (j == 16)
  			{ // Here, top-of-stack
    		printf("p->cabsbms[19].coef[%0d] = %10.7Ef;\n",i,coeff[i]);
  			}
  			else
  			{ // Here, cells
    		printf("p->cabsbms[%2d].coef[%0d] = %10.7Ef;\n",j,i,coeff[i]);
    		}
	   	}
  	}

	printf ("\n");
  return 0;
}