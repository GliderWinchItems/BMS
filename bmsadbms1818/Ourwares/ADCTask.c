/******************************************************************************
* File Name          : ADCTask.c
* Board              : bmsadbms1818: STM32L431
* Date First Issued  : 06/19/2022
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
//#include "adcextendsum.h"

#include "main.h"
#include "DTW_counter.h"
#include "iir_f1.h"

extern ADC_HandleTypeDef hadc1;

/* Summation of one ADC scan (with oversampling) */
// One array being filled while other being processed
// Size is DMA (regular conversions) + plus (injected Vrefint, Vtemp)
uint32_t adcsumdb[2][ADC1IDX_ADCSCANSIZE]; 
static uint32_t* padcsum = &adcsumdb[0][0];
uint8_t  adcsumidx = 0;

// IIR filtering of adcsumdb[][]
float adcsumfilt[2][ADC1IDX_ADCSCANSIZE];
float* padcfilt = &adcsumfilt[0][0];

static uint8_t decimatectr = 0; 

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

	uint16_t* pdma;
	float ftmp;
	struct ADCCHANNEL* pz;
	
	/* A notification copies the internal notification word to this. */
	uint32_t noteval = 0;    // Receives notification word upon an API notify

	/* Initialize SPI-DMA CRC ... */
	adcspi_preinit();

	/* Initialize params for ADC. */
	adcparams_init();	

	/* Get buffers, "our" control block, and start ADC/DMA running. */
	struct ADCDMATSKBLK* pblk = adctask_init(&hadc1,TSK02BIT02,TSK02BIT03,&noteval);
	if (pblk == NULL) {morse_trap(15);}

	/* ADC calibration sequence before enabling ADC. */
	ret = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	if (ret == HAL_ERROR)  morse_trap(330);


  	/* Infinite loop */
  	for(;;)
  	{
		/* Wait for DMA interrupt */
		xTaskNotifyWait(0,0xffffffff, &noteval, portMAX_DELAY);

		if (noteval & TSK02BIT02)
		{
			pdma = adc1dmatskblk[0].pdma1;
p16 = pdma;			
		}
		else
		{
			pdma = adc1dmatskblk[0].pdma2;
		}

		/* Sum the readings 1/2 of DMA buffer to an array. */
		adcfastsum16(&adc1.chan[0], pdma); // Fast in-line addition
		adc1.ctr += 1; // Update count

dwt1 = DTWTIME;
dwtdiff = dwt1 - dwt2;
dwt2 = dwt1;

		// Calibrate and Pass sum through IIR filter
		padcfilt = &adcsumfilt[adcsumidx][0];
		pz = &adc1.chan[0];
		for (int i = 0; i < ADC1IDX_ADCSCANSIZE; i++)
		{ // Calibrate and filter sums
			ftmp = adc1.chan[i].sum; //*(padcsum + i); // Convert to floats
			// y = a + b * x;
			ftmp = adc1.lc.cabs[i].offset + adc1.lc.cabs[i].scale * ftmp;
//			*(padcfilt + i) = ftmp;
			*(padcfilt + i) = iir_f1_f(&adc1.lc.cabs[i].iir_f1, ftmp);
			pz->sum = 0;
			pz += 1;
		}
		adcsumidx ^= 1; // Switch to alternate summation array

		/* Notify that new readings are ready. */
		// Throttle 'main' notifications
		decimatectr += 1;
		if (decimatectr >= 12) 
		{
			decimatectr = 0;
			xTaskNotify(defaultTaskHandle, DEFAULTTASKBIT00, eSetBits);
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

   	return ADCTaskHandle;	
}

