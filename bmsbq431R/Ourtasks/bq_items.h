/******************************************************************************
* File Name          : chgr_items.h
* Date First Issued  : 10/02/2021
* Description        : routines associated with charging task
*******************************************************************************/


#ifndef __BQ_ITEMS
#define __BQ_ITEMS

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "ChgrTask.h"
#include "BQTask.h"

/* *************************************************************************/
 void bq_items_seq(int16_t* p);
/* @brief	: Go thought a sequence of steps to determine balancing
 * @param   : p = pointer to 16 cell voltage array
 * *************************************************************************/
 void bq_items_qsortV(struct BQCELLV* p);
/* @brief	: Sort array by voltage
 * @param   : p = pointer to 16 cell struct array
 * *************************************************************************/

#endif