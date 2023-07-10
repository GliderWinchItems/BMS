/******************************************************************************
* File Name          : logilter.c
* Date First Issued  : 07/08/2023
* Board              : Linux PC
* Description        : Average & decimate BMS ascii cell readings
*******************************************************************************/
/*
07/08/2023 example with averaging length of 4 as argument--
gcc logfilter.c -o logfilter -lm && cat <file> | ./logfilter 4

Example input format, piped in--	
3881.9  3899.0  3884.6  3897.9  3879.7  3885.8  3895.6  3899.2  3896.1  3890.5  3881.9  3892.3  3895.5  3885.9  3897.5  3894.4  3893.4  3891.7

Procedure to convert CAN msgs to cell readings ascii file--
 General plan: feed CAN msgs to cangateBMS, via hub-server, and saving
   the console output of cangateBMS.

In terminal 1: setup a hub-server
  hub-server 127.0.0.1 32124 [be sure port isn't one in use]

In terminal 2: convert the CAN msg file to ascii lines using cangateBMS
  cd ~/GliderWinchItems/BMS/PC/logfilter
  cangateBMS 127.0.0.1 32125 | tee log070823.out
  d B1E00000 [blind type in command to display CAN msgs for given CAN ID]

In terminal 3: Send the CAN msg file to hub-server which sends it to cangateBMS
	nc localhost 32125 < ~/G*ems/BMS/log070823.CAN

The file log070823.out has the converted lines.

Run the filter routine that averages readings, e.g. average 4
  using the generated ascii file, and saving a file name of your choice
./logfilter 4 < log070823.out | tee mychoice.filt

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>


/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

#define NCELL 18

double sum[18];
double ave[18];

FILE* fpIn;
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;
	uint32_t id;
	uint32_t ctr = 0;
	uint32_t avectr = 0;
	uint32_t n = 8;
	uint32_t ctrout = 0;

	sscanf(*(argv+1),"%d",&n);

	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		ctr += 1;
		i = strlen(buf);
		if (i < 144)
			continue;

		sscanf(buf,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ",
			&sum[0],&sum[ 1],&sum[ 2],&sum[ 3],&sum[ 4],&sum[ 5],&sum[ 6],&sum[ 7],&sum[ 8],
			&sum[9],&sum[10],&sum[11],&sum[12],&sum[13],&sum[14],&sum[15],&sum[16],&sum[17]);

		for (i = 0; i < NCELL; i++)
		{
//			printf("%7.1f",sum[i]);

		}
//		printf("\n");

		for (i = 0; i < NCELL; i++)
		{
			ave[i] += sum[i];
		}
		avectr += 1;
		if (avectr >= n)
		{
			avectr = 0;

			for (i = 0; i < NCELL; i++)
				ave[i] /= n;

			printf("%7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf %7.2lf ",
			ave[0],ave[ 1],ave[ 2],ave[ 3],ave[ 4],ave[ 5],ave[ 6],ave[ 7],ave[ 8],
			ave[9],ave[10],ave[11],ave[12],ave[13],ave[14],ave[15],ave[16],ave[17]);

			for (i = 0; i < NCELL; i++)
				ave[i] = 0;

			printf("\n");
			ctrout += 1;
		}

	}
	printf("ave ctr             %6d\n",n);
	printf("Number input  lines %6d\n",ctr);
	printf("Number output lines %6d\n",ctrout);
		return 0;
}

