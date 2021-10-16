/******************************************************************************
* File Name          : ADCTask.h
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : ADC w DMA using FreeRTOS/ST HAL
*******************************************************************************/

#ifndef __ADCTASK
#define __ADCTASK

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "adcparams.h"

#define ADCSEQNUM 1  // Number of ADC scans in 1/2 of the DMA buffer
#define ADCSUMCT 16 // Number of ADC scans in sum

/* *************************************************************************/
osThreadId xADCTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ADCTaskHandle
 * *************************************************************************/

//extern osThreadId ADCTaskHandle;
extern TaskHandle_t ADCTaskHandle;

// Summation of ADC scan 
extern uint32_t adcsumdb[2][ADC1IDX_ADCSCANSIZE + 2]; 
// Filtered
extern float adcsumfilt[2][ADC1IDX_ADCSCANSIZE + 2];

extern uint8_t  adcsumidx; // Index for currently being summed



#endif

