/******************************************************************************
* File Name          : bqcellbal.h
* Date First Issued  : 09/23/2021
* Description        : BQ76952: cell balancing
*******************************************************************************/
#ifndef __BQCELLBAL
#define __BQCELLBAL

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "SerialTaskSend.h"

/* *************************************************************************/
 void bqcellbal_data (void);
/*	@brief	: compute average cell voltage and deviations around average
 * *************************************************************************/

extern int32_t cellave;        // Average of readings
extern int16_t celldev[16];	// deviation around average
extern int16_t celldevabs[16]; // devation absolute
extern int16_t cellmax;        // Max (signed) deviation
extern int16_t cellmin;        // Min (signed) deviation
extern int16_t cellabs;		// Max absolute deviation
extern uint8_t cellmaxidx;		// Index for cellmax
extern uint8_t cellminidx;     // Index for cellmin
extern uint8_t cellabsidx;     // Index for cell absolute

#endif

