/******************************************************************************
* File Name          : stackwatermark.h
* Date First Issued  : 01/22/2019
* Description        : Output stack useage for a task
*******************************************************************************/

#ifndef __STACKWATERMARK
#define __STACKWATERMARK

#include <stdint.h>
#include "getserialbuf.h"

/* *************************************************************************/
void stackwatermark_show(osThreadId TaskHandle, struct SERIALSENDTASKBCB** ppbcb, char* pchar);
/*	@brief	:
 * @param	: TaskHandle = task handle for stackwatermark check  
 * @param	: pycb = pointer to stuct to pointer with uart pointers and buffer parameters
 * @param	: pchar = pointer to text string to precede stack value number
 * *************************************************************************/

#endif
