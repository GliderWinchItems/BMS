/******************************************************************************
* File Name          : LedTask.h
* Date First Issued  : 10/02/2021
* Description        : LED blinking Task
*******************************************************************************/


#ifndef __LEDTASK
#define __LEDTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

enum ledtaskcode
{
	BOTH_OFF,
	GRN_OFF,
	RED_OFF,
	GRN_ON,
	RED_ON,
	GRN_WINK,
	RED_WINK,
	GRN_WINK_FAST,
	RED_WINK_FAST,
	BOTH_ALT
};


/* LED Task Queue Block */
struct LEDTASKQB 
{
	uint32_t code;       // Type of blink
};


/* *************************************************************************/
 TaskHandle_t xLedTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: LedTaskHandle
 * *************************************************************************/

extern TaskHandle_t LedTaskHandle;
extern osMessageQId LedTaskQHandle;

#endif