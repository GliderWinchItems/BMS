/******************************************************************************
* File Name          : cellbal.h
* Date First Issued  : 03/20/2022
* Description        : MAX14921: cell balancing
*******************************************************************************/
/* 

*/
#ifndef __CELLBAL
#define __CELLBAL

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "ADCTask.h"

/* *************************************************************************/
 void cellbal_do(struct ADCREADREQ* parq);
/* @brief	: Check cell voltages and set 
 *          : -  I/O pins for DUMP resistor FET ON/OFF
 *          : -  I/O pin for DUMP2 FET (External charger) ON/OFF
 *          : -  Discharge FETs
 *          : -  Trickle charger OFF, low rate, hight rate setting
 * @brief   : parq = pointer to INITIALIZED bms read request struct
 * *************************************************************************/

 #endif
