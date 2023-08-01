/******************************************************************************
* File Name          : bridgemissing.c
* Date First Issued  : 07/31/2023
* Board              : Linux PC
* Description        : Bridge -1.0 cell readings is minuse last good reading
*******************************************************************************/
/*
07/31/2023 example--
gcc bridgemissing.c -o bridgemissing -lm && ./bridgemissing < ../../log230723discharge2000.B0E00000 tee ../../log230723discharge2000.B0E00000.clean

Example input format, piped in--	
First three lines
  -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0
  3568.0  3567.5  3567.2  3570.5  3570.1  3571.7  3567.7  3569.6  3570.4  3568.3  3571.4  3570.2  3568.8  3567.7  3566.8  3569.0  3569.0  3567.3
  3567.9  3567.3  3567.0  3570.8  3570.1  3571.8  3567.7  3569.5  3570.1  3568.7  3571.5  3570.1  3568.8  3567.6  3566.9  3569.5  3569.1  3567.3

Middle section a number of missing CAN msgs--
  3319.5  3320.3  3321.0  3323.7  3323.6  3324.1  3318.9  3319.6  3320.0  3321.9  3323.9  3324.1  3319.5  3320.0  3321.9  3324.2  3324.3  3314.9
  3319.5  3320.4  3320.8    -1.0    -1.0    -1.0  3319.0  3319.7  3320.0    -1.0    -1.0    -1.0  3319.5  3319.8  3321.8    -1.0    -1.0    -1.0
  3319.4  3320.2  3321.0  3323.4  3323.7  3324.2  3318.9  3319.7  3320.1  3321.7  3323.9  3324.1  3319.5  3319.8  3321.9  3324.0  3324.4  3314.8
  3319.6  3320.2  3320.7    -1.0    -1.0    -1.0  3319.0  3319.7  3320.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0    -1.0
  3319.5  3320.2  3321.0  3322.7  3323.6  3324.2  3318.8  3319.7  3320.2  3321.0  3323.9  3324.0  3319.4  3320.0  3322.0  3323.4  3324.4  3314.9
  3319.5  3320.3  3320.9  3324.0  3323.5  3324.1  3318.7  3319.7  3320.0  3322.1  3323.8  3324.1  3319.5  3319.9  3321.8  3324.4  3324.4  3314.9
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

double cell1[18]; // New
double cell2[18]; // Previous
double cellLG[18]; // Last good reading

uint32_t consecutive[18];
uint32_t worst[18];
uint32_t total[18];

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
	uint32_t skipbytes;
	uint32_t skiplines;
	uint8_t err_short = 0;
	uint8_t oto_sw = 0;
	uint8_t flag = 0;
	double dhighest = 0;
	double dlowest = 65535;
	int linehighest;
	int linelowest;
	int cellhighest;
	int celllowest;

	if (argc < 3)
	{
		printf("Expect two arguments: got %d\n Arg list: skiplines skip-leading-chars\n",argc);
		return -1;
	}
	sscanf(*(argv+1),"%i",&skiplines); // Skip lines at beginning of file
	sscanf(*(argv+2),"%i",&skipbytes); // Skip bytes at beginning of line

//	printf("\nskipbytes: %d limit: %f\n",skipbytes,dlow);
//return 0;	

	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		ctr += 1;
		/* Skip leading lines. */
		if (ctr <= skiplines)
			continue;
		i = strlen(buf);

		/* Skip short lines */
		if (i < 131)
		{
			printf("%d short line %d: %s",ctr,i,buf);
			err_short += 1;
			continue;
		}

		/* Scan for readings beginning after skipbytes count. */
		sscanf(&buf[skipbytes],"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ",
			&cell1[0],&cell1[ 1],&cell1[ 2],&cell1[ 3],&cell1[ 4],&cell1[ 5],&cell1[ 6],&cell1[ 7],&cell1[ 8],
			&cell1[9],&cell1[10],&cell1[11],&cell1[12],&cell1[13],&cell1[14],&cell1[15],&cell1[16],&cell1[17]);

		/* Get a good reading to start so that there is a good previous reading next. */
		if (oto_sw == 0)
		{
			oto_sw = 1;
			for (i = 0; i < NCELL; i++)
			{
				cell2[i]  = cell1[i]; // Save first line for previous line
				cellLG[i] = cell2[i];
				// If first line has missing cell reading get another line
				if (cell1[i] < 0)
				{
					oto_sw = 0;
				}
			}
			if (oto_sw != 0)
			{ // Here, all 18 cells had valid readings. Output line
				for (i = 0; i < NCELL; i++)
					printf(" %7.1f",cell2[i]);
				printf("\n");
			}
			continue;
		}

		/* Here, the reading no longer startup */
		for (i = 0; i < NCELL; i++)
		{
//			printf("%7.1f",cell1[i]);

		}
//		printf("\n");
		flag = 0;
		for (i = 0; i < NCELL; i++)
		{
			if (cell1[i] < 0)
			{ // Here input line cell missing a reading
				flag = 1;
				cell1[i] = -cellLG[i]; // Substiture last good reading
				consecutive[i] += 1;  // Count number of consecutive cases.
				total[i] += 1;
			}
			else
			{ // 
				cellLG[i] = cell1[i];
				if (consecutive[i] > worst[i])
				{
					worst[i] = consecutive[i];
				}
				consecutive[i] = 0;
			}
		}
		for (i = 0; i < NCELL; i++)
		printf(" %7.1f",cell1[i]);
		printf("\n");
	}
	printf("Number input  lines %6d\n",ctr);
	printf("Short lines: %d\n",err_short);
	printf("           :");
	for (i = 0; i < NCELL; i++) printf(" %4d",i+1);	
	printf("\nConsecutive:");
	for (i = 0; i < NCELL; i++) printf(" %4d",consecutive[i]);
	printf("\nWorst      :");
	for (i = 0; i < NCELL; i++) printf(" %4d",worst[i]);
	printf("\n");
		return 0;
}

