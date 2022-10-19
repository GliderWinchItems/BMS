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
#include "BQTask.h"

/* *************************************************************************
 * uint8_t fetonoff(uint8_t fetnum, unit8_t fetcommand);
 * @brief	: Set i/o bits to turn fet on or off
 * @param	: fetnum = designate FET
 * @param	: fetcommand: 1 = on; not 1 = off
 * @return  : byte with bits set/reset for each FET 
 * *************************************************************************/
uint8_t fetonoff(uint8_t fetnum, uint8_t fetcommand)
{
	struct BQFUNCTION* pbq = &bqfunction;

	if (fetcommand == FET_SETOFF)
	{ /* Set I/O pins to turn FET OFF. */
		switch (fetnum)
		{
		case FET_DUMP:   // DUMP = Battery module discharge "dump"
			HAL_GPIO_WritePin(DUMP_NOT_GPIO_Port,DUMP_NOT_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DUMP_GPIO_Port,DUMP_Pin, GPIO_PIN_SET);
			pbq->fet_status |= FET_DUMP;
			break;

		case FET_DUMP2:  // DUMP2 = Spare for relays, etc.
			HAL_GPIO_WritePin(DUMP2_GPIO_Port,DUMP2_Pin, GPIO_PIN_RESET);
			pbq->fet_status |= FET_DUMP2;
			break;

		case FET_HEATER: // Battery module warmup heater
			HAL_GPIO_WritePin(HEATER_NOT_GPIO_Port, HEATER_NOT_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(HEATER_GPIO_Port,HEATER_Pin, GPIO_PIN_SET);
			pbq->fet_status |= FET_HEATER;

		 case FET_CHGR:
		 	TIM1->CCR1 = 0; // FET ON time
			pbq->fet_status &= ~FET_CHGR;
			break;

		default: // Bogus FET designation
			morse_trap(661);
			break;
		}
	}
	else
	{ /* Set I/O pins to turn FET ON. */
		switch (fetnum)
		{
		case FET_DUMP:   // Battery module discharge "dump"
			HAL_GPIO_WritePin(DUMP_NOT_GPIO_Port,DUMP_NOT_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(DUMP_GPIO_Port,DUMP_Pin, GPIO_PIN_RESET);
			pbq->fet_status &= ~FET_DUMP;
			break;

		case FET_DUMP2:  // DUMP2 = Spare for relays, etc.
			HAL_GPIO_WritePin(DUMP2_GPIO_Port,DUMP2_Pin, GPIO_PIN_SET);
			pbq->fet_status &= ~FET_DUMP2;
			break;

		case FET_HEATER: // Battery module warmup heater
			HAL_GPIO_WritePin(HEATER_NOT_GPIO_Port, HEATER_NOT_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(HEATER_GPIO_Port,HEATER_Pin, GPIO_PIN_RESET);
			pbq->fet_status &= ~FET_HEATER;
			break;

		 case FET_CHGR:
		 	TIM1->CCR1 = pbq->tim1_ccr1; // FET ON time
			pbq->fet_status |= FET_CHGR;
			break;

		default: // Bogus FET bit designation
			morse_trap(662);
			break;
		}
	}
	return pbq->fet_status;
}
/* *************************************************************************
 * void fetonoff_status_set(uint8_t status);
 * @brief	: Set FETs according to status byte (see BQTask.h)
 * @param   : status = status bits
 * *************************************************************************/
void fetonoff_status_set(uint8_t status)
{
//status = 0x00;// Testing External FETs on/off
	if ((status & FET_DUMP)    == 0)  // External charger
		fetonoff( FET_DUMP,   FET_SETOFF);
	else
		fetonoff( FET_DUMP,   FET_SETON);

	if ((status & FET_DUMP2)   == 0)   // Module discharge
		fetonoff( FET_DUMP2,  FET_SETOFF);
	else
		fetonoff( FET_DUMP2,  FET_SETON); 

	if ((status & FET_HEATER) == 0) // Module heater
		fetonoff( FET_HEATER, FET_SETOFF);
	else
		fetonoff( FET_HEATER, FET_SETON);

	if ((status & FET_CHGR)   == 0) // Charger switching FET
		fetonoff( FET_CHGR,  FET_SETOFF);
	else
		fetonoff( FET_CHGR,  FET_SETON);

	if ((status & (FET_DUMP2 | FET_CHGR)) != 0)
		bqfunction.battery_status |= BSTATUS_CHARGING;
	else
		bqfunction.battery_status &= ~BSTATUS_CHARGING;


	return;
}
