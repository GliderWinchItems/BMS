/******************************************************************************
* File Name          : adcparamsinit.h
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : Initialization of parameters for ADC app configuration
*******************************************************************************/

#ifndef __ADCPARAMSINIT
#define __ADCPARAMSINIT

#include <stdint.h>
#include "adcparams.h"

//#define SCALE1 (1 << 16)

/* Factory calibration pointers. */
// Raw data acquired at a temperature of 30 °C (± 5 °C), V DDA = V REF+ = 3.0 V (± 10 mV)
#define PVREFINT_CAL ((uint16_t*)0x1FFF75AA)  // Pointer to factory calibration: Vref

// TS ADC raw data acquired at a temperature of 30 °C (± 5 °C), V DDA = V REF+ = 3.0 V (± 10 mV
#define PTS_CAL1     ((uint16_t*)0x1FFF75A8)  // Pointer to factory calibration: Vtemp
#define PTS_CAL1_TEMP 30

// TS ADC raw data acquired at a temperature of 130 °C (± 5 °C), V DDA = V REF+ = 3.0 V (± 10 mV)
#define PTS_CAL2     ((uint16_t*)0x1FFF75CA)  // Pointer to factory calibration: Vtemp
#define PTS_CAL2_TEMP 130


/* Factory Vdd for Vref calibration. */
#define VREFCALVOLT 3000  // Factory cal voltage (mv)
#define VREFCALVOLTF (VREFCALVOLT * 0.001)f  // Factory cal voltage, float (volts)

/* *************************************************************************/
void adcparamsinit_init(struct ADCFUNCTION* p);
/*	@brief	: Load structs for compensation, calibration and filtering for ADC channels
 * @param	: p = Points to struct with "everything" for this ADC module
 * *************************************************************************/
int16_t ratiometric_cal_zero(struct ADCFUNCTION* p, int16_t idx);
/*	@brief	: Adjust no-current ratio for a Hall-effect sensor
 * @param	: p = Pointer to struct "everything" for this ADC module
 * @param	: idx = ADC scan sequence index
 * @return	: 0 = no fault; -1 = out of tolerance
 * *************************************************************************/


#endif

