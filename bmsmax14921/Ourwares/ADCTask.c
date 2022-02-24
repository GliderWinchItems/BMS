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

struct ADCREADREQ* pssb; // Pointer to struct for request details

/* Queue */
#define QUEUESIZE 16	// Total size of bcb's tasks can queue up
osMessageQId ADCTaskReadReqQHandle;

struct ADCSPIALL adcspiall;

uint8_t readyflag = 0;

static struct ADCREADREQ adccalibreq;

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
	uint16_t* pdma;
	float ftmp;
	uint32_t tmp;
	
	/* A notification copies the internal notification word to this. */
	uint32_t noteval = 0;    // Receives notification word upon an API notify

	/* Initialize params for ADC. */
	adcparams_init();

	/* Calibration sequence before enabling ADC. */
	ret = HAL_ADCEx_Calibration_Start(phadc, ADC_SINGLE_ENDED);
	if (ret == HAL_ERROR)  morse_trap(330);

	/* Do initial self-calibration. */
	// Setup dummy struct for request by this task.
	pssb = &adccalibreq;
	pssb->taskhandle = xTaskGetCurrentTaskHandle(); // Requesting task's handle
	pssb->tasknote = TSKNOTEBIT00; // Requesting task's notification bit
	pssb->taskdata = NULL;   // Requesting task's pointer to buffer to receive data
	pssb->updn     = 0;      // see above 'struct ADCSPIALL'
	pssb->reqcode  = REQ_OPENCELL; // Code for service requested
	adcspi_calib();

  	/* Infinite loop */
  	for(;;)
  	{
  		/* Wait for someone to requrest a reading. */
		/* Skip over NULL pointers that would cause trouble */
		do
		{
			xQueueReceive(ADCTaskReadReqQHandle ,&pssb, portMAX_DELAY);
if ((pssb->tskhandle == NULL)) morse_trap(802); // JIC debugging
		} while (pssb->tskhandle == NULL);

		/* Execute request. */
		switch (pssb->reqcode)
		{
		case REQ_BOGUS: // JIC debug
			morse_trap(811);
			break;
		case REQ_READBMS:   // Read MAX14921
			adcspi_readbms();
			break;
		case REQ_CALIB:    // Execute a self-calib (MAX14921 Aout offset) cycle
			adcspi_calib();
			break;
		case REQ_OPENCELL:// Do an open cell wire test
			adcspi_opencell();
			break;
		case REQ_LOWPOWER:// Place MAX14921 into low power mode.
			adcspi_lowpower();
			break;
		case REQ_READADC: // Read non-MAX14921 ADC inputs
			adcspi_readadc();
			break;
		case REQ_SETFETS: // Set discharge FETs
			adcspi_setfets();
			break;
		default:
			morse_trap(806); // Debugging trap
			break;
		}

		// Notify requesting task that data request has completed
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
