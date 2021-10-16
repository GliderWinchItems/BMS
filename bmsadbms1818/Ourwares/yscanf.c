/******************************************************************************
* File Name          : yscanf.c
* Date First Issued  : 01/22/2019
* Board              : 
* Description        : For FreeRTOS sscanf'ing
*******************************************************************************/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "yscanf.h"

osSemaphoreId sscanfSemaphoreHandle;
static uint8_t sw = 0;	// OTO initiationzation switch

/* **************************************************************************************
 *  int yscanf_init(void);
 * @brief	: Setup semaphore
 * @return	: 0 = init executed; -1 = init already done
 * ************************************************************************************** */
int yscanf_init(void)
{
	if (sw == 0)
	{
		sw = -1;
		osSemaphoreDef(sscanfSemaphore);
		sscanfSemaphoreHandle = osSemaphoreCreate(osSemaphore(sscanfSemaphore), 1);
	}
	return sw;
}
/* **************************************************************************************
 *  int yscanf(char* pline, const char *fmt, ...);
 * @brief	: 'sscanf' for uart input
 * @param	: pline = pointer to stuct with uart pointers and buffer parameters
 * @param	: format = usual scanf format
 * @param	: ... = usual scanf arguments
 * @return	: Number of items matched
 * ************************************************************************************** */
int yscanf(char* pline, const char *fmt, ...)
{
	va_list argp;
	int itemct;		// Number of items matched

	yscanf_init();	// JIC not init'd

	/* Block if sscanf is being uses by someone else. */
	xSemaphoreTake( sscanfSemaphoreHandle, portMAX_DELAY );

	/* Construct line of data.  Stop filling buffer if it is full. */
	va_start(argp, fmt);
	va_start(argp, fmt);
	itemct = vsscanf(pline, fmt, argp);
	va_end(argp);

	/* Release semaphore controlling sscanf. */
	xSemaphoreGive( sscanfSemaphoreHandle );

	return itemct;
}

