/******************************************************************************
* File Name          : adcbms.h
* Board              : bms14921: STM32L431
* Date First Issued  : 02/17/2022
* Description        : ADC management with MAX1421 board
*******************************************************************************/
/* 

*/
#ifndef __ADCBMS
#define __ADCBMS

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

/* *************************************************************************/
void adcbms_preinit(void);
/* @brief	: Save some things used later.
 * *************************************************************************/
void adcbms_config_adc(void);
/* @brief	: Set up for STM32L431 ADC readouts (excluding ADC used with MAX14921)
 * *************************************************************************/
 void adcbms_config_bms(void);
/* @brief	: Set up for MAX14921 BMS cell readout
 * *************************************************************************/
 void adcbms_startreadbms(void);
/* @brief	: Set up and start a readout sequence
 * *************************************************************************/

extern uint32_t* pspi; // SPI register base pointer.

#endif
