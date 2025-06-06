/******************************************************************************
* File Name          : adcparamsinit.c
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : Initialization of parameters for ADC app configuration
*******************************************************************************/

/* 
This is where hard-coded parameters for the ADC are entered.

Later, this may be replaced with a "copy" of the flat file in high flash, generated
by the java program from the sql database.
*/

#include "adcparamsinit.h"
#include "adcparams.h"
#include "ADCTask.h"
#include "morse.h"

// Define limits for initialization check
#define VREFMIN (1.15)
#define VREFMAX (1.23)

/* *************************************************************************
 * void adcparamsinit_init(struct ADCFUNCTION* p);
 * @brief	: Initialize struct with parameters common to all ADC for this =>board<=
 * @param	: p = Pointer to struct "everything" for this ADC module
 * *************************************************************************/
void adcparamsinit_init(struct ADCFUNCTION* p)
{
	/* Convenience pointers. */
	struct ADCCALCOMMON* padccommon = &p->common;

	padccommon->ts_vref      = *PVREFINT_CAL; // Factory calibration
	padccommon->tcoef        = 30E-6; // 30 typ, 50 max, (ppm/deg C)

	padccommon->ts_cal1      = (float)(*PTS_CAL1); // Factory calibration
	padccommon->ts_cal2      = (float)(*PTS_CAL2); // Factory calibration
	padccommon->ts_caldiff   = (padccommon->ts_cal2 - padccommon->ts_cal1); // Pre-compute
	padccommon->ts_calrate = ((float)(PTS_CAL2_TEMP - PTS_CAL1_TEMP) / (padccommon->ts_caldiff * (float)ADCSCALEFACTOR)); // Pre-compute

	/* Data sheet gave these values.  May not need them. */
	padccommon->v25     = 0.76; // Voltage at 25 °C, typ
	padccommon->slope   = 2.0;  // Average slope (mv/deg C), typ
	padccommon->offset  = 25.0;

	// Compute vref from measurements
	p->common.fadc  = p->lc.calintern.adcvdd; // ADC reading (~27360)
	p->common.fvref = p->lc.calintern.fvref;

	// Check for out-of-datasheet Vref spec 
	if ((p->common.fvref < (VREFMIN)) || (p->common.fvref > (VREFMAX))) 
	{
		morse_trap(81);
	}

	return;
}
