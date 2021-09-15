/******************************************************************************
* File Name          : fetonoff.c
* Date First Issued  : 09/05/2021
* Description        : FET on/off control
*******************************************************************************/
#include <stdint.h>
#include "DTW_counter.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "fetonoff.h"
#include "morse.h"

/* Status of FETs in the bits of the byte. */
static uint8_t fetonoffstatus = 0; 

/* *************************************************************************
 * uint8_t fetonoff(uint8_t fetnum, unit8_t fetcommand);
 * @brief	: Set i/o bits to turn fet on or off
 * @param	: fetnum = designate FET
 * @param	: fetcommand: 1 = on; not 1 = off
 * @return  : byte with bits set/reset for each FET 
 * *************************************************************************/
uint8_t fetonoff(uint8_t fetnum, uint8_t fetcommand)
{
	if (fetcommand == FETON_SETON)
	{ /* Set I/O pins to turn FET ON. */
		switch (fetnum)
		{
		case FETON_DUMP:   // DUMP = Battery module discharge "dump"
			HAL_GPIO_WritePin(DUMP_NOT_GPIO_Port,DUMP_NOT_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DUMP_GPIO_Port,DUMP_Pin, GPIO_PIN_SET);
			fetonoffstatus |= FETON_DUMP_STATUS;
			break;

		case FETON_DUMP2:  // DUMP2 = Spare for relays, etc.
			HAL_GPIO_WritePin(DUMP2_GPIO_Port,DUMP2_Pin, GPIO_PIN_SET);
			fetonoffstatus |= FETON_DUMP2_STATUS;
			break;

		case FETON_HEATER: // Battery module warmup heater
			HAL_GPIO_WritePin(HEATER_NOT_GPIO_Port, HEATER_NOT_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(HEATER_GPIO_Port,HEATER_Pin, GPIO_PIN_SET);
			fetonoffstatus |= FETON_HEATER_STATUS;
			break;

		default: // Bogus FET designation
			morse_trap(661);
			break;
		}
	}
	else
	{ /* Set I/O pins to turn FET OFF. */
		switch (fetnum)
		{
		case FETON_DUMP:   // Battery module discharge "dump"
			HAL_GPIO_WritePin(DUMP_NOT_GPIO_Port,DUMP_NOT_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(DUMP_GPIO_Port,DUMP_Pin, GPIO_PIN_RESET);
			fetonoffstatus &= ~FETON_DUMP_STATUS;
			break;

		case FETON_DUMP2:  // DUMP2 = Spare for relays, etc.
			HAL_GPIO_WritePin(DUMP2_GPIO_Port,DUMP2_Pin, GPIO_PIN_RESET);
			fetonoffstatus &= ~FETON_DUMP2_STATUS;
			break;

		case FETON_HEATER: // Battery module warmup heater
			HAL_GPIO_WritePin(HEATER_NOT_GPIO_Port, HEATER_NOT_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(HEATER_GPIO_Port,HEATER_Pin, GPIO_PIN_RESET);
			fetonoffstatus &= ~FETON_HEATER_STATUS;
			break;

		default: // Bogus FET bit designation
			morse_trap(662);
			break;
		}
	}
	return fetonoffstatus;
}
/* *************************************************************************
 * uint8_t fetonoff_status(void);
 * @brief	: Return status
 * @return  : byte with bits set/reset for each FET 
 * *************************************************************************/
uint8_t fetonoff_status(void)
{
	return fetonoffstatus;
}
