/******************************************************************************
* File Name          : filterminicom.c
* Date First Issued  : 07/18/23
* Board              : Linux PC
* Description        : Select (filter) lines from input file for output based on arguments
*******************************************************************************/
/*
Compile and execute example with two arguements to select lines from input--
gcc filterminicom.c -o filterminicom -lm && ./filterminicom ADCVAX Jcellv < G*ems/BMS/minicom-230714-155954.txt

                 1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18
    4ADCVAX  39310  39317  39334  39325  39331  39327  39311  39318  39319  39338  39326  39345  39315  39312  39328  39327  39342  39331 3975
                 1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18
Jcellv[i]:   39310  39317  39333  39324  39330  39328  39312  39319  39318  39338  39327  39345  39315  39313  39330  39327  39343  39330 707859

Procedure--
- Save minicom serial monitoring of BMS board with CtlA Z L and give file name
- Run filterminicom using the path/name for file.
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
	uint32_t ctrout = 0;
	int m;
	int j;

	if (argc < 2)
	{
		printf("\nNeed at least one argument: I see %d\n",argc-1);
		printf("./filterminicom <selection list> < <input path/file\n");
		printf("Example:\n./filterminicom ADCVAX Jcellv < G*ems/BMS/minicom-230714-155954.txt\n");
		return -1;
	}
#if 0
	printf("Lines with the following will selected for output\n");
	for (i = 1; i < argc; i++)
	{
		printf("%d %s\n",i,*(argv+i));
	}
#endif
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		ctr += 1;
		i = strlen(buf);
		int flag = 0;
		for (i = 1; i < argc; i++)
		{
			if (strstr(buf, *(argv+i)))
			{
				flag = i;
			}
		}
		if (flag != 0)
		{
			ctrout += 1;
			if ((strcmp(*(argv+flag),"ADCVAX") == 0) ||
				(strcmp(*(argv+flag),"Jcellv") == 0) )
			{
				m = 138;
	 			buf[m] = '\n';
	 			buf[m+1] = 0;
	 			j = 11;
 			}
 			else
 				j = 0;
			printf("%2d %s",flag, &buf[j]);
		}
	}
#if 1	
	printf("Number input  lines %6d\n",ctr);
	printf("Number output lines %6d\n",ctrout);
		for (i = 1; i < argc; i++)
	{
		printf("%d %s\n",i,*(argv+i));
	}
#endif	
		return 0;
}

