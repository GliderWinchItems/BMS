/******************************************************************************
* File Name          : ChgrTask.c
* Date First Issued  : 09/24/2021
* Description        : BMS Charger Task
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"
#include "ChgrTask.h"
#include "BQTask.h"
#include "fetonoff.h"
#include "bq_idx_v_struct.h"

extern DAC_HandleTypeDef hdac1;
extern OPAMP_HandleTypeDef hopamp1;
extern TIM_HandleTypeDef htim1;
extern COMP_HandleTypeDef hcomp1;
extern COMP_HandleTypeDef hcomp2;

TaskHandle_t ChgrTaskHandle = NULL;
/* *************************************************************************
 * void StartChgrTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartChgrTask(void* argument)
{
//while(1==1) osDelay(100);
//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET); // GRN LED

	struct BQFUNCTION* p = &bqfunction; // convenience pointer

	HAL_StatusTypeDef ret;

	/* DAC setup. */
 	// Set HV level to cut off TIM1
	ret = HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, p->lc.dac1_hv_setting);
	if (ret != HAL_OK) morse_trap(701);
	// Set Current sense level to cut off TIM1
	ret = HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, p->lc.dac2_ix_setting); 
	if (ret != HAL_OK) morse_trap(702);

	HAL_DAC_Start(&hdac1,DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);

	/* Op Amp RC filtered current sense (PA0) with gain to PA3 */
	ret = HAL_OPAMP_Start(&hopamp1);
	if (ret != HAL_OK) morse_trap(703);

	/* TIM1 Setup */
	ret = HAL_TIM_PWM_Init(&htim1);	
	if (ret != HAL_OK) morse_trap(704);
	ret = HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	if (ret != HAL_OK) morse_trap(705);

	/* Comparators. */
	ret = HAL_COMP_Start(&hcomp1);
	if (ret != HAL_OK) morse_trap(706);
	ret = HAL_COMP_Start(&hcomp2);
	if (ret != HAL_OK) morse_trap(707);

	/* Working value for FET ON duration in PWM frame */
	bqfunction.tim1_ccr1 = bqfunction.lc.tim1_ccr1_on; 

	for (;;)
	{
		/* Wink green led */
//		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET); // GRN LED
//		osDelay(15); 	
//		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET); // GRN LED
		osDelay(1985); 	
		
#ifdef USECHARGERUPDATEINCHRGTASK
		/* Internal charger control. */
		if ((p->fet_status & FET_CHGR) != 0)
		{ // Here, set charging 
			TIM1->CCR1 = bqfunction.tim1_ccr1; // Set charge rate
		}
		else
		{ // If not normal rate, should it be Very Low Charge rate?
			if ((p->fet_status & FET_CHGR_VLC) != 0)
			{ // Here, yes.
				TIM1->CCR1 = bqfunction.lc.tim1_ccr1_on_vlc; // Set vlc rate
			}
			else
			{ // Here, stop charging
				TIM1->CCR1 = 0;	// FET is off
			}
		}

		/* DUMP controls module discharging FET. */
		if ((p->fet_status & FET_DUMP) != 0)
		{ // Here, turn on FET for resistor discharge
			fetonoff(FET_DUMP,  FET_SETON);
		}
		else
		{ // Here, turn off FET for resistor discharge
			fetonoff(FET_DUMP,  FET_SETOFF);
		}

		/* DUMP2 controls external charger. */
		if ((p->fet_status & FET_DUMP2) != 0)
		{ // Here, turn on external module (or string?) charger
			fetonoff(FET_DUMP2,  FET_SETON);
		}
		else
		{ // Here, turn it off.
			fetonoff(FET_DUMP2,  FET_SETOFF);
		}
#endif
	} 	
}

/* *************************************************************************
 * TaskHandle_t xChgrTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ChgrTaskHandle
 * *************************************************************************/
TaskHandle_t xChgrTaskCreate(uint32_t taskpriority)
{

/*
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
const char * const pcName,
unsigned short usStackDepth,
void *pvParameters,
UBaseType_t uxPriority,
TaskHandle_t *pxCreatedTask );
*/
	BaseType_t ret = xTaskCreate(StartChgrTask, "ChgrTask",\
     (128), NULL, taskpriority,\
     &ChgrTaskHandle);
	if (ret != pdPASS) return NULL;

	return ChgrTaskHandle;
}
