/******************************************************************************
* File Name          : bq_items.h
* Date First Issued  : 07/11/2022
* Description        : routines associated with charging & cell balance
*******************************************************************************/


#ifndef __BQ_ITEMS
#define __BQ_ITEMS

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "ChgrTask.h"
#include "BQTask.h"
#include "BMSTask.h"

#define BQITEMSNOTE00 (1<<0) // WaitTask Notification bit

/* Sort Cell voltages */
#define USESORTCODE

/* *************************************************************************/
 void bq_items_init(void);
/* @brief	: Cell balance 
 * *************************************************************************/
uint8_t bq_items(void);
/* @brief	: Cell balance 
 * @return  : 0 = did nothing; 1 = Did read and balance seq
 * *************************************************************************/
void bq_items_seq(void);
/* @brief	: Go thought a sequence of steps to determine balancing
 * *************************************************************************/
 void bq_items_qsortV(struct BQCELLV* p);
/* @brief	: Sort array by voltage
 * @param   : p = pointer to 16 cell struct array
 * *************************************************************************/

 extern struct BQFUNCTION bqfunction;

#endif