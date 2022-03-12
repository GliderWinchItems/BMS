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

/* *************************************************************************/
 void adcspi_readadc(void);
/* @brief	: Read selected ADCs (non-MAX14921)
 * *************************************************************************/
void adcspi_readbms(void);
/* @brief	: Do a sequence for reading MA14921 plus direct ADC inputs
 * *************************************************************************/
void adcspi_calib(void);
/* @brief	: Execute a self-calib sequence: nulls BMS output buffer offset.
 * *************************************************************************/
 void adcspi_opencell(void);
/* @brief	: Check for open cell wires
 * *************************************************************************/
void adcspi_lowpower(void);
/* @brief	: Place MAX14921 into low power mode
 * *************************************************************************/
void adcspi_setfets(void);
/* @brief	: Read selected ADCs (non-MAX14921)
 * *************************************************************************/
void adcspi_init(void);
/* @brief	: 
 * *************************************************************************/

/* #################### interrupt ######################################## */
 void adcspi_tim2(TIM_TypeDef  *pT2base);
/* @brief	: TIM2:CC4IF interrupt (arrives here from FanTask.c)
 * @param	: pT2base = pointer to TIM2 register base
* ####################################################################### */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
/* SPI DMA RX complete callback
 * RX lags TX slightly, so RX is used
* ####################################################################### */
void adcspi_adc_IRQHandler(ADC_HandleTypeDef* phadc1);
/* ADC interrupt (from .it)
   ####################################################################### 
 * DMA CH1 interrupt (from stm32l4xx_it.c) */
void  adcspi_dma_handler(void);
 /*  ####################################################################### */
void adcspi_tim15_IRQHandler(void);
/* @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */

extern uint8_t readbmsflag; // Let main know a BMS reading was made

#endif
