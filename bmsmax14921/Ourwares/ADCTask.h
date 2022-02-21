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

struct ADCREADOUTCTL
{
	uint32_t* ptr;  // Pointer for storing adc data
	uint32_t* endA; // Pointer for ending BMS cell sequence
	uint32_t* endT; // Pointer for ending BMS thermister sequence
	uint8_t   idxspi;  // Index of spi setup
	uint8_t   spiflag; // 1 = spi busy; 0 = spi idle
	uint8_t   adcflag; // 1 = adc busy; 0 = adc idle
};


struct ADCSPIALL
{
struct ADCREADOUTCTL adcctl;
union SPI24 spitx24;
union SPI24 spirx24;
uint8_t* pspirx24;
uint8_t cellnum;
uint8_t adcstate;
uint8_t timstate;
uint8_t spistate;
uint8_t readyflag;
uint8_t adcidx;
uint8_t spiidx;
uint8_t updn;  // Readout "up" (cells 1->16) = 1; Down (cells 16->1) = 0
};

/* Queue for requesting a readout. */
struct ADCREADREQ
{
	osThreadId	taskhandle; // Requesting task's handle
	BaseType_t  tasknote;   // Requesting task's notification bit
	uint32_t*   taskdata;   // Requesting task's pointer to buffer to receive data
};


/* *************************************************************************/
osThreadId xADCTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ADCTaskHandle
 * *************************************************************************/

//extern osThreadId ADCTaskHandle;
extern TaskHandle_t ADCTaskHandle;

// Read request queue
extern osMessageQId ADCTaskReadReqQHandle;

extern struct ADCSPIALL adcspiall;




#endif

