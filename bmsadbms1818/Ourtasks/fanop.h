/******************************************************************************
* File Name          : fanop.h
* Date First Issued  : 06/22/2022
* Description        : 12v Fan operation (call from 'main')
*******************************************************************************/


#ifndef __FANOP
#define __FANOP

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

/* *************************************************************************/
void fanop_init(void);
/*	@brief	: Init fan operation
 * *************************************************************************/
 void fanop(void);
/*	@brief	: Update 
 * *************************************************************************/

extern uint8_t fanspeed; // Fan speed: rpm pct 0 - 100
extern uint8_t fantim2ready;

extern TIM_TypeDef  *pT2base; // Register base address 

#endif