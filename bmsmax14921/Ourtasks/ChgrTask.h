/******************************************************************************
* File Name          : ChgrTask.h
* Date First Issued  : 09/24/2021
* Description        : BMS Charger Task
*******************************************************************************/


#ifndef __CHGRTASK
#define __CHGRTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

/* *************************************************************************/
 TaskHandle_t xChgrTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ChgrTaskHandle
 * *************************************************************************/

extern TaskHandle_t ChgrTaskHandle;

#endif