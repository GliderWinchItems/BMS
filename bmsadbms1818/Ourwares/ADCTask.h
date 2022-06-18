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
#include "main.h"

#define ADCSEQNUM 16  // Number of ADC scans in 1/2 of the DMA buffer
#define ADCSUMCT 16 // Number of ADC scans in sum

enum TIMSTATE
{
	TIMSTATE_IDLE,
	TIMSTATE_1,
	TIMSTATE_2,
	TIMSTATE_3,
};

enum READCELLS
{
	READCELLSGPIO12,
	READGPIO,
	READSTAT,
	READCONFIG,

};


/* SPI 12 bytes: CMD[2]+PEC[2]+DATA[6]+PEC[2] */
union SPI12
{
	uint8_t   u8[12];
	uint16_t u16[6];
	uint32_t u32[3];
};

struct ADCSPIALL
{
	union SPI12 spitx12; // SPI command sent to '1818'
 	union SPI12 spirx12; // SPI monitor received from '1818'
	uint16_t raw[ADCBMSMAX]; // Raw readings from BMS sequence
	uint16_t cellreg[6*3]; // Cell readings
	uint16_t auxreg [4*3]; // Aux readings
	uint16_t statreg[1*3]; // Status readings
	uint16_t configreg[2*3]; // Configuration register
	uint16_t cellbitssave; // Depends on readbmsfets code
	int16_t  tim15ctr;   // TIM15CH1:OC turnover counter
	uint8_t  timstate;   // State for ISR handling: TIM
	uint8_t  spistate;   // State for ISR handling: SPI
	uint8_t  adcidx;     // ADC array storage index [0->20]
	uint8_t  readbmsfets; // 
};

/* *************************************************************************/
osThreadId xADCTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ADCTaskHandle
 * *************************************************************************/

extern TaskHandle_t ADCTaskHandle;// replace: extern osThreadId ADCTaskHandle;

extern uint32_t adcsumdb[2][ADC1IDX_ADCSCANSIZE]; // Summation of ADC scan 

extern float adcsumfilt[2][ADC1IDX_ADCSCANSIZE];// Filtered

extern uint8_t  adcsumidx; // Index for currently being summed

extern struct ADCSPIALL adcspiall;

#endif

