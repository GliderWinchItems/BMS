/******************************************************************************
* File Name          : ADCTask.c
* Board              : BMScable: STM32L431
* Date First Issued  : 02/01/2019
* Description        : Processing ADC readings after ADC/DMA issues interrupt
*******************************************************************************/
/* 10/23/2020: Revised for Levelwind */

#include "ADCTask.h"
#include "adctask.h"
#include "morse.h"

#include "main.h"
#include "DTW_counter.h"
#include "iir_f1.h"
#include "adcbms.h"
#include "adcspi.h"

extern ADC_HandleTypeDef hadc1;
extern osThreadId_t defaultTaskHandle;

extern uint32_t	dbisrflag;


/* Queue */
#define QUEUESIZE 16	// Total size of bcb's tasks can queue up
osMessageQId ADCTaskReadReqQHandle;
struct ADCREADREQ* pssb; // Pointer to struct for request details
static struct ADCREADREQ adccalibreq;

/* Just about everything for reading BMS with ADC/SPI. */
struct ADCSPIALL adcspiall;

//float adcf[ADCDIRECTMAX]; // Calibrated

uint8_t readyflag = 0;

TaskHandle_t ADCTaskHandle;

uint32_t adcdbctr = 0;// debug

/* *************************************************************************
 * void StartADCTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
uint16_t* p16;
uint32_t dwt1,dwt2,dwtdiff;

void StartADCTask(void *argument)
{
	BaseType_t qret;
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	
	/* Initialize params for ADC. */
	adcparams_init();

	/* Save some of the intialized registers. */
	adcbms_preinit();

	/* ADC calibration sequence before enabling ADC. */
	BaseType_t ret = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	if (ret == HAL_ERROR)  morse_trap(330);

	/* Max14921 output buffer offset self-calibration. */
	// Setup dummy struct for request by this task.
	pssb = &adccalibreq; // Point to "our" request struct
	pssb->taskhandle = xTaskGetCurrentTaskHandle(); // Requesting task's handle
	pssb->tasknote = TSKNOTEBIT00; // Requesting task's notification bit
	pssb->taskdata = NULL;   // Requesting task's pointer to buffer to receive data
	pssb->updn     = 0;      // see above 'struct ADCSPIALL'
	pssb->reqcode  = REQ_OPENCELL; // Code for service requested

// $$$$$$	adcspi_calib();

	// Default to overlapped spi commands with ADC conversions
	p->noverlap = 0;

	// Assure configuration of ADC mode upon first pass. */
	p->config = 2; // Show ADC not configured


  	/* Infinite loop */
  	for(;;)
  	{
  		/* Check queue of any items, but do not wait. */
		qret = xQueueReceive(ADCTaskReadReqQHandle ,&pssb, 0);
		if (qret != pdPASS)
		{ // No item was in the queue. Read ADC channels
			adcspi_readadc();
		}
		else
		{ // Request arrived

if ((pssb->taskhandle == NULL)) morse_trap(802); // JIC debugging

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
