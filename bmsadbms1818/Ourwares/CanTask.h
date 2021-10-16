/******************************************************************************
* File Name          : CanTask.h
* Date First Issued  : 01/01/2019
* Description        : CAN interface to FreeRTOS/ST HAL
*******************************************************************************/

#ifndef __CANTASK
#define __CANTASK

#include "stm32l4xx_hal_def.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "common_can.h"

/* CAN msg passed on queue from a Task sending a CAN msg */
struct CANTXQMSG
{
	struct CANRCVBUF can;		// CAN msg
	struct CAN_CTLBLOCK* pctl;	// Pointer to control block for this CAN
	uint8_t maxretryct;
	uint8_t bits;
};


/* *************************************************************************/
QueueHandle_t  xCanTxTaskCreate(uint32_t taskpriority, int32_t queuesize);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @param	: queuesize = number of items in Tx queue
 * @return	: QueueHandle_t = queue handle
 * *************************************************************************/
QueueHandle_t xCanRxTaskCreate(uint32_t taskpriority, int32_t queuesize);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @param	: queuesize = number of RX0 + RX1 msgs
 * @return	: CanRxQHandle
 * *************************************************************************/

extern QueueHandle_t CanTxQHandle;
extern QueueHandle_t CanRxQHandle;

#endif

