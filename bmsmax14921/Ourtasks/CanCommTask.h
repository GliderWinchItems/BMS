/******************************************************************************
* File Name          : CanCommTask.h
* Date First Issued  : 11/06/2021
* Description        : Can communications
*******************************************************************************/
#ifndef __CANCOMMTASK
#define __CANCOMMTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "cancomm_items.h"
#include "BQTask.h"

/* *************************************************************************/
 void CanComm_init(struct BQFUNCTION* p );
/*	@brief	: Task startup
 * *************************************************************************/
 TaskHandle_t xCanCommCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: CanCommHandle
 * *************************************************************************/

extern TaskHandle_t CanCommHandle;
extern uint8_t rdyflag_cancomm;  // Initialization complete and ready = 1

#endif