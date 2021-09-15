/******************************************************************************
* File Name          : GevcuUpdates.h
* Date First Issued  : 07/02/2019
* Description        : Update outputs in Gevcu function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __GEVCUUPDATES
#define __GEVCUUPDATES

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "adc_idx_v_struct.h"
#include "GevcuTask.h"


/* *************************************************************************/
void GevcuUpdates(void);
/* @brief	: Update outputs based on bits set
 * *************************************************************************/
#endif

