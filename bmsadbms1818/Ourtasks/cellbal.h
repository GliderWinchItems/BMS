/******************************************************************************
* File Name          : cellbal.h
* Date First Issued  : 006/26/2022
* Description        : ADBMS1818: cell balancing
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
#include "bmsdriver.h"

/* *************************************************************************/
 void cellbal_do(void);
/* @brief	: Check cell voltages and set 
 *          : -  I/O pins for DUMP resistor FET ON/OFF
 *          : -  I/O pin for DUMP2 FET (External charger) ON/OFF
 *          : -  Discharge FETs
 *          : -  Trickle charger OFF, low rate, hight rate setting
 * *************************************************************************/

 #endif
