/******************************************************************************
* File Name          : bms_items.h
* Date First Issued  : 07/14/2022
* Description        : routines associated BMSTask and bmsspi
*******************************************************************************/

#ifndef __BMS_ITEMS
#define __BMS_ITEMS

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "BQTask.h"
#include "BMSTask.h"
#include "bq_idx_v_struct.h"
#include "bmsspi.h"

/* *************************************************************************/
void bms_items_extract_statreg(void);
/* @brief	: Extract & calibrate SC, ITMP, VA
 * *************************************************************************/
void bms_items_cfgset_overunder(void);
/* @brief	: Configuration set: Compute and set over and under voltage comparisons
 * *************************************************************************/
void bms_items_cfgset_dischargebits(uint32_t b);
/* @brief	: Configuration set: Update discharge bits in configreg (in memory)
 * @param   : b = bits for 18 cells (right justified): 0 = OFF, 1 = ON 
 * *************************************************************************/
void bms_items_cfgset_misc(void);
/* @brief	: Configuration set: 
 * @param   : b = bits for 18 cells (right justified): 0 = OFF, 1 = ON 
 * *************************************************************************/
void bms_items_cfg_int(void);
/* @brief	: Configuration register initialize
 * *************************************************************************/
void bms_items_extract_configreg(void);
/* @brief	: Extract current configreg settings
 * *************************************************************************/
void bms_items_therm_temps(void);
/* @brief	: Convert to temperature the latest thermistor voltages
 * *************************************************************************/
void bms_items_current_sense(void);
/* @brief	:  Compute a calibrated current from AUX GPIO reading
 * *************************************************************************/

#endif

