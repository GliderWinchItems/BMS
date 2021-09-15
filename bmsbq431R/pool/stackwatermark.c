/******************************************************************************
* File Name          : stackwatermark.c
* Date First Issued  : 01/22/2019
* Description        : Output stack watermark
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "SerialTaskSend.h"

#include "stackwatermark.h"
#include "yprintf.h"

extern osSemaphoreId sprintfSemaphoreHandle;

/* *************************************************************************
 * void stackwatermark_show(osThreadId TaskHandle, struct SERIALSENDTASKBCB** ppbcb, char* pchar);
 *	@brief	:
 * @param	: TaskHandle = task handle for stackwatermark check  
 * @param	: pycb = pointer to stuct to pointer with uart pointers and buffer parameters
 * @param	: pchar = pointer to text string to precede stack value number
 * *************************************************************************/
void stackwatermark_show(osThreadId TaskHandle, struct SERIALSENDTASKBCB** ppbcb, char* pchar)
{
	UBaseType_t stackwatermark; // For unused task stack space

		/* Get stack high water mark for tasks of interest. */
		stackwatermark = uxTaskGetStackHighWaterMark( TaskHandle );

		yprintf(ppbcb, "\n\rStack: %s %3d",pchar, (int)stackwatermark);
		
	return;
}
