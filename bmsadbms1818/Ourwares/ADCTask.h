/******************************************************************************
* File Name          : ADCTask.h
* Board              : bmsadbms1818: STM32L431
* Date First Issued  : 06/19/2022
* Description        : Processing ADC readings after ADC/DMA issues interrupt
*******************************************************************************/
#ifndef __ADCTASK
#define __ADCTASK

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "adcparams.h"
#include "main.h"



#define ADCSEQNUM 16  // Number of ADC scans in 1/2 of the DMA buffer
#define ADCSUMCT 16 // Number of ADC scans in sum
#define ADCEXTENDSUMCT       64 // Sum of 1/2 DMA sums for additional averaging

/* *************************************************************************/
osThreadId xADCTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ADCTaskHandle
 * *************************************************************************/

extern TaskHandle_t ADCTaskHandle;// replace: extern osThreadId ADCTaskHandle;

//extern uint32_t adcsumdb[2][ADC1IDX_ADCSCANSIZE]; // Summation of ADC scan 

//extern float adcsumfilt[2][ADC1IDX_ADCSCANSIZE];// Filtered

extern uint8_t  adcsumidx; // Index for currently being summed

#endif

