/******************************************************************************
* File Name          : yscanf.h
* Date First Issued  : 01/22/2019
* Board              : 
* Description        : For FreeRTOS sscanf'ing
*******************************************************************************/

#ifndef __YSCANF
#define __YSCANF

#include "getserialbuf.h"

/* **************************************************************************************/
 int yscanf_init(void);
/* @brief	: Setup semaphore
 * @return	: 0 = init executed; -1 = init already done
 * **************************************************************************************/
int yscanf(char* pline, const char *fmt, ...);
/* @brief	: 'sscanf' for uart input
 * @param	: pline = pointer to stuct with uart pointers and buffer parameters
 * @param	: format = usual scanf format
 * @param	: ... = usual scanf arguments
 * @return	: Number of items matched
 * ************************************************************************************** */

extern osSemaphoreId sscanfSemaphoreHandle;

#endif 

