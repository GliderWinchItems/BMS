/******************************************************************************
* File Name          : FanTask.h
* Date First Issued  : 12/04/2021
* Description        : 12v Fan Task
*******************************************************************************/


#ifndef __FANTASK
#define __FANTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"


/* *************************************************************************/
 TaskHandle_t xFanTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: FanTaskHandle
 * *************************************************************************/
 void FanTask_TIM2_IRQHandler(void);
/* @brief	: TIM2 IRQ (see stm32l4xx_it.c)
 * *************************************************************************/

extern TaskHandle_t FanTaskHandle;
extern osMessageQId FanTaskQHandle;

extern uint8_t fanspeed; // Fan speed: rpm pct 0 - 100
extern uint8_t fantim2ready;

extern TIM_TypeDef  *pT2base; // Register base address 

#endif