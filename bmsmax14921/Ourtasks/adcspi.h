/******************************************************************************
* File Name          : adcspi.h
* Board              : bms14921: STM32L431
* Date First Issued  : 02/19/2022
* Description        : ADC w SPI management with MAX1421 board
*******************************************************************************/
/* 

*/
#ifndef __ADCSPI
#define __ADCSPI

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

union SPI24
{
	uint8_t  uc[4];
	uint16_t us[2];
	uint32_t ui;

};


extern union SPI24 spitx24;
extern union SPI24 spirx24;



/* #################### interrupt ######################################## */
 void adcspi_tim2(TIM_TypeDef  *pT2base);
/* @brief	: TIM2:CC4IF interrupt (arrives here from FanTask.c)
 * @param	: pT2base = pointer to TIM2 register base
/* ####################################################################### */
void HAL_SPI_MasterRxCpltCallback(void);
/* SPI DMA RX complete callback
 * RX lags TX slightly, so RX is used
/* ####################################################################### */
void adcspi_adc_Handler(ADC_HandleTypeDef* phadc1);
/* ADC interrupt (from .it)
   ####################################################################### */

#endif
