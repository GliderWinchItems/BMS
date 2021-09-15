/******************************************************************************
* File Name          : yprintf.c
* Date First Issued  : 01/17/2019
* Board              : 
* Description        : Substitute for 'fprintf' for multiple uarts
*******************************************************************************/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "yprintf.h"

//osSemaphoreId vsnprintfSemaphoreHandle;
SemaphoreHandle_t vsnprintfSemaphoreHandle;

static uint8_t sw = 0;	// OTO initialization switch

/* **************************************************************************************
 *  int yprintf_init(void);
 * @brief	: Setup semaphore
 * @return	: 0 = init executed; -1 = init already done
 * ************************************************************************************** */
int yprintf_init(void)
{
	if (sw == 0)
	{
		sw = 1;
//		osSemaphoreDef(vsnprintfSemaphore);
//		vsnprintfSemaphoreHandle = osSemaphoreCreate(osSemaphore(vsnprintfSemaphore), 1);
		vsnprintfSemaphoreHandle = xSemaphoreCreateBinary();
		xSemaphoreGive( vsnprintfSemaphoreHandle );
	}
	return sw;
}
/* **************************************************************************************
 * int yprintf(struct SERIALSENDTASKCB** ppbcb, const char *fmt, ...);
 * @brief	: 'printf' for uarts
 * @param	: pbcb = pointer to pointer to stuct with uart pointers and buffer parameters
 * @param	: format = usual printf format
 * @param	: ... = usual printf arguments
 * @return	: Number of chars "printed"
 * ************************************************************************************** */

uint32_t yctr = 0;

int yprintf(struct SERIALSENDTASKBCB** ppbcb, const char *fmt, ...)
{
	struct SERIALSENDTASKBCB* pbcb = *ppbcb;
	va_list argp;

	yprintf_init();	// JIC not init'd

	/* Block if this buffer is not available. SerialSendTask will 'give' the semaphore 
      when the buffer has been sent. */
	xSemaphoreTake(pbcb->semaphore, 6000);

	/* Block if vsnprintf is being uses by someone else. */
	xSemaphoreTake( vsnprintfSemaphoreHandle, portMAX_DELAY );
//xSemaphoreTake( vsnprintfSemaphoreHandle, 8000 );
	/* Construct line of data.  Stop filling buffer if it is full. */
//	va_start(argp, fmt);
	va_start(argp, fmt);
	pbcb->size = vsnprintf((char*)(pbcb->pbuf),pbcb->maxsize, fmt, argp);
	va_end(argp);
yctr += 1;

	/* Limit byte count in BCB to be put on queue, from vsnprintf to max buffer sizes. */
	if (pbcb->size > pbcb->maxsize) 
			pbcb->size = pbcb->maxsize;

	/* Release semaphore controlling vsnprintf. */
	xSemaphoreGive( vsnprintfSemaphoreHandle );

	/* JIC */
	if (pbcb->size == 0) return 0;

	/* Place Buffer Control Block on queue to SerialTaskSend */
	vSerialTaskSendQueueBuf(ppbcb); // Place on queue

	return pbcb->size;
}
/* **************************************************************************************
 * int yputs(struct SERIALSENDTASKBCB** ppbcb, char* pchr);
 * @brief	: Send zero terminated string to SerialTaskSend
 * @param	: pbcb = pointer to pointer to stuct with uart pointers and buffer parameters
 * @return	: Number of chars sent
 * ************************************************************************************** */
int yputs(struct SERIALSENDTASKBCB** ppbcb, char* pchr)
{
	struct SERIALSENDTASKBCB* pbcb = *ppbcb;
	int sz = strlen(pchr); // Check length of input string
	if (sz == 0) return 0;

	/* Block if this buffer is not available. SerialSendTask will 'give' the semaphore 
      when the buffer has been sent. */
	xSemaphoreTake(pbcb->semaphore, 6000);

	strncpy((char*)pbcb->pbuf,pchr,pbcb->maxsize);	// Copy and limit size.

	/* Set size serial send will use. */
	if (sz >= pbcb->maxsize)	// Did strcpy truncate?
		pbcb->size = pbcb->maxsize;	// Yes
	else
		pbcb->size = sz;	// No

	vSerialTaskSendQueueBuf(ppbcb); // Place on queue
	return pbcb->size; 
}

