/******************************************************************************
* File Name          : yprintf.h
* Date First Issued  : 01/17/2019
* Board              : 
* Description        : Substitute for 'fprintf' for multiple uarts
*******************************************************************************/

#ifndef __YPRINTF
#define __YPRINTF

#include "SerialTaskSend.h"

/* **************************************************************************************/
 int yprintf_init(void);
/* @brief	: Setup semaphore
 * @return	: 0 = init executed; -1 = init already done
 * **************************************************************************************/
int yprintf(struct SERIALSENDTASKBCB** ppbcb, const char *fmt, ...);
/* @brief	: 'printf' for uarts
 * @param	: pbcb = pointer to pointer to stuct with uart pointers and buffer parameters
 * @param	: format = usual printf format
 * @param	: ... = usual printf arguments
 * @return	: Number of chars "printed"
 * ************************************************************************************** */
int yputs(struct SERIALSENDTASKBCB** ppbcb, char* pchr);
/* @brief	: Send zero terminated string to SerialTaskSend
 * @param	: pbcb = pointer to pointer to stuct with uart pointers and buffer parameters
 * @return	: Number of chars sent
 * ************************************************************************************** */

//extern osSemaphoreId vsnprintfSemaphoreHandle;
extern SemaphoreHandle_t vsnprintfSemaphoreHandle;

#endif 

