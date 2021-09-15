/******************************************************************************
* File Name          : GevcuStates.h
* Date First Issued  : 07/01/2019
* Description        : States in Gevcu function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __GEVCUSTATES
#define __GEVCUSTATES

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "adc_idx_v_struct.h"

void GevcuStates_GEVCU_INIT(void);
void GevcuStates_GEVCU_SAFE_TRANSITION(void);
void GevcuStates_GEVCU_SAFE(void);
void GevcuStates_GEVCU_ACTIVE_TRANSITION(void);
void GevcuStates_GEVCU_ACTIVE(void);
void GevcuStates_GEVCU_ARM_TRANSITION(void);
void GevcuStates_GEVCU_ARM(void);

#endif

