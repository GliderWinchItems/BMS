/******************************************************************************
* File Name          : BrakeTask.h
* Date First Issued  : 10/02/2020
* Description        : Brake function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __BRAKEASK
#define __BRAKETASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "CanTask.h"

struct BRAKELC
{
	float K;  // I assume there will be some constants
};


struct BRAKEFUNCTION
{
   struct BRAKELC lc;    // Parameters
   uint32_t something;   // Working values

};

/* *************************************************************************/
 osThreadId xBrakeTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BrakeTaskHandle
 * *************************************************************************/

 extern osThreadId BrakeTaskHandle;
 extern struct BRAKEFUNCTION brakefunction;

#endif

