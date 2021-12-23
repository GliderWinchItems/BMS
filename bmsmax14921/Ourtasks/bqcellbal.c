/******************************************************************************
* File Name          : bqcellbal.c
* Date First Issued  : 09/23/2021
* Description        : BQ76952: cell balancing
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"

#include "BQTask.h"
#include "bqview.h"
#include "yprintf.h"
#include "BQ769x2Header.h"
#include "bqcellbal.h"

int32_t cellave;        // Average of readings
int16_t celldev[16];	// deviation around average
int16_t celldevabs[16]; // devation absolute
int16_t cellmax;        // Max (signed) deviation
int16_t cellmin;        // Min (signed) deviation
int16_t cellabs;		// Max absolute deviation
uint8_t cellmaxidx;		// Index for cellmax
uint8_t cellminidx;     // Index for cellmin
uint8_t cellabsidx;     // Index for cell absolute


/* *************************************************************************
 * void bqcellbal_data (void);
 *	@brief	: display parameters
 * *************************************************************************/
extern int16_t cellv[2][BQVSIZE];
extern uint8_t cvidx;
void bqcellbal_data (void)
{
	int16_t* pv = &cellv[cvidx][0];
	int16_t* p = pv;
	int32_t sum = 0;
	int i;

	/* Sum reported voltages. */
	for (i = 0; i < 16; i++) sum += *p++;
	sum = sum / 16;
	cellave = sum;
	
	/* Compute deviation above/below average */
	p = pv;
	for (i = 0; i < 16; i++)
	{
		celldev[i] = (*p++ - sum);
		if (celldev[i] < 0)
			celldevabs[i] = -celldev[i];
		else
			celldevabs[i] =  celldev[i];
	}

	/* Find max & min. */
	p = &celldev[0];
	cellmax = -7000; cellmin = 7000; // Largest deviations possible
	cellabs = -1;
	for (i = 0; i < 16; i++)
	{
		 if (*p > cellmax)
		 {
		 	cellmax = *p;
		 	cellmaxidx = i;
		 }
		 if (*p < cellmin)
		 {
		 	cellmin = *p;
		 	cellminidx = i;
		 }
		 if (celldevabs[i] > cellabs )
		 {
		 	cellabs = celldevabs[i];
		 	cellabsidx = i;
		 }
		 p += 1;
	}
}