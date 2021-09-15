/******************************************************************************
* File Name          : DrumTask.c
* Date First Issued  : 09/15/2020
* Description        : Drum function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"

#include "levelwind_items.h"
#include "drum_items.h"
#include "DrumTask.h"

/* Struct with many things for the drum function. */
struct DRUMFUNCTION drumfunction;

osThreadId DrumTaskHandle;

/* *************************************************************************
 * void StartDrumTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartDrumTask(void const * argument)
{
	struct DRUMFUNCTION* p = &drumfunction;

	drum_items_init(p);

	for (;;)
	{
		osDelay(10);
	}
}
/* *************************************************************************
 * osThreadId xDrumTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: DrumTaskHandle
 * *************************************************************************/
osThreadId xDrumTaskCreate(uint32_t taskpriority)
{
 	osThreadDef(DrumTask, StartDrumTask, osPriorityNormal, 0, (192));
	DrumTaskHandle = osThreadCreate(osThread(DrumTask), NULL);
	vTaskPrioritySet( DrumTaskHandle, taskpriority );
	return DrumTaskHandle;
}