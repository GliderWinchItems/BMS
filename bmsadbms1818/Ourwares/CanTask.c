/******************************************************************************
* File Name          : CanTask.c
* Date First Issued  : 01/01/2019
* Description        : CAN interface to FreeRTOS/ST HAL
*******************************************************************************/

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_can.h"
#include "CanTask.h"
#include "can_iface.h"
#include "morse.h"

#include "main.h"

extern CAN_HandleTypeDef hcan1;

//osThreadId CanTxTaskHandle;
TaskHandle_t CanTxTaskHandle;
QueueHandle_t CanTxQHandle;

uint8_t CANTaskreadyflag = 0;

/* ====== Tx ==============================================================*/
/* *************************************************************************
 * void canmsg_expand(CAN_TxHeaderTypeDef *phal, uint8_t *pdat, struct CANRCVBUF *pcan);
 * @brief	: Convert hardware compressed format to silly HAL expanded format
 * @param	: phal = pointer to HAL output
 * @param	: pdat = pointer to HAL payload data array
 * @param	: pcan = pointer to convenient hardware format
 * *************************************************************************/
void canmsg_expand(CAN_TxHeaderTypeDef *phal, uint8_t *pdat, struct CANRCVBUF *pcan)
{
	phal->StdId = (pcan->id >> 21);
	phal->ExtId = (pcan->id >>  3);
	phal->IDE   = (pcan->id & CAN_ID_EXT);
	phal->RTR   = (pcan->id & CAN_RTR_REMOTE);
	phal->DLC   = (pcan->dlc & 0x7);
	*(pdat+0) = pcan->cd.uc[0];
	*(pdat+1) = pcan->cd.uc[1];
	*(pdat+2) = pcan->cd.uc[2];
	*(pdat+3) = pcan->cd.uc[3];
	*(pdat+4) = pcan->cd.uc[4];
	*(pdat+5) = pcan->cd.uc[5];
	*(pdat+6) = pcan->cd.uc[6];
	*(pdat+7) = pcan->cd.uc[7];
	return;
}


/* *************************************************************************
 * void StartCanTxTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
uint32_t dbgCanTask1;
void StartCanTxTask(void* argument)
{
   BaseType_t Qret;	// queue receive return
	struct CANTXQMSG txq;
	int ret;

	extern CAN_HandleTypeDef hcan1;
	ret = HAL_CAN_Start(&hcan1); // Start CAN1
	if (ret != HAL_OK)
	{
		morse_trap(738);
	}

	CANTaskreadyflag = 1;

  /* Infinite RTOS Task loop */
  for(;;)
  {
//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14); // 14-RED, 13-ORANGE
		Qret = xQueueReceive(CanTxQHandle,&txq,portMAX_DELAY);
		if (Qret == pdPASS) // Break loop if not empty
		{
dbgCanTask1 += 1;			
			ret = can_driver_put(txq.pctl, &txq.can, txq.maxretryct, txq.bits);
/* ===> Trap errors
 *				: -1 = Buffer overrun (no free slots for the new msg)
 *				: -2 = Bogus CAN id rejected
 *				: -3 = control block pointer NULL */
			if (ret == -1) morse_trap(91);
			if (ret == -2) morse_trap(92);
			if (ret == -3) morse_trap(93);
		}
  }
}
/* *************************************************************************
 * QueueHandle_t  xCanTxTaskCreate(uint32_t taskpriority, int32_t queuesize);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @param	: queuesize = number of items in Tx queue
 * @return	: QueueHandle_t = queue handle
 * *************************************************************************/
QueueHandle_t  xCanTxTaskCreate(uint32_t taskpriority, int32_t queuesize)
{
 /* definition and creation of CanTask */
//  osThreadDef(CanTxTask, StartCanTxTask, osPriorityNormal, 0, 96);
//  CanTxTaskHandle = osThreadCreate(osThread(CanTxTask), NULL);
//	vTaskPrioritySet( CanTxTaskHandle, taskpriority );

	BaseType_t ret = xTaskCreate(StartCanTxTask, "CanTxTask",\
     (96), NULL, taskpriority,\
     &CanTxTaskHandle);
	if (ret != pdPASS) return NULL;


	/* FreeRTOS queue for task with data to send. */
	CanTxQHandle = xQueueCreate(queuesize, sizeof(struct CANTXQMSG));
	return CanTxQHandle;
}

/* ====== Rx ==============================================================*/
//osThreadId CanRxTaskHandle;
TaskHandle_t CanRxTaskHandle;
QueueHandle_t CanRxQHandle;


/* *************************************************************************
 * void StartCanRxTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartCanRxTask(void* argument)
{
/* NOTE:  Since there is just one receiving task, this task is not needed.
          However, the initialization to create the queue is needed.
*/
	vTaskSuspend( NULL );

//   BaseType_t Qret;	// queue receive return
//	struct CANRCVBUFN ncan;

  /* Infinite RTOS Task loop */
  for(;;)
  {
osDelay(11000);
//$ TODO: distribute CAN msg
//$		Qret = xQueueReceive(CanRxQHandle,&ncan,portMAX_DELAY);
  }
  return;
}
/* *************************************************************************
 * QueueHandle_t xCanRxTaskCreate(uint32_t taskpriority, int32_t queuesize);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @param	: queuesize = number of RX0 + RX1 msgs
 * @return	: CanRxQHandle
 * *************************************************************************/
QueueHandle_t xCanRxTaskCreate(uint32_t taskpriority, int32_t queuesize)
{
 /* definition and creation of CanTask */
//  osThreadDef(CanRxTask, StartCanRxTask, osPriorityNormal, 0, 96);
//  CanRxTaskHandle = osThreadCreate(osThread(CanRxTask), NULL);
//	vTaskPrioritySet( CanRxTaskHandle, taskpriority );


	BaseType_t ret = xTaskCreate(StartCanRxTask, "CanRxTask",\
     (128), NULL, taskpriority,\
     &CanRxTaskHandle);
	if (ret != pdPASS) return NULL;

	/* FreeRTOS queue for task with data to send. */
	CanRxQHandle = xQueueCreate(queuesize, sizeof(struct CANRCVBUFN));

	return CanRxQHandle;
}
