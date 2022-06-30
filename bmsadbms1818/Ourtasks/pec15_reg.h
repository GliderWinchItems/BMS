/******************************************************************************
* File Name          : pec15_reg.h
* Date First Issued  : 06/11/2022
* Description        : ADBMS1818 PEC computation: 16 1/2 word table lookup
*******************************************************************************/
#include <stdint.h>

#ifndef __PEC15_REG
#define __PEC15_REG

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_crc.h"

/* *************************************************************************/
void pec15_reg_init (void);
/*  @brief  : Iniitalize RCC and CRCregisters for ADBMS1818 CRC-15 computation
 * *************************************************************************/
 uint16_t pec15_reg (uint8_t *pdata , int len);
/*	@brief	: Reset and compute CRC
 *  @param  : pdata = pointer to input bytes
 *  @param  : len = number of bytes
 *  @return : CRC-15 * 2 (ADBMS1818 16b format)
 * *************************************************************************/

 #endif