/******************************************************************************
* File Name          : bmsspi.h
* Board              : bmsbms1818: STM32L431
* Date First Issued  : 06/17/2022
* Description        : SPI management with ADBMS1818 board
*******************************************************************************/
#ifndef __BMSSPI
#define __BMSSPI

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"


/* *************************************************************************/
 void bmsspi_wakeseq(void);
/* @brief	: Execute a non-interrupt driven '1818 wake up sequence.
 * *************************************************************************/
 void bmsspi_preinit(void);
/* @brief	: Initialization
 * *************************************************************************/
 void bmsspi_readbms(void);
/* @brief	: Read BMS plus GPIO1,2
 * *************************************************************************/
 void bmsspi_gpio(void);
/* @brief	: Read BMS 9 GPIOs & calibrate temperature sensors
 * *************************************************************************/
void bmsspi_setfets(void);
/* @brief	: Load discharge fet settings into '1818 & set discharge timer
 * *************************************************************************/
uint8_t bmsspi_keepawake(void);
/* @brief	: Execute a valid command to keep awake
 * @return  : state number of latest reading
 * *************************************************************************/
 
/* *************************************************************************/
void bmsspi_writereg(uint8_t code);
/* @brief   : Write register group(s)
 * @param   : code = code for selection of register group
 * *************************************************************************/
void bmsspi_readstuff(uint8_t code);
/* @brief	: Do conversion, then read registers with results
 * @brief   : code = code for selection of register group
 * *************************************************************************/
void bmsspi_rw_cmd(const uint16_t* pcmd, uint16_t* pdata, uint8_t rw);
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
 void bmsspi_tim2(TIM_TypeDef  *pT2base);
/* @brief	: TIM2:CC4IF interrupt (arrives here from FanTask.c)
 * @param	: pT2base = pointer to TIM2 register base
* ####################################################################### */
void bmsspi_tim15_IRQHandler(void);
/* @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
void bmsspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx);
void bmsspi_spidmatx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_tx);
/* SPI tx & rs dma transfer complete (if enabled!)
   ####################################################################### */
void EXTI4_IRQHandler(void);
/* PB4 SPI MISO pin-1818 SDO pin: interrupt upon rising edge
   ####################################################################### */	

#endif
