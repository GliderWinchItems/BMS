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
#include "adcextendsum.h"

#include "main.h"
#include "DTW_counter.h"
#include "iir_f1.h"

extern ADC_HandleTypeDef hadc1;

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

	/* ==> SHAMLESS! <== injected ADC1 data register pointer. */
	#define JDRVREF  ((uint16_t*)(0x50040000 + 0x80)) // Vrefint 
	#define JDRVTEMP ((uint16_t*)(0x50040000 + 0x80 + 0x04)) // Vtemp

	uint16_t* pdma;
	float ftmp;
	
	/* A notification copies the internal notification word to this. */
	uint32_t noteval = 0;    // Receives notification word upon an API notify

	/* Get buffers, "our" control block, and start ADC/DMA running. */
	struct ADCDMATSKBLK* pblk = adctask_init(&hadc1,TSK02BIT02,TSK02BIT03,&noteval);
	if (pblk == NULL) {morse_trap(15);}


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
//		adcfastsum16(&adc1.chan[0], pdma); // Fast in-line addition
		adc1.ctr += 1; // Update count

dwt1 = DTWTIME;
dwtdiff = dwt1 - dwt2;
dwt2 = dwt1;

//		padcsum = &adcsumdb[adcsumidx][0]; // Redundant

		*(padcsum +  0) += *(pdma +  0); // 
		*(padcsum +  1) += *(pdma +  1); // 
		*(padcsum +  2) += *(pdma +  2); // 
		*(padcsum +  3) += *(pdma +  3); // 
		*(padcsum +  4) += *(pdma +  4); // 
		*(padcsum +  5) += *(pdma +  5); // 
		*(padcsum +  6) += *(pdma +  6); // 
		*(padcsum +  7) += *(pdma +  7); // 
		*(padcsum +  8) += *(pdma +  8); // 
		*(padcsum +  9) += *(pdma +  9); // 
		*(padcsum + 10) += *(pdma + 10); // 
		*(padcsum + 11) += *(pdma + 11); // 
		*(padcsum + 12) += *(pdma + 12); // 
		*(padcsum + 13) += *(pdma + 13); // 
		*(padcsum + 14) += *(pdma + 14); // 
		*(padcsum + 15) += *(pdma + 15); // 
		*(padcsum + 16) += *(JDRVREF ); // Vrefint
		*(padcsum + 17) += *(JDRVTEMP); // Vtemp

		adcsumctr += 1;
		if (adcsumctr >= ADCSUMCT)
		{ // Here, summation complete. 
			padcfilt = &adcsumfilt[adcsumidx][0];

			// Calibrate and Pass sum through IIR filter
			for (int i = 0; i < 18; i++)
			{ // Calibrate and filter sums
				ftmp = *(padcsum + i);
				// y = a + b * x;
//				ftmp =  adc1.lc.cabs[i].scale * ftmp;
				ftmp = adc1.lc.cabs[i].offset + adc1.lc.cabs[i].scale * ftmp;
				//*(padcfilt + i) = ftmp;
				*(padcfilt + i) = iir_f1_f(&adc1.lc.cabs[i].iir_f1, ftmp);
			}


			// Prepare for next summation.
			adcsumctr  = 0;	// Reset sum counter
			adcsumidx ^= 1; // Switch to alternate summation array
			padcsum = &adcsumdb[adcsumidx][0]; 

			*(padcsum +  0) = 0; // 
			*(padcsum +  1) = 0; // 
			*(padcsum +  2) = 0; // 
			*(padcsum +  3) = 0; // 
			*(padcsum +  4) = 0; //  
			*(padcsum +  5) = 0; //  
			*(padcsum +  6) = 0; //  
			*(padcsum +  7) = 0; // 
			*(padcsum +  8) = 0; // 
			*(padcsum +  9) = 0; // 
			*(padcsum + 10) = 0; // 
			*(padcsum + 11) = 0; // 
			*(padcsum + 12) = 0; // 
			*(padcsum + 13) = 0; // 
			*(padcsum + 14) = 0; // 
			*(padcsum + 15) = 0; // 
			*(padcsum + 16) = 0; // 
			*(padcsum + 17) = 0; // 

			/* Notify that new readings are ready. */
			decimatectr += 1;
			if (decimatectr >= ADCFILTDECIMATE)
			{
				decimatectr = 0;
				xTaskNotify(defaultTaskHandle, DEFAULTTASKBIT00, eSetBits);
			}
		}
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

   	return ADCTaskHandle;	
}

