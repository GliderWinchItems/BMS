/******************************************************************************
* File Name          : SerialTaskSend.c
* Date First Issued  : 12/09/2018
* Description        : Multiple task serial output using FreeRTOS/ST HAL
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "SerialTaskSend.h"

#include "stm32l4xx_hal_usart.h"
#include "stm32l4xx_hal_uart.h"

#include "main.h"
#include "morse.h"


/*
Goals: 
- Use the HAl driver unmodified
  .  Issue: the interrupt callback callback is common
     to all uarts and carries a uart control block
     pointer as an argument.

- (Maybe!) use same scheme with usb CDC.  Maybe same code?

- Handle 'n' uart/usarts 
  .  So far, three have been used, so allowing for six
     (stm32F407) may be overkill, but later stm32 versions
     may allow even more.

- FreeRTOS can output serial data from multiple tasks

- Buffering:
  .  Not depend on time before reusing a buffer, therefore
     interrupts need to drive it.
  .  Use a circular buffer that holds pointers
     and handles to the originating task & task buffer.
     .. copying char-by-char wasteful so use pointer control block
     .. Multiple buffers for multiple uarts
  .  Use source task's buffer and notify source task
     when the buffer has been sent, using
       'xTaskNotifyFromISR'

Scheme:

- Initialization:
  .  Create, or add, to a linked list with pointer to next
     block, uart/usart handle, and pointers to a circular
     buffer calloc'ed for the number of task buffers to be used.
     This done for each uart/usart implemented.

- Usage:
  .  A task generates the bytes to be sent in a buffer.
  .  A buffer control block is initialized with the 
     task handle, uart/usart handle, buffer pointer, and
     number of bytes to be sent (size).  (All but the size 
     can be initialized before the task endless loop begins.)
  .  The buffer control block (bcb) is sent to this routine's queue.
     This routine removes the bcb from the queue and adds it
     to the circular buffer for the uart/usart, then attempts
     to send it with the HAL routine.  The HAL routine rejects
     the attempt if it is already busy.
  .  Upon interrupt, the interrupt callback routine checks 
     the circular buffer for the uart/usart causing the completion
     of the sending interrupt.  
     .. It issues a "notification" to the task identified in the
        circular buffer block, so that the originating task can
        reuse the buffer.
     .. It "removes" (moves the pointer) the bcb from the circular
        buffer.  If more bcb's are in the circular buffer, it
        starts the HAL routine with the next bcb.
           
Note: If a uart/usart is not called from multiple tasks, i.e.
     just one task uses the usart/usart, and there is no need
     to overlap processing in that task with more than one
     output buffer, then this routine does not need to be used.

Multiple tasks can place "struct SERIALSENDTASKBCB" items (BCB) in a queue. 
The BCB holds:
 - Originating task handle
 - uart/usart control block pointer
 - buffer pointer
 - buffer size
 - uart|usart flag
 - bits for notifying originating task when the buffer has been sent, (so
     that the originating task can reuse the buffer).

A circular buffer of BCBs is created for each uart/usart.  The size of this
circular buffer must be at least as large as the number of buffers of
the tasks using this routine.



*/


/* Task */
#define SSPRIORITY 1	// Priority for this task (0 = Normal, -3 = Idle)

//static uint32_t SerialTaskSendBuffer[ 64 ];

//static osStaticThreadDef_t SerialTaskSendControlBlock;

//osThreadId SerialTaskHandle = NULL;
TaskHandle_t SerialTaskHandle = NULL;


/* Queue */
#define QUEUESIZE 16	// Total size of bcb's tasks can queue up

osMessageQId SerialTaskSendQHandle;
//static uint8_t SerialTaskSendQBuffer[ QUEUESIZE * sizeof( struct SERIALSENDTASKBCB ) ];
//static osStaticMessageQDef_t SerialTaskSendQCB;

/* Pattern
osMessageQId testQueue01Handle;
uint8_t myQueue01Buffer[ 16 * sizeof( uint16_t ) ];
osStaticMessageQDef_t myQueue01ControlBlock;
*/

/* Serial Send Cir Buf: Block with pointers into the circular buffer of pointers. */
struct SSCIRBUF
{
	struct SSCIRBUF* pnext;
	struct SERIALSENDTASKBCB** pbegin;
	struct SERIALSENDTASKBCB** pend;
	struct SERIALSENDTASKBCB** padd;
	struct SERIALSENDTASKBCB** ptake;
	UART_HandleTypeDef* phuart;
	int8_t	dmaflag;           // 0 = char-by-char; 1 = dma
};

/* Points to first of list of struct SSCIRBUF */
struct SSCIRBUF* pbhd = NULL;


/* *************************************************************************
 * BaseType_t xSerialTaskSendAdd(UART_HandleTypeDef* p, uint16_t qsize, int8_t dmaflag);
 *	@brief	: Add a uart and circular buffer to a linked list
 * @param	: p = pointer to uart control block
 * @param	: qsize = total number of buffer control blocks circular buffer can hold
 * @param	: dmaflag = 0 = char-by-char, 1 = dma
 * @return	: 0 = OK, -1 = failed 1st calloc, -2 = failed 2nd calloc
 * *************************************************************************/
BaseType_t xSerialTaskSendAdd(UART_HandleTypeDef* p, uint16_t qsize, int8_t dmaflag)
{
	struct SSCIRBUF* ptmp1;
	struct SSCIRBUF* ptmp2;
	struct SERIALSENDTASKBCB** pssb;

taskENTER_CRITICAL();
	/* Add block with circular buffer pointers for this uart/usart to list */
	ptmp1 = (struct SSCIRBUF*)calloc(1, sizeof(struct SSCIRBUF));
	if (ptmp1 == NULL) {taskEXIT_CRITICAL();return -1;}
	if (pbhd  == NULL) // Is this the first?
	{ // Yes
		pbhd = ptmp1;	// Point head to first on list
		ptmp1->pnext = ptmp1; // Point first item on list to self
	}
	else
	{ // No, one or more have been added
		/* Find end of list */
		ptmp2 = pbhd;	// Start at head
		while (ptmp2 != ptmp2->pnext) ptmp2 = ptmp2->pnext;
		ptmp2->pnext = ptmp1; // Last block points to added block
		ptmp1->pnext = ptmp1; // Added block points to self
	}

	/* Get memory for circular buffer of buffer control blocks (bcb) */	
	pssb = (struct SERIALSENDTASKBCB**)calloc(qsize, sizeof(struct SERIALSENDTASKBCB*));
	if ( pssb == NULL) {taskEXIT_CRITICAL();return -2;}

	/* Initialize pointers for circular buffer */
	// ptmp1 points to last item on list
	ptmp1->pbegin  = pssb;
	ptmp1->padd    = pssb;
	ptmp1->ptake   = pssb;
	ptmp1->pend    = pssb + qsize;
	ptmp1->phuart  = p;
	ptmp1->dmaflag = dmaflag;
taskEXIT_CRITICAL();
	return 0;
}

/* *************************************************************************
 * void StartSerialTaskSend(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartSerialTaskSend(void* argument1)
{
	struct SERIALSENDTASKBCB*  pssb; // Copied item from queue
	struct SSCIRBUF* ptmp;	// Circular buffer pointer block pointer

  /* Infinite loop */
  for(;;)
  {
		do
		{
		/* Wait indefinitely for someone to load something into the queue */
		/* Skip over empty returns, and NULL pointers that would cause trouble */
			xQueueReceive(SerialTaskSendQHandle,&pssb,portMAX_DELAY);
		} while ((pssb->phuart == NULL) || (pssb->tskhandle == NULL));

		/* Add Q item to linked list for this uart/usart */

		/* Find uart/usart list for this item from Q */
		ptmp = pbhd;
		while (ptmp->phuart != pssb->phuart) ptmp = ptmp->pnext;

	 	if ((pssb->pbuf == NULL) || (pssb->size == 0))
		{ // Here, HAL is going to reject it
  			/* Release buffer just sent so it can be reused. */
			xSemaphoreGive(pssb->semaphore);
		}
		else
		{
			/* Add bcb to circular buffer for this uart/usart */
			*ptmp->padd = pssb; //Copy BCB pointer into circular buffer

			ptmp->padd += 1;	// Advance list ptr with wraparound
			if (ptmp->padd == ptmp->pend) ptmp->padd = ptmp->pbegin;
			{		
   	   /* If HAL for this uart/usart is busy nothing happens. */
				
				if (ptmp->dmaflag == 0) // send buffer via char-by-char or dma 
				{
		 			HAL_UART_Transmit_IT((UART_HandleTypeDef*)pssb->phuart,pssb->pbuf,pssb->size);
				}
				else
				{
 					HAL_UART_Transmit_DMA((UART_HandleTypeDef*)pssb->phuart,pssb->pbuf,pssb->size);
				}
			}
		}
	}
}
/* *************************************************************************
 * osThreadId xSerialTaskSendCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: SerialTaskSendHandle
 * *************************************************************************/
osThreadId xSerialTaskSendCreate(uint32_t taskpriority)
{
/*
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
const char * const pcName,
unsigned short usStackDepth,
void *pvParameters,
UBaseType_t uxPriority,
TaskHandle_t *pxCreatedTask );
*/
	BaseType_t ret = xTaskCreate(StartSerialTaskSend, "SerialTaskSend",\
     (128), NULL, taskpriority,\
     &SerialTaskHandle);
	if (ret != pdPASS) return NULL;

	SerialTaskSendQHandle = xQueueCreate(QUEUESIZE, sizeof(struct SERIALSENDTASKBCB*) );
	if (SerialTaskSendQHandle == NULL) return NULL;
	return SerialTaskHandle;
}
/* #######################################################################
   UART interrupt callback: file|size has been sent
   ####################################################################### */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *phuart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	struct SERIALSENDTASKBCB* pbcb; // Buffer control block ptr
	struct SSCIRBUF* ptmp1;	// Linked list of usarts

	/* Find bcb circular buffer for this uart */
	ptmp1 = pbhd; // Polnt to first on list
	while (ptmp1->phuart != phuart) 
	{
		ptmp1 = ptmp1->pnext; // Step to next uart
	}

	/* Pointr to buffer control block for next buffer to send. */
	pbcb = *ptmp1->ptake;

   /* Release buffer just sent to it can be reused. */
	xSemaphoreGiveFromISR(pbcb->semaphore,&xHigherPriorityTaskWoken);

	/* Advance 'take' pointer of circular buffer. */
	ptmp1->ptake += 1;	// Advance ptr with wraparound
	if (ptmp1->ptake == ptmp1->pend) ptmp1->ptake = ptmp1->pbegin;	

	/* If more bcb remain in the buffer start the next sending. */
	if (ptmp1->ptake != ptmp1->padd)
	{
		pbcb = *ptmp1->ptake;
		if (ptmp1->dmaflag == 0)
		{
			HAL_UART_Transmit_IT (pbcb->phuart,pbcb->pbuf,pbcb->size);
		}
		else
		{
			HAL_UART_Transmit_DMA(pbcb->phuart,pbcb->pbuf,pbcb->size);
		}
	}
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	return;
}

/* *************************************************************************
 * void vSerialTaskSendQueueBuf(struct SERIALSENDTASKBCB** ppbcb);
 *	@brief	: Load buffer control block onto queue for sending
 * @param	: ppbcb = Pointer to pointer to Buffer Control Block
 * *************************************************************************/
void vSerialTaskSendQueueBuf(struct SERIALSENDTASKBCB** ppbcb)
{
	uint32_t qret;

	do 
	{
		qret=xQueueSendToBack(SerialTaskSendQHandle, ppbcb, portMAX_DELAY);
		if (qret == errQUEUE_FULL) osDelay(1); // Delay, don't spin.

	} while(qret == errQUEUE_FULL);
	return;
}


