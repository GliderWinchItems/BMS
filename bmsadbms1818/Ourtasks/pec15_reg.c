/******************************************************************************
* File Name          : pec15_reg.c
* Date First Issued  : 06/11/2022
* Description        : ADBMS1818 PEC computation: non-HAL register direct
*******************************************************************************/
/*
Instead of calling pec15_reg, the following line could be used for 
six byte command ped15 computation--

  Six byte PEC15 computation
    CRC->CR = 0x9; // 16b poly, + reset
    *(__IO uint32_t*)CRC_BASE = (uint32_t)__REV (*(uint32_t*)&data[0]);
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (*(uint16_t*)&data[4]);
    p15H = CDC->DR;  // Store 1/2 word result

   Six byte PEC15 computation
    CRC->CR = 0x9; // 16b poly, + reset
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (*(uint16_t*)&data[0]);
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (*(uint16_t*)&data[2]);
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (*(uint16_t*)&data[4]);
    p15E = CRC->DR; // Store 1/2 word result
*/
#include "pec15_reg.h"

/* *************************************************************************
 * void pec15_reg_init (void);
 *  @brief  : Iniitalize RCC and CRCregisters for ADBMS1818 CRC-15 computation
 * *************************************************************************/
#define SEED 0x10 // PEC15 initial value
void pec15_reg_init (void)
{
  /* Bit 12 CRCEN: CRC clock enable */
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;

  /* Set CRC registers. */
  CRC->INIT = SEED*2;
  CRC->POL = 0x8B32; // CRC_POL: 0x4599 Polynomial * 2

  return;
}

/* *************************************************************************
 * uint16_t pec15_reg (uint8_t *pdata , int len);
 *  @brief  : Reset and compute CRC
 *  @param  : pdata = pointer to input bytes
 *  @param  : len = number of bytes
 *  @return : CRC-15 * 2 (ADBMS1818 16b format)
 * *************************************************************************/
uint16_t pec15_reg (uint8_t *pdata , int len)
{
  /* Control register configuration includes reset. */
  //*(CRCBASE+2) = 0x9; // CRC_CR: 16b + reset
  CRC->CR = 0x9;

  uint8_t* pend = pdata + len;
  do
  {
     *(__IO uint8_t*)CRC_BASE = *pdata++;
  } while (pdata < pend);

  return CRC->DR;//*CRCBASE;
}

