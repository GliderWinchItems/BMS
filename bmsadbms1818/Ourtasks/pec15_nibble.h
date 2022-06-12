/******************************************************************************
* File Name          : pec15_nibble.h
* Date First Issued  : 06/11/2022
* Description        : ADBMS1818 PEC computation: 16 1/2 word table lookup
*******************************************************************************/
#include <stdint.h>

#ifndef __PEC15_NIBBLE
#define __PEC15_NIBBLE

uint16_t pec15_nibble(uint8_t *data, int size);

#endif
