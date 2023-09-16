/******************************************************************************
* File Name          : mklist2.c
* Date First Issued  : 09/15/2023
* Board              : Linux PC
* Description        : Load all BMS: Discover BMS nodes on bus
*******************************************************************************/
/*
./ccall
This will pipe ls *bq*.c into this routine.

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

FILE* fpIn;
char *paramfile = "../../bmsadbms1818/params/paramIDlist";
FILE* fpOut;
char *call_file = "../../bmsadbms1818/params/call_file";

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

uint32_t paramlist[32];
uint8_t idpresent[32];

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i = 0;
	int j;
	int len;
	uint32_t ctr = 0;
	uint32_t id;
	int z[4];
	char* pchr;

/* Read in list of CAN IDs that have a parameter file. */
printf("BEGIN mklist2\n");
	if ( (fpIn = fopen (paramfile,"r")) == NULL)
	{
		printf ("\nInput file did not open: %s\n",paramfile); 
		exit (-1);
	}	

	while ( (fgets (&buf[0],LINESIZE,fpIn)) != NULL)	// Get a line from stdin
	{
		sscanf(buf,"%8X",&id);
		paramlist[i] = id;
		i += 1;
	}
printf("CT: %d\n",i);
	for (j = 0; j < i; j++)
		printf("%08X\n",paramlist[j]);
printf("END CAN IDs in parameter file\n");

/* Select CAN for msgs that have a parameter file. */
#define SECSTOWAIT 5 // Duration to monitor
	time_t starttime = time(&starttime) + SECSTOWAIT;
  	time_t tim;
  	time_t ttmp;
  	time_t tic = time(&tic) + 1;
	while ((int)(time(&tim) - starttime) < 0)	
	{
		fgets(&buf[0],LINESIZE,stdin); // netcat feeds this with CAN msgs
		// Convert ascii CAN ID to uint32_t
		sscanf(&buf[2],"%2x%2x%2x%2X",&z[0],&z[1],&z[2],&z[3]);
		id = (z[3]<<24)|(z[2]<<16)|(z[1]<<8)|(z[0]<<0);

		for (j = 0; j < i; j++)
		{ // Check if this CAN is in the list
			if(paramlist[j] == id)
			{
				idpresent[j] += 1;
				break;
			}
		}
		if ((int)(time(&ttmp) - tic) >= 0)
		{
			tic = ttmp + 1;
			printf("tick\n");
		}
//		printf("%08X: ",id);
//		len = strlen(buf);
//		printf("%5d %s",len,buf);
	}
	fclose(fpIn);

	/* Generate a file that will reload nodes. */
	if ( (fpOut = fopen (call_file,"w")) == NULL)
	{
		printf ("\nOutput file did not open: %s\n",call_file); 
		exit (-1);
	}	
	int cty = 0;
	for (j = 0; j < i; j++)
	{ 
		if (idpresent[j] > 0)
		{
			cty +=1;
			printf("%2d %08X %2d\n",j,paramlist[j],idpresent[j]);
			fprintf(fpOut,"./cc %08X\n",paramlist[j]);
		}
	}	
	fclose(fpOut);
	printf("END build .cc list w count: %d\n",cty);
	return 0;
}

