/******************************************************************************
* File Name          : adcspi.h
* Board              : bmsbms1818: STM32L431
* Date First Issued  : 06/17/2022
* Description        : SPI management with ADBMS1818 board
*******************************************************************************/
/* 

*/
#ifndef __ADCSPI
#define __ADCSPI

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"


/* *************************************************************************/
 void adcspi_preinit(void);
/* @brief	: Initialization
 * *************************************************************************/


/* #################### interrupt ######################################## */
 void adcspi_tim2(TIM_TypeDef  *pT2base);
/* @brief	: TIM2:CC4IF interrupt (arrives here from FanTask.c)
 * @param	: pT2base = pointer to TIM2 register base
* ####################################################################### */
void adcspi_tim15_IRQHandler(void);
/* @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
void adcspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx);
void adcspi_spidmatx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_tx);
/* SPI tx & rs dma transfer complete (if enabled!)
   ####################################################################### */
void EXTI4_IRQHandler(void);
/* PB4 SPI MISO pin-1818 SDO pin: interrupt upon rising edge
   ####################################################################### */	

extern uint8_t readbmsflag; // Let main know a BMS reading was made

#endif
