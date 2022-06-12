/******************************************************************************
* File Name          : pec15_reg.c
* Date First Issued  : 06/11/2022
* Description        : ADBMS1818 PEC computation: non-HAL register direct
*******************************************************************************/
#include "pec15_reg.h"

/* *************************************************************************
 * uint16_t pec15_reg_init (void);
 *  @brief  : Iniitalize RCC and CRCregisters for ADBMS1818 CRC-15 computation
 * *************************************************************************/
#define CRCBASE ((__IO uint32_t*)0x40023000)
#define SEED 0x10  // PEC15 initial value
void pec15_reg_init (void)
{
 __IO uint32_t* rccbase = (uint32_t*)0x40021000;

  /* Bit 12 CRCEN: CRC clock enable */
  *(rccbase+0x12) |= 0x1000; // Set CEN bit

  /* Set CRC registers. */
  *(uint32_t*)(CRCBASE+4) = SEED*2; // CRC_INT: 
  *(uint32_t*)(CRCBASE+5) = 0x8B32; // CRC_POL: Polynomial * 2

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
  *(CRCBASE+2) = 0x9; // CRC_CR: 16b + reset

  uint8_t* pend = pdata + len;
  do
  {
     *(__IO uint8_t*)CRCBASE = *pdata++;
  } while (pdata < pend);

  return *CRCBASE;
}
