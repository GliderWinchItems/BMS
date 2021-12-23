/******************************************************************************
* File Name          : morseprintf.c
* Date First Issued  : 11/13/2019
* Board              : 
* Description        : printf for sending Morse on the beeper
*******************************************************************************/
/* 
This uses the buffer facility from 'SerialTaskSend' and works in conjunction 
with 'morseprintf' which is 'yprintf' with minor modifications.
*/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "yprintf.h"
#include "morseprintf.h"

extner osSemaphoreId vsnprintfSemaphoreHandle;

/* **************************************************************************************
 * int morseprintf_init(void);
 * @brief	: Setup semaphore
 * @return	: 0 = init executed; -1 = init already done
 * ************************************************************************************** */
int morseprintf_init(void)
{
	if (vsnprintfSemaphoreHandle == NULL)
	{
		osSemaphoreDef(vsnprintfSemaphore);
		vsnprintfSemaphoreHandle = osSemaphoreCreate(osSemaphore(vsnprintfSemaphore), 1);
	}
	return sw;
}
/* **************************************************************************************
 * int morseprintf(struct SERIALSENDTASKCB** ppbcb, const char *fmt, ...);
 * @brief	: 'printf' for uarts
 * @param	: pbcb = pointer to pointer to stuct with uart pointers and buffer parameters
 * @param	: format = usual printf format
 * @param	: ... = usual printf arguments
 * @return	: Number of chars "printed"
 * ************************************************************************************** */
int morseprintf(struct SERIALSENDTASKBCB** ppbcb, const char *fmt, ...)
{
	struct SERIALSENDTASKBCB* pbcb = *ppbcb;
	va_list argp;

	morseprintf_init();	// JIC not init'd

	/* Block if this buffer is not available. SerialSendTask will 'give' the semaphore 
      when the buffer has been sent. */
	xSemaphoreTake(pbcb->semaphore, 6000);

	/* Block if vsnprintf is being uses by someone else. */
	xSemaphoreTake( vsnprintfSemaphoreHandle, portMAX_DELAY );

	/* Construct line of data.  Stop filling buffer if it is full. */
	va_start(argp, fmt);
	va_start(argp, fmt);
	pbcb->size = vsnprintf((char*)(pbcb->pbuf),pbcb->maxsize, fmt, argp);
	va_end(argp);

	/* Limit byte count in BCB to be put on queue, from vsnprintf to max buffer sizes. */
	if (pbcb->size > pbcb->maxsize) 
			pbcb->size = pbcb->maxsize;

	/* Release semaphore controlling vsnprintf. */
	xSemaphoreGive( vsnprintfSemaphoreHandle );

	/* JIC */
	if (pbcb->size == 0) return 0;

	/* Place Buffer Control Block on queue to SerialTaskSend */
	vMorseBeepTaskQueueBuf(ppbcb); // Place on queue

	return pbcb->size;
}
/* **************************************************************************************
 * int morseputs(struct SERIALSENDTASKBCB** ppbcb, char* pchr);
 * @brief	: Send zero terminated string to SerialTaskSend
 * @param	: pbcb = pointer to pointer to stuct with uart pointers and buffer parameters
 * @return	: Number of chars sent
 * ************************************************************************************** */
int morseputs(struct SERIALSENDTASKBCB** ppbcb, char* pchr)
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

	BeepTaskQHandle(ppbcb); // Place on MorseBeep queue
	return pbcb->size; 
}
/* *************************************************************************
 * void vMorseBeepTaskQueueBuf(struct SERIALSENDTASKBCB** ppbcb);
 *	@brief	: Load buffer control block onto queue for sending
 * @param	: ppbcb = Pointer to pointer to Buffer Control Block
 * *************************************************************************/
void vMorseBeepTaskQueueBuf(struct SERIALSENDTASKBCB** ppbcb)
{
	uint32_t qret;
	do 
	{
		qret=xQueueSendToBack(MorseBeepTaskQHandle, ppbcb, portMAX_DELAY);
		if (qret == errQUEUE_FULL) osDelay(1); // Delay, don't spin.

	} while(qret == errQUEUE_FULL);
	return;
}

