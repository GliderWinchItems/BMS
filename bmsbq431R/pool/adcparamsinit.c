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

static void abs_init(struct ADCFUNCTION* p, int8_t idx1);

/* *************************************************************************
 * void adcparamsinit_init(struct ADCFUNCTION* p); //struct ADCCALCOMMON* padccommon, struct ADCCHANNELSTUFF* pacsx);
 *	@brief	: Initialize struct with parameters common to all ADC for this =>board<=
 * @param	: padccommon = pointer to struct holding parameters
 * @param	: pacsx = Pointer to struct "everything" for this ADC module
 * *************************************************************************/
void adcparamsinit_init(struct ADCFUNCTION* p)
{
	/* Convenience pointers. */
	struct ADCCALCOMMON* padccommon = &p->common;

	/* Pointers to fixed pt iir filter contstants. */
	padccommon->iiradcvref.pprm = &p->lc.calintern.iiradcvref;
	padccommon->iiradctemp.pprm = &p->lc.calintern.iiradctemp;

	/* Reassign float pt iir filter constants. */
	p->chan[ADC1IDX_INTERNALTEMP].iir_f1.skipctr  = 8;    // Initial readings skip count
	p->chan[ADC1IDX_INTERNALTEMP].iir_f1.coef     = 0.99;  // Filter coefficient (< 1.0)
	p->chan[ADC1IDX_INTERNALTEMP].iir_f1.onemcoef = (1 - p->chan[ADC1IDX_INTERNALTEMP].iir_f1.coef); // Pre-computed

	p->chan[ADC1IDX_INTERNALVREF].iir_f1.skipctr  = 8;    // Initial readings skip count
	p->chan[ADC1IDX_INTERNALVREF].iir_f1.coef     = 0.99;  // Filter coefficient (< 1.0)
	p->chan[ADC1IDX_INTERNALVREF].iir_f1.onemcoef = (1 - p->chan[ADC1IDX_INTERNALVREF].iir_f1.coef); // Pre-computed

	padccommon->ts_vref      = *PVREFINT_CAL; // Factory calibration
	padccommon->tcoef        = 30E-6; // 30 typ, 50 max, (ppm/deg C)

	padccommon->ts_cal1      = (float)(*PTS_CAL1); // Factory calibration
	padccommon->ts_cal2      = (float)(*PTS_CAL2); // Factory calibration
	padccommon->ts_caldiff   = (padccommon->ts_cal2 - padccommon->ts_cal1); // Pre-compute
	padccommon->ts_calrate = ((float)(PTS_CAL2_TEMP - PTS_CAL1_TEMP) / (padccommon->ts_caldiff *(float)ADC1DMANUMSEQ)); // Pre-compute
	padccommon->tx_cal1dma   = padccommon->ts_cal1 * (float)ADC1DMANUMSEQ;

	/* Data sheet gave these values.  May not need them. */
	padccommon->v25     = 0.76; // Voltage at 25 °C, typ
	padccommon->slope   = 2.0;  // Average slope (mv/deg C), typ
	padccommon->offset  = 25.0;

	// Compute vref from measurements
	p->common.fadc  = p->lc.calintern.adcvdd; // ADC reading (~27360)
//	p->common.fvref = p->lc.calintern.fvdd * (p->common.fadc / 65520.0);
	p->common.fvref = p->lc.calintern.fvref;

	// Check for out-of-datasheet Vref spec 
	if ((p->common.fvref < (VREFMIN)) || (p->common.fvref > (VREFMAX))) 
	{
		morse_trap(81);
	}

		/* Initialize floating pt iir values for all. (JIC) */
	int i;
	for (i = 0; i < (ADC1IDX_ADCSCANSIZE + 2); i++)
	{ // Initialize all with default. ==> Others can change it later <==
		p->chan[i].iir_f1.onemcoef = (1 - p->chan[i].iir_f1.coef); // Pre-computed
	}


	return;
}
/* *************************************************************************
 * static void abs_init(struct ADCFUNCTION* p, int8_t idx1);
 *	@brief	: Load structs for compensation, calibration and filtering for ADC channels
 * @param	: p = Points to struct with "everything" for this ADC module
 * @param	: idx1 = index in ADC sequence array
 * *************************************************************************/
#ifdef USEABS_INIT
static void abs_init(struct ADCFUNCTION* p, int8_t idx1)
{
/* Reproduced for convenience
struct ADCABSOLUTE
{
	struct IIRFILTERL iir;// Intermediate filter params
	float fscale;        // Computed from measurements
	float k;             // divider ratio: (Vref/adcvref)*(adcvx/Vx)
	uint32_t adcfil;      // Filtered ADC reading
	uint32_t ival;        // scaled int computed value (not divider scaled)
}; */	

	/* Lookup index in absolute array, given ADC sequence index. */
//	int8_t idx2 = adcmapabs[idx1];
//	if (idx2 < 0) morse_trap(60);	// Illegal index: Coding error

	struct ADCABSOLUTE*    pabs = &p->abs[idx1]; // Working param
	struct ADCCALABS* plc  = &p->lc.cabs[idx1]; // Calibration param
	
	pabs->iir.pprm = &plc->iir_f1; // Filter param pointer
	pabs->k   = (plc->fvn / p->common.fvref) * (p->common.fadc / plc->adcvn);
	pabs->fscale = pabs->k * p->common.fvref;
	p->chan[idx1].fscale = pabs->k; // Save in channel array
	return;
}
#endif

