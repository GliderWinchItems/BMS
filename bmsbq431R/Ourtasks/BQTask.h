/******************************************************************************
* File Name          : BQTask.h
* Date First Issued  : 09/08/2021
* Description        : BQ76952 BMS w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __BQTASK
#define __BQTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

#define BQVSIZE 20 // Readout loop size (16 cells plus others)


/* Cell current voltage for last measurement. */
struct VI
{
	uint32_t v;
	uint32_t i;
};
union ADCCELLCOUNTS
{
	struct VI vi[16];
// [x][0] = current, [x][1] = voltage, x = cell
	uint32_t blk[16][2]; // Cells 1-16
};

struct BQPN
{
	uint8_t* p;
	uint8_t  n;
};

struct BQFUNCTION
{
	uint32_t x;

};

/* *************************************************************************/
TaskHandle_t xBQTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BQTaskHandle
 * *************************************************************************/

 extern TaskHandle_t BQTaskHandle;
 extern struct BQFUNCTION bqfunction;

 extern union ADCCELLCOUNTS cellcts;

#endif

