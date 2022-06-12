/******************************************************************************
* File Name          : pec15_reg.c
* Date First Issued  : 06/11/2022
* Description        : ADBMS1818 PEC computation: non-HAL register direct
*******************************************************************************/
#include "stm32l4xx_hal.h"

uint16_t pec15_reg_init (void)
{
	__IO uint32_t* crcbase = CRC;//(uint32_t*)0x40023000;
	__IO uint32_t* rccbase = (uint32_t*)0x40021000;

	/* Bit 12 CRCEN: CRC clock enable */
	*(rccbase+0x12) |= 0x1000; // Set CEN bit

	/* Set CRC registers. */
	*(uint32_t*)(crcbase+4) = SEED*2; // CRC_INT: 
	*(uint32_t*)(crcbase+5) = 0x8B32; // CRC_POL: Polynomial * 2

	retrun;
}

uint16_t pec15_reg (uint8_t *pdata , int len)
{
	uint32_t* pend = pdata + len;
	/* Control register configuration includes reset. */
	*(uint32_t*)(crcbase+2) = 0x9; // CRC_CR: 16b + reset
	do
	{
		*(__IO uint8_t *)(crcbase+0) = *pdata++;
	} while (pdata < pend);

	return *(crcbase+0);
}
