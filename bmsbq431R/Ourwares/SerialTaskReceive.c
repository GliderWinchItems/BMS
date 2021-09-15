/******************************************************************************
* File Name          : SerialTaskReceive.c
* Date First Issued  : 01/21/2019
* Description        : Serial input using FreeRTOS/ST HAL
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "SerialTaskReceive.h"
#include "stm32l4xx_hal_usart.h"
#include "stm32l4xx_hal_uart.h"
#include "morse.h"
#include "main.h"

/*
BaseType_t Rret; // Return value
...
// Initialization before schedular start
...
// uart handle, and dma flag
Rret = xSerialTaskReceiveAdd(&huart6, 1);
if (Rret != 0) while(1==1); // Hang
...
// Task initialization, before endless loop
...
// uart handle, number line buffers, size of line buffers (not including \0)
#define NOTEBIT 0x1;	// Unique bit in this task for this buffer notification
uint32_t noteval = 0;	// OS copies its notification word upon a "notify"
// uart handle, dma flag, buffer notify bit, number line buffers, size of line buffers);
struct SERIALTASKRCVBCB* pinbuf1 = getserialinbuf(&huart6,1, NOTEBIT,&noteval,10,32); 
...other getserialinbuf(...) if more than one...

...
for ( ;; )
{
...
yscanf(pinbuf1," ...",...);
...
}

*/

static void unloaddma(struct SERIALRCVBCB* prbcb);

//osThreadId SerialTaskReceiveHandle = NULL;
TaskHandle_t SerialTaskReceiveHandle = NULL;

/* THE FOLLOWING COPIED FOR REFERENCE--
// Line buffer control block for one uart 
struct SERIALRCVBCB
{
	struct SERIALRCVBCB* pnext;	// Link to next uart RBCB
	char* pbegin;// Ptr to first line buffer
	char* pend;  // Ptr to last+1 line buffer
	char* padd;  // Ptr to line buffer being filled
	char* ptake; // Ptr to line buffer to take
	char* pwork; // Ptr to next char to be added
	char* pworkend; // Ptr to end of current active line buffer
	UART_HandleTypeDef* phuart;// Pointer to 'MX uart handle
	osThreadId tskhandle;      // Task handle of originating task
	uint32_t  notebit;         // Unique notification bit (within task)
	uint32_t* pnoteval;        // Pointer to word receiving notification 
	char*  pbegindma;          // Pointer to beginning of dma buffer
	char*  penddma;            // Pointer to ebd + 1 of dma buffer
	char*  ptakedma;           // Pointer to last + 1 char taken from dma buffer
	uint32_t  numlinexsize;    // Number of lines * line size (chars)
	uint16_t  linesize;        // Number of chars in each line buffer (1)
	uint16_t  dmasize;         // Number of chars in total circular DMA buffer
	uint8_t   numline;         // Number of line (or CAN msg) buffers for this uart
	int8_t    dmaflag;         // dmaflag = 0 for char-by-char mode; 1 = dma mode (1)
	uint8_t   CANmode;         // 0 = ordinary lines; 1 = ascii/hex CAN
	struct GATEWAYPCTOCAN* pgptc; // Pointer to gateway_PCtoCAN control block
	uint32_t errorct;				// uart error callback counter
};

*/

/* Pointer to linked list of Receive Buffer Control Blocks */
// Initial is NULL; pnext in last points to last
static struct SERIALRCVBCB* prbhd = NULL;

/* *************************************************************************
 * struct SERIALRCVBCB* xSerialTaskRxAdduart(\
		UART_HandleTypeDef* phuart,\
		int8_t    dmaflag,\
		uint32_t  notebit,\
		uint32_t* pnoteval,\
		uint8_t   numline,\
		uint8_t   linesize,\
		char  dmasize,\
		uint8_t   CANmode);
 *	@brief	: Setup circular line buffers this uart
 * @param	: phuart = pointer to uart control block
 * @param	: dmaflag = 0 for char-by-char mode; 1 = dma mode
 * @param	: notebit = unique bit for notification for this task
 * @param	: pnoteval = pointer to word receiving notification word from OS
 * @param	: numline = number of line buffers in circular line buffer
 * @param	: linesize = number of chars in each line buffer
 * @param	: dmasize = number of chars in total circular DMA buffer
 * @param	: CANmode = 0 = straight ascii lines; 1 = convert ascii to CAN msgs
 * @return	: pointer = 'RCVBCB for this uart; NULL = failed
 * *************************************************************************/
struct SERIALRCVBCB* xSerialTaskRxAdduart(\
		UART_HandleTypeDef* phuart,\
		int8_t    dmaflag,\
		uint32_t  notebit,\
		uint32_t* pnoteval,\
		uint8_t   numline,\
		uint8_t   linesize,\
		char  dmasize,\
		uint8_t   CANmode)
{
	struct SERIALRCVBCB* ptmp1;
	struct SERIALRCVBCB* ptmp2;
	char* pbuf;

HAL_StatusTypeDef halret;

	//struct GATEWAYPCTOCAN* pgptc; // Pointer to Gateway Pc To Can

	/* There can be a problem with Tasks not started if the calling task gets here first */
	osDelay(10);

taskENTER_CRITICAL();
	/* Add block with circular buffer pointers for this uart/usart to list */
	ptmp1 = (struct SERIALRCVBCB*)calloc(1, sizeof(struct SERIALRCVBCB));
	if (ptmp1  == NULL) {taskEXIT_CRITICAL();return NULL;}
	if (prbhd  == NULL) // Is this the first?
	{ // Yes.  
		prbhd = ptmp1;	// Point head to first on list
		ptmp1->pnext = ptmp1; // Point first (and last) item on list to self
	}
	else
	{ // No. One or more have been added
		/* Find end of list */
		ptmp2 = prbhd;	// Start at head
		while (ptmp2 != ptmp2->pnext) ptmp2 = ptmp2->pnext;
		ptmp2->pnext = ptmp1; // Last block points to added block
		ptmp1->pnext = ptmp1; // Added (and last) lock points to self
	}

	/* CAN msg conversion depends on line buffer size being large enough for CAN msg. */
	if ((CANmode != 0) && (linesize < sizeof(struct CANRCVBUFPLUS))) 
	     linesize = sizeof(struct CANRCVBUFPLUS);

	/* Get memory for an array of line buffers for this uart */	
	pbuf = (char*)calloc(numline*linesize, sizeof(char));
	if ( pbuf == NULL) {taskEXIT_CRITICAL();return NULL;}

	/* Save parameters */
	// ptmp1 points to last item on list
	ptmp1->numlinexsize = numline*linesize;
	ptmp1->linesize  = linesize;
	ptmp1->numline   = numline;
	ptmp1->dmaflag   = dmaflag;
	ptmp1->pnoteval  = pnoteval;
	ptmp1->notebit   = notebit;
	ptmp1->phuart    = phuart;
	ptmp1->tskhandle = xTaskGetCurrentTaskHandle();
	ptmp1->errorct   = 0;

	/* Initialize line buffer pointers */
	ptmp1->pbegin = pbuf; // First line buffer beginning
	ptmp1->padd   = pbuf; // Pointer to where next line will be added
	ptmp1->ptake  = pbuf; // Pointer to where next line will be taken
	ptmp1->pwork  = pbuf; // Pointer where next char in active line will be added
	ptmp1->pworkend = pbuf + linesize - 2; // End of 1st LINE buffer (allow for zero terminator)
	ptmp1->pend = pbuf + numline*linesize; // End of line buffers + 1 line
	ptmp1->CANmode = CANmode;

	if (dmaflag != 0)
	{ // Circular DMA buffer 
		pbuf = (char*)calloc((int)dmasize, sizeof(char));
		if ( pbuf == NULL) return NULL;
		ptmp1->pbegindma = pbuf;   // Pointer to beginning of DMA circular buffer
		ptmp1->penddma   = pbuf + dmasize; // Pointer to end + 1
		ptmp1->ptakedma  = pbuf;   // "Take" Pointer into DMA buffer
		ptmp1->dmasize   = dmasize; // Total number of chars in DMA buffer

		/* When CANmode is requested, the conversion control block is used */
		if (CANmode == 1)
		{ // Initialize CAN conversion control block
morse_trap(3341);			;
//			pgptc = gateway_PCtoCAN_init(ptmp1);
//			if (pgptc == NULL)  {taskEXIT_CRITICAL();return NULL;}
//			ptmp1->pgptc = pgptc; // Save pointer to CAN conversion control block
		}

		/* Start uart-dma circular mode.  Start once; run forever. */
		halret = HAL_UART_Receive_DMA(ptmp1->phuart, (uint8_t*)ptmp1->pbegindma, ptmp1->dmasize);
		if (halret == HAL_ERROR)
		{
			taskEXIT_CRITICAL();
			return NULL;
		}
	}
	else
	{ // Start char-by-char mode. Restart upon each interrupt.
		halret = HAL_UART_Receive_IT(ptmp1->phuart, (uint8_t*)ptmp1->pwork, 1);
		if (halret == HAL_ERROR)
		{
			taskEXIT_CRITICAL();
			return NULL;
		}
	}
taskEXIT_CRITICAL();
	return ptmp1;	// Success return pointer to this 'BCB
}

/* *************************************************************************
 * void StartSerialTaskReceive(void* argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartSerialTaskReceive(void* argument)
{
	struct SERIALRCVBCB* prtmp = prbhd;
	struct SERIALRCVBCB* prtmp2;
	
	/* Do nothing until at least one tasks calls 
      'xSerialTaskRxAdduart' and sets up the 
       uart and buffering. */

	while (prtmp == NULL)
	{
		osDelay(10);
		prtmp = prbhd;
	}

  /* Infinite loop */
  for(;;)
  {
		/* Wait for one tick or notification from a dma callback */
		xTaskNotifyWait(0, 0, NULL, 2);

		/* Go through list of receiving uarts and unload only dma uart buffers. */
		prtmp = prbhd;
		do
		{
			if (prtmp->dmaflag != 0)
			{ // Here, dma mode
				if (prtmp->CANmode == 1)
				{ // Here, convert to CAN msg buffers
morse_trap(3342);					
					//gateway_PCtoCAN_unloaddma(prtmp);
				}
				else
				{ // Here, straight ascii line buffers
					unloaddma(prtmp);
				}
			}
			prtmp2 = prtmp;
			prtmp = prtmp2->pnext;
		} while (prtmp2->pnext != prtmp2);
  }
}
/* *************************************************************************
 * osThreadId xSerialTaskReceiveCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: SerialTaskReceiveHandle
 * *************************************************************************/
 BaseType_t xSerialTaskReceiveCreate(uint32_t taskpriority)
{
/*
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
const char * const pcName,
unsigned short usStackDepth,
void *pvParameters,
UBaseType_t uxPriority,
TaskHandle_t *pxCreatedTask );
*/
	return xTaskCreate(StartSerialTaskReceive, "StartSerialTaskReceive",\
     96, NULL, taskpriority,\
     &SerialTaskReceiveHandle);
}
/* *************************************************************************
 * char* xSerialTaskReceiveGetline(struct SERIALRCVBCB* pbcb);
 *	@brief	: Load buffer control block onto queue for sending
 * @param	: pbcb = Pointer to Buffer Control Block
 * @return	: Pointer to line buffer; NULL = no new lines
 * *************************************************************************/
char* xSerialTaskReceiveGetline(struct SERIALRCVBCB* pbcb)
{
	char* p = NULL;

	/* Check no new lines. */
	if (pbcb->ptake == pbcb->padd) return p;
	p = pbcb->ptake;

	/* Advance 'take' pointer w wraparound check. */
	pbcb->ptake += pbcb->linesize;
	if (pbcb->ptake >= pbcb->pend) pbcb->ptake = pbcb->pbegin;

	return p;
}
/* *************************************************************************
 * static void advancebuf(struct SERIALRCVBCB* prtmp);
 * @brief	: Advance to next line buffer
 * *************************************************************************/
static void advancebuf(struct SERIALRCVBCB* prtmp)
{		
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* Zero terminator addition. */
	*prtmp->pwork = 0; // Add string terminator
	
	/* Advance to beginning of next line buffer */
	prtmp->padd += prtmp->linesize;	// Step ahead one buffer length
	if (prtmp->padd == prtmp->pend) prtmp->padd = prtmp->pbegin;

	/* Initialize working char pointers */
	prtmp->pwork = prtmp->padd;	// Begin
	prtmp->pworkend = prtmp->padd + prtmp->linesize - 2; // End

	/* Notify originating task know a line is ready. */
	if (prtmp->tskhandle != NULL)
	{
		xTaskNotifyFromISR(prtmp->tskhandle, 
			prtmp->notebit,	/* 'or' bit assigned to buffer to notification value. */
			eSetBits,      /* Set 'or' option */
			&xHigherPriorityTaskWoken );

		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
	return;
}
/* *************************************************************************
 * static void advanceptr(struct SERIALRCVBCB* prtmp);
 * @brief	: Advance pointer within the active line buffer
 * *************************************************************************/
static void advanceptr(struct SERIALRCVBCB* prtmp, char c)
{
	*prtmp->pwork++ = c;
	if ((c == LINETERMINATOR) || (c == 0XD))
	{ // Here End of Line
		advancebuf(prtmp); // Advance to new line buffer and notify originator
		return;
	}

	/* Here, just an ordinary char stored. */
	if (prtmp->pwork == prtmp->pworkend)
	{ // Here we are at end - 1 of line buffer
		advancebuf(prtmp); // Advance to new line buffer and notify originator
	}	
	return;
}

/* *************************************************************************
 * static void unloaddma(struct SERIALRCVBCB* prbcb);
 * @brief	: DMA: Check for line terminator and store; enter from task poll
 * @param	: prbcb = pointer to buffer control block for uart causing callback
 * *************************************************************************/
static void unloaddma(struct SERIALRCVBCB* prbcb)
{
	uint16_t dmandtr;	// Number of data items remaining in DMA NDTR register
	int32_t diff;
	char c;

// bsp_uart.c handling of dma
//		Diff = ( pctl->rxbuff_end - DMA_SNDTR(pctl->idma,pctl->rxdma_stream) - pctl->rxbuff_out );
//		if (Diff < 0)
//			Diff += pctl->rxbuff_size;  // Adjust for wrap

		/* Get number of data item count in DMA buffer "now" from DMA NDTR register. */
		dmandtr = __HAL_DMA_GET_COUNTER(prbcb->phuart->hdmarx); 

		/* Difference between where we are taking out chars, and where DMA is or was storing. */
		diff = prbcb->penddma - dmandtr - prbcb->ptakedma; 
		if (diff < 0)
		{ // Wrap around
			diff += prbcb->dmasize;
		}

		/* Copy dma circular buffer into buffered lines */
		while (diff > 0)
		{
			diff -= 1;
			c = *prbcb->ptakedma++; // Get char from dma buffer
			if (prbcb->ptakedma == prbcb->penddma) prbcb->ptakedma = prbcb->pbegindma;
			
			advanceptr(prbcb,c);
		}
		return;
}

/* #######################################################################
   UART interrupt callbacks
   ####################################################################### */
/* *************************************************************************
 * void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *phuart);
 * @brief	: DMA callback at the halfway point in the circular buffer
 * *************************************************************************/
/* NOTE: under interrupt from callback. */

/* DMA Half buffer complete callback (dma only) */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *phuart)
{
	HAL_UART_RxCpltCallback(phuart);
}
/* *************************************************************************
 * void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *phuart);
 * @brief	: DMA callback at the halfway point in the circular buffer
 *				: OR, char-by-char completion of sending
 * *************************************************************************/
/* DMA buffer complete, => OR <= char-by-char complete */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *phuart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* Look up buffer control block, given uart handle */
	struct SERIALRCVBCB* prtmp = prbhd;
	while (prtmp->phuart != phuart) 
	{
		prtmp++;
		if (prtmp == prtmp->pnext)      morse_trap(553);
		if (prtmp > (struct SERIALRCVBCB*)0x2001ff00) morse_trap(554);
	}

	/* Note char-by-char mode from dma mode. */
	if (prtmp->dmaflag == 0)
	{ // Here char-by-char interrupt mode

		// Note: char-by-char stores directly to line buffer
		advanceptr(prtmp,*prtmp->pwork); 

		/* Restart receiving one char. */
		HAL_UART_Receive_IT(phuart, (uint8_t*)prtmp->pwork, 1); // Get next char		
		return;
	}

	if (SerialTaskReceiveHandle == NULL) return;

	/* Trigger Recieve Task to poll dma uarts */
	xTaskNotifyFromISR(SerialTaskReceiveHandle, 
		0,	/* 'or' bit assigned to buffer to notification value. */
		eSetBits,      /* Set 'or' option */
		&xHigherPriorityTaskWoken ); 

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	return;
}
/* *************************************************************************
 * void HAL_UART_ErrorCallback(UART_HandleTypeDef *phuart);
 *	@brief	: Call back from receive errror, stm32f4xx_hal_uart
 * *************************************************************************/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *phuart)
{
	/* Look up buffer control block, given uart handle */
	/* Look up buffer control block, given uart handle */
	struct SERIALRCVBCB* prtmp = prbhd;
	while (prtmp->phuart != phuart) prtmp++;
	prtmp->errorct += 1;
	return;
}


