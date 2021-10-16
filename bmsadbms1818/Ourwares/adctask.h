/******************************************************************************
* File Name          : adctask.h
* Board              : BMScable: STM32L431
* Date First Issued  : 02/01/2019
* Description        : Handle ADC w DMA using FreeRTOS/ST HAL within a task
*******************************************************************************/
/* 
08/05/2021: Update for STM32L431RB BMScable project. 
*/
#ifndef __ADCTASKLC
#define __ADCTASKLC

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

#define ADCNUM 1  // Number of ADC modules

/* Augment 'MX adc control block for dma buffering and summing */
struct ADCDMATSKBLK
{
	struct ADCDMATSKBLK* pnext;
	ADC_HandleTypeDef* phadc; // Pointer to 'MX adc control block
	uint32_t  notebit1; // Notification bit for dma half complete interrupt
	uint32_t  notebit2; // Notification bit for dma complete interrupt
	uint32_t* pnoteval; // Pointer to notification word
	uint16_t* pdma1;    // Pointer to first half of dma buffer
	uint16_t* pdma2;    // Pointer to second half of dma buffer
	osThreadId adctaskHandle; // Task to notify upon DMA interrupt
};

/* *************************************************************************/
struct ADCDMATSKBLK* adctask_init(ADC_HandleTypeDef* phadc,\
	uint32_t  notebit1,\
	uint32_t  notebit2,\
	uint32_t* pnoteval);
/*	@brief	: Setup ADC DMA buffers and control block
 * @param	: phadc = pointer to ADC control block
 * @param	: notebit1 = unique bit for notification @ 1/2 dma buffer
 * @param	: notebit2 = unique bit for notification @ end dma buffer
 * @param	: pnoteval = pointer to word receiving notification word from OS
 * @return	: NULL = fail
 * *************************************************************************/

extern struct ADCDMATSKBLK adc1dmatskblk[ADCNUM];

/* Calibration values common to all ADC modules. */
extern struct ADCCALCOMMON adcommon;

#endif

