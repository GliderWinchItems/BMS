/******************************************************************************
* File Name          : chgr_items.h
* Date First Issued  : 02/11/2022
* Description        : routines associated with charging
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"
#include "LedTask.h"
#include "chgr_items.h"
#include "BQTask.h"

#include "fetonoff.h"
#include "bq_idx_v_struct.h"

extern DAC_HandleTypeDef hdac1;
extern OPAMP_HandleTypeDef hopamp1;
extern TIM_HandleTypeDef htim1;
extern COMP_HandleTypeDef hcomp1;
extern COMP_HandleTypeDef hcomp2;

/* *************************************************************************
 * void chgr_items_init(void);
 *	@brief	: Initializations for charger
 * *************************************************************************/
void chgr_items_init(void)
{
	HAL_StatusTypeDef ret;

	/* DAC setup. */
 	// Set HV level to cut off TIM1
	ret = HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, bqfunction.lc.dac1_hv_setting);
	if (ret != HAL_OK) morse_trap(701);
	// Set Current sense level to cut off TIM1
	ret = HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, bqfunction.lc.dac2_ix_setting); 
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

//	TIM1->CCR1 = 0; // Initialize to OFF
TIM1->CCR1 = bqfunction.tim1_ccr1;
	return;
}