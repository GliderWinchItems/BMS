/******************************************************************************
* File Name          : ADCTask.c
* Board              : BMScable: STM32L431
* Date First Issued  : 02/01/2019
* Description        : Processing ADC readings after ADC/DMA issues interrupt
*******************************************************************************/
/* 10/23/2020: Revised for Levelwind */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "ADCTask.h"
#include "adctask.h"
#include "morse.h"
#include "adcfastsum16.h"

#include "main.h"
#include "DTW_counter.h"
#include "iir_f1.h"

extern ADC_HandleTypeDef hadc1;

/* Queue */
#define QUEUESIZE 16	// Total size of bcb's tasks can queue up
osMessageQId ADCTaskReadReqQHandle;

struct ADCSPIALL adcspiall;


/* Summation of one ADC scan (with oversampling) */
// One array being filled while other being processed
// Size is DMA (regular conversions) + plus (injected Vrefint, Vtemp)
uint32_t adcsumdb[2][ADC1IDX_ADCSCANSIZE + 2]; 
static uint32_t* padcsum = &adcsumdb[0][0];
uint8_t  adcsumidx = 0;
static uint8_t  adcsumctr = 0;

// IIR filtering of adcsumdb[][]
#define ADCFILTDECIMATE 8  // Post filtering decimate
float adcsumfilt[2][ADC1IDX_ADCSCANSIZE + 2];
float* padcfilt = &adcsumfilt[0][0];
static uint8_t decimatectr = 0; 

uint32_t adcdbctr = 0;// debug

TaskHandle_t ADCTaskHandle;

extern osThreadId_t defaultTaskHandle;

float fclpos;

/* *************************************************************************
 * void StartADCTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
uint16_t* p16;
uint32_t dwt1,dwt2,dwtdiff;

void StartADCTask(void *argument)
{
	#define TSK02BIT02	(1 << 0)  // Task notification bit for ADC dma 1st 1/2 (adctask.c)
	#define TSK02BIT03	(1 << 1)  // Task notification bit for ADC dma end (adctask.c)

	struct ADCREADREQ* pssb; // Pointer to "stuff" for read data request

	uint16_t* pdma;
	float ftmp;
	
	/* A notification copies the internal notification word to this. */
	uint32_t noteval = 0;    // Receives notification word upon an API notify

	/* Get buffers, "our" control block, and start ADC/DMA running. */
	struct ADCDMATSKBLK* pblk = adctask_init(&hadc1,TSK02BIT02,TSK02BIT03,&noteval);
	if (pblk == NULL) {morse_trap(15);}

	/* Initialize MAX14921 */
	adcspi_init();

  	/* Infinite loop */
  	for(;;)
  	{
  		/* Wait for someone to requrest a reading, or just timeout. */
		/* Skip over empty returns, and NULL pointers that would cause trouble */
		do
		{
			xQueueReceive(ADCTaskReadReqQHandle ,&bqfunction.lc.adc_hb);
		} while ((pssb->tskhandle == NULL) || (pssb->taskdata == NULL));

		// Start read sequence: supply pointer to received data buffer
		adcbms_startread(pssb->taskdata);

		// Wait for sequence to complete
		// Once started, sequence driven by interrupts until complete
		xTaskNotifyWait(0,0xffffffff, &noteval, 10000);

		// Notify requesting task that data readout has completed
		xTaskNotify(pssb->taskhandle, pssb->tasknote, eSetBits);

		// Running count, jic
		adcdbctr += 1;
  	}
}
/* *************************************************************************
 * osThreadId xADCTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ADCTaskHandle
 * *************************************************************************/
osThreadId xADCTaskCreate(uint32_t taskpriority)
{
	BaseType_t xRet;

	xRet = xTaskCreate(
		StartADCTask,     /* Function that implements the task. */
		"ADCTask",        /* Text name for the task. */
		256,              /* Stack size in words, not bytes. */
		NULL,             /* Parameter passed into the task. */
		taskpriority,     /* Priority at which the task is created. */
		&ADCTaskHandle ); /* Used to pass out the created task's handle. */ 

	if( xRet != pdPASS )return NULL;

	ADCTaskReadReqQHandle = xQueueCreate(QUEUESIZE, sizeof(struct ADCREADREQ*) );
	if (ADCTaskReadReqQHandle == NULL) return NULL;

   	return ADCTaskHandle;	
}

