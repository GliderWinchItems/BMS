/******************************************************************************
* File Name          : GevcuStates.c
* Date First Issued  : 07/01/2019
* Description        : States in Gevcu function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "ADCTask.h"
#include "adctask.h"
#include "GevcuEvents.h"
#include "GevcuStates.h"
#include "GevcuTask.h"
#include "yprintf.h"
#include "gevcu_idx_v_struct.h"
#include "morse.h"
#include "adcparamsinit.h"
#include "stepper_items.h"

#define GEVCULCDMSGDELAY 32 // Minimum number of time ticks between LCD msgs
#define GEVCULCDMSGLONG (128*30) // Very long delay

extern struct LCDI2C_UNIT* punitd4x20; // Pointer LCDI2C 4x20 unit

enum GEVCU_INIT_SUBSTATEA
{
	GISA_OTO,  
	GISA_WAIT,
};


/* *************************************************************************
 * void payloadfloat(uint8_t *po, float f);
 *	@brief	: Convert float to bytes and load into payload
 * @param	: po = pointer to payload byte location to start (Little Endian)
 * *************************************************************************/
void payloadfloat(uint8_t *po, float f)
{
	union FF
	{
		float f;
		uint8_t ui[4];
	}ff;
	ff.f = f; 

	*(po + 0) = ff.ui[0];
	*(po + 1) = ff.ui[1];
	*(po + 2) = ff.ui[2];
	*(po + 3) = ff.ui[3];
	return;
}

/* *************************************************************************
 * void GevcuStates_GEVCU_INIT(void);
 * @brief	: Initialization sequence: One Time Only
 * *************************************************************************/

void GevcuStates_GEVCU_INIT(void)
{	
	switch (gevcufunction.substateA)
	{
	case GISA_OTO: // Cycle Safe/Active sw.



	case GISA_WAIT: // More OTO to do here?
		/* Transition into safe mode,. */
		gevcufunction.state = GEVCU_SAFE_TRANSITION;
		break;

		default:
			break;
	}
	return;
}
/* *************************************************************************
 * void GevcuStates_GEVCU_SAFE_TRANSITION(void);
 * @brief	: Peace and quiet, waiting for hapless Op.
 * *************************************************************************/

void GevcuStates_GEVCU_SAFE_TRANSITION(void)
{


	/* Assure disable stepper controller states. */
	HAL_GPIO_WritePin(EN_port,EN_pin, GPIO_PIN_RESET); // Disable Stepper motor

	/* Default stepper motor controller direction. */
	HAL_GPIO_WritePin(DR_port,DR_pin, GPIO_PIN_RESET); 



	gevcufunction.state = GEVCU_SAFE;
	return;
}
/* *************************************************************************
 * void GevcuStates_GEVCU_SAFE(void);
 * @brief	: Peace and quiet, waiting for hapless Op.
 * *************************************************************************/

void GevcuStates_GEVCU_SAFE(void)
{
		gevcufunction.state = GEVCU_ACTIVE_TRANSITION;
	return;
}
/* *************************************************************************
 * void GevcuStates_GEVCU_ACTIVE_TRANSITION(void);
 * @brief	: Contactor & DMOC are ready. Keep fingers to yourself.
 * *************************************************************************/

void GevcuStates_GEVCU_ACTIVE_TRANSITION(void)
{
	gevcufunction.state = GEVCU_ACTIVE;
	return;
}
/* *************************************************************************
 * void GevcuStates_GEVCU_ACTIVE(void);
 * @brief	: Contactor & DMOC are ready. Keep fingers to yourself.
 * *************************************************************************/
void GevcuStates_GEVCU_ACTIVE(void)
{
//	HAL_GPIO_WritePin(EN_port,EN_pin, GPIO_PIN_SET); // Enable Stepper motor

	gevcufunction.state = GEVCU_ARM_TRANSITION;
	return;
}
/* *************************************************************************
 * void GevcuStates_GEVCU_ARM_TRANSITION(void);
 * @brief	: Do everything needed to get into state
 * *************************************************************************/

void GevcuStates_GEVCU_ARM_TRANSITION(void)
{

		gevcufunction.state = GEVCU_ARM;
		return;
}
/* *************************************************************************
 * void GevcuStates_GEVCU_ARM(void);
 * @brief	: Contactor & DMOC are ready. Keep fingers to yourself.
 * *************************************************************************/
void GevcuStates_GEVCU_ARM(void)
{
	return;
}

