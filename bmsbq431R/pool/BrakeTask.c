/******************************************************************************
* File Name          : BrakeTask.c
* Date First Issued  : 10/02/2020
* Description        : Brake function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"

#include "levelwind_items.h"
#include "drum_items.h"
#include "BrakeTask.h"

extern TIM_HandleTypeDef htim13;


/* Struct with many things for the drum function. */
struct BRAKEFUNCTION brakefunction;

osThreadId BrakeTaskHandle;

/* *************************************************************************
 * static void brake_init(struct BRAKEFUNCTION* p);
 *	@brief	: Some initialization before endless loop starts
 * *************************************************************************/
static void brake_init(void)
{
	HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);  
	return;
}
/* *************************************************************************
 * void StartBrakeTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartBrakeTask(void const * argument)
{
	brake_init();

	for (;;)
	{
		osDelay(10);
	}
}
/* *************************************************************************
 * osThreadId xBrakeTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BrakeTaskHandle
 * *************************************************************************/
osThreadId xBrakeTaskCreate(uint32_t taskpriority)
{
 	osThreadDef(BrakeTask, StartBrakeTask, osPriorityNormal, 0, (192));
	BrakeTaskHandle = osThreadCreate(osThread(BrakeTask), NULL);
	vTaskPrioritySet( BrakeTaskHandle, taskpriority );
	return BrakeTaskHandle;
}