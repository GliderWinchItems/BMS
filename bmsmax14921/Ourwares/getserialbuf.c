/******************************************************************************
* File Name          : getserialbuf.c
* Date First Issued  : 01/12/2019
* Description        : Get a buffer  & control block for SerialTaskSend use
*******************************************************************************/

#include <malloc.h>
#include "getserialbuf.h"

/* Reproduced for convenience from 'SerialTaskSend.h'
struct SERIALSENDTASKBCB
{
	UART_HandleTypeDef* phuart; // Pointer to 'MX uart handle
	osThreadId tskhandle;       // Task handle of originating task
	SemaphoreHandle_t semaphore;// Semaphore hanlde
	uint8_t	*pbuf;             // Pointer to byte buffer to be sent
	uint16_t size;              // Number of bytes to be sent
	uint16_t maxsize;           // Buffer size 		
};
*/

/* *************************************************************************
 * struct SERIALSENDTASKBCB* getserialbuf( UART_HandleTypeDef* phuart, uint16_t maxsize);
 * @brief	: Create a buffer control block (BCB) for serial sending
 * @param	: phuart = usart handle (pointer)
 * @param	: size = number of uint8_t bytes for this buffer
 * @return	: pointer to BCB; NULL = failed
 * *************************************************************************/
/*
Construct a list of "struct NOTEBITLIST" items, for each different usart.

The items maintain the bit used by SerialTaskSend to notify the originating
task that the buffer has been sent and is available for reuse.
*/
struct SERIALSENDTASKBCB* getserialbuf( UART_HandleTypeDef* phuart, uint16_t maxsize)
{
/* This must be called AFTER a FreeRTOS task starts so that this routine can call
   FreeRTOS to get the task handle.

	To avoid problems of a time tick switching tasks in the middle of this routine the
   FreeRTOS interrupts are locked via "taskENTER_CRITICAL();".

*/
	/* BCB: Buffer control block, passed on queue to SerialTaskSend. See SerialTaskSend.h */
	struct SERIALSENDTASKBCB* pbcb; // calloc'ed bcb pointer

	uint8_t* pbuf;	// callloc'ed byte buffer

taskENTER_CRITICAL();
	/* Get one BCB block */
	pbcb = (struct SERIALSENDTASKBCB*)calloc(1, sizeof(struct SERIALSENDTASKBCB));	
	if (pbcb == NULL){taskEXIT_CRITICAL(); return NULL;}

	/* Get byte buffer */
	pbuf = (uint8_t*)calloc(maxsize, sizeof(uint8_t));	
	if (pbuf == NULL) return NULL;

	/* Initialize the BCB. */
	pbcb->phuart    = phuart;  // 'MX uart handle
	pbcb->tskhandle = xTaskGetCurrentTaskHandle();
	pbcb->pbuf      = pbuf;		// Ptr to uint8_t buffer
	pbcb->maxsize   = maxsize;    // Size of uint8_t buffer
	pbcb->semaphore = xSemaphoreCreateBinary(); // Semaphore for this buffer

	if (pbcb->semaphore == NULL){taskEXIT_CRITICAL(); return NULL;}
	xSemaphoreGive(pbcb->semaphore); // Initialize

taskEXIT_CRITICAL();

	return pbcb;
}

