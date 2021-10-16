/******************************************************************************
* File Name          : SerialTaskSend.h
* Date First Issued  : 12/04/2018
* Description        : FreeRTOS ST HAL serial gateway/buffering
*******************************************************************************/

#ifndef __SERIALTASKSEND
#define __SERIALTASKSEND

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

#define MAXNUMSERIAL 6	// Number of uart + usart.


/* Serial Send Task Buffer Control Block

SERIALSENDTASKBCB is added to the queue which holds the information
for uart/usarts sending.

First three items eventually get used by calls to--
  HAL_UART_Transmit_IT(p->phuart,p->pbuf,p->size)
or depending on 'utype',
  HAL_USART_Transmit_IT(p->phusart,p->pbuf,p->size)

'taskhandle' and 'notebufbit' are used to notify the
originating task that the buffer has been sent and is
now free.  The buffer is identified by bit position,
which is arbitrary and up the originating task to
assign, and of course limited to 32.

*/

struct SERIALSENDTASKBCB
{
	UART_HandleTypeDef* phuart; // Pointer to 'MX uart handle
	osThreadId tskhandle;       // Task handle of originating task
	SemaphoreHandle_t semaphore;// Semaphore handle
	uint8_t	*pbuf;             // Pointer to byte buffer to be sent
	uint16_t size;              // Number of bytes to be sent
	uint16_t maxsize;           // Buffer size 	
};

/* *************************************************************************/
BaseType_t xSerialTaskSendAdd(UART_HandleTypeDef* p, uint16_t qsize, int8_t dmaflag);
/*	@brief	: Add a uart and circular buffer to a linked list
 * @param	: p = pointer to uart control block
 * @param	: qsize = total number of buffer control blocks circular buffer can hold
 * @param	: dmaflag = 0 = char-by-char, 1 = dma
 * @return	: 0 = OK, -1 = failed 1st calloc, -2 = failed 2nd calloc
 * *************************************************************************/
osThreadId xSerialTaskSendCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: SerialTaskSendHandle
 * *************************************************************************/
 void vStartSerialTaskSend(void);
/*	@brief	: Task startup
 * *************************************************************************/
void vSerialTaskSendQueueBuf(struct SERIALSENDTASKBCB** ppbcb);
/*	@brief	: Load buffer control block onto queue for sending
 * @param	: ppbcb = Pointer to pointer to Buffer Control Block
 * *************************************************************************/

extern osMessageQId SerialTaskSendQHandle;

//extern osThreadId SerialTaskHandle;
extern TaskHandle_t SerialTaskHandle;

/* Macros for simplifying the wait and loading of the queue */
#define mSerialTaskSendWait( noteval, bit){while((noteval & bit) == 0){xTaskNotifyWait(bit, 0, &noteval, 5000);}}
#define mSerialTaskSendQueueBuf(bcb){uint32_t qret;do{qret=xQueueSendToBack(SerialTaskSendQHandle,bcb,portMAX_DELAY);}while(qret == errQUEUE_FULL);}

#define mSerialTaskSendQueueBuf2(bcb,bit){uint32_t qret;do{noteval&=~bit;qret=xQueueSendToBack(SerialTaskSendQHandle,bcb,portMAX_DELAY);}while(qret == errQUEUE_FULL);}

#endif

