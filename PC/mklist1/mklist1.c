/******************************************************************************
* File Name          : mklist1.c
* Date First Issued  : 09/15/2023
* Board              : Linux PC
* Description        : Load all BMS: first step
*******************************************************************************/
/*
./ccall
This will pipe ls *bq*.c into this routine, which is a list of
<can id>-bq_idx_v_struct.c
files.

This extracts the CAN ID and makes a file which is used by
mklist2._

gcc mklist1.c -o mklist1
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

FILE* fpOut;
char *paramlist = "../../bmsadbms1818/params/paramIDlist";

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;
	uint32_t ctr = 0;

	if ( (fpOut = fopen (paramlist,"w")) == NULL)
	{
		printf ("\nOutput file did not open: %s\n",paramlist); 
		exit (-1);
	}

	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		ctr += 1;
		i = strlen(buf);
		if (i < 14)
			continue;

		/* Extract CAN ID from line */
		buf[8] = '\n';
		buf[9] = 0;
		printf("%s",buf);
		fprintf(fpOut,buf,strlen(buf));
	}
	
		return 0;
}

