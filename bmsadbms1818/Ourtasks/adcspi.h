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
static uint16_t readreg(uint16_t* p, uint16_t* pcmdr, uint8_t n);
/* @brief	: Read register, convert endianness
 * @param   : p = pointer to output array (little endian)
 * @param   : pcmdr = pointer to read command (NULL = skip conversionc command)
 * @param   ; n = number of register reads
 * @return  : 0 = no PEC15 error
 * *************************************************************************/
static void writereg(uint16_t* pcmdw, uint16_t* p);
/* @brief	: Write register with convert endianness
 * @param   : p = pointer to input array (little endian)
 * @param   : pcmdw = pointer to write command
 * *************************************************************************/
void adcspi_writereg(uint8_t code);
/* @brief   : Write register group(s)
 * @param   : code = code for selection of register group
 * *************************************************************************/
void adcspi_readstuff(uint8_t code);
/* @brief	: Do conversion, then read registers with results
 * @brief   : code = code for selection of register group
 * *************************************************************************/
void adcspi_rw_cmd(uint16_t* pcmd, uint16_t* pdata, unit8_t rw);
/* @brief	: Send command  and write data (little endian)
 * @param   : pcmd = pointer to 2 byte command (little endian)
 * @brief   : pdata = pointer to six bytes to be written (little endian),
 *          :    ignore pdata for read commands.
 * @brief   : rw = type of command sequence
 *          : 0 = Send 2 byte command  + pec
 *          : 1 = Send command+pec, plus 6 bytes data+pec
 *          : 2 = Send command+pec, read 6 bytes data+pec into spirx12.uc[4]-[11]
 *          : 3 = Send 2 byte command  + pec. Switch '1818 SDO to interrupt conversion completion
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
