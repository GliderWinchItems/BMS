/******************************************************************************
* File Name          : adcparams.c
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : Parameters for ADC app configuration
*******************************************************************************/
/*
Not thread safe.
*/
#include "adcparams.h"
#include "adcparamsinit.h"
#include "ADCTask.h"
#include "morse.h"
#include "iir_f1.h"
#include "BQTask.h"

#include "DTW_counter.h"

/* Map ADC reading sequence index to Calibration type index 
   Given: ADC seq index, lookup calibration type array index. */
const int8_t adcmapabs[4]   = {  0,  1,  2, -1}; // Absolute
const int8_t adcmapratio[4] = { -1, -1, -1,  0}; // Ratiometric


/*
AN3964
https://www.st.com/resource/en/application_note/dm00035957.pdf
V DDA = 3 × Val_V REFINT_CAL ⁄ Val_V REFINT

Temp = 80 ⁄ ( TS_CAL2 – TS_CAL1 ) × ( ValTS – TS_CAL1 ) + 30

The accurate embedded internal reference voltage (V REFINT ) is individually sampled by the
ADC, and the converted value for each device (Val_V REFINT_CAL ) is stored during the
manufacturing process in the protected memory area at address VREFINT_CAL specified
in the product datasheet. The internal reference voltage calibration data is a 12-bit unsigned
number (right-aligned bits, stored in 2 bytes) acquired by the STM32L1x ADC referenced to
V VREF_MEAS = V REF+ = 3V ± 0.01V
The total accuracy of the factory measured calibration data is then provided with an
accuracy of ± 5 mV (refer to the datasheet for more details).
We can determine the actual V DDA voltage by using the formula above as follows:
V DDA = 3 × Val_V REFINT_CAL ⁄ Val_V REFINT
The temperature sensor data, ValTS_bat, are sampled with the ADC scale referenced to the
actual V DDA value determined at the previous steps. Since the temperature sensor factory
calibration data are acquired with the ADC scale set to 3 V, we need to normalize ValTS_bat
to get the temperature sensor data (ValTS) as it would be acquired with ADC scale set to
3 V. ValTS_bat can be normalized by using the formula below:
ValTS = 3 × ValTS_bat ⁄ V DDA
If the ADC is referenced to the 3 V power supply (which is the case of the STM32L1
Discovery) such a normalization is not needed and the sampled temperature data can be
directly used to determine the temperature as described in Section 2.2.1: Temperature
sensor calibration.


Vdd = 3300*(*VREFINT_CAL_ADDR)/ADC_raw;

Temp(degree) = (V_sense - V_25)/Avg_slope + 25

*/

/* Calibration values common to all ADC modules. */
struct ADCCALCOMMON adcommon;

/* Everything for ADC1. */
struct ADCFUNCTION adc1;

/* *************************************************************************
 * void adcparams_init(void);
 *	@brief	: Copy parameters into structs
 * NOTE: => ASSUMES ADC1 ONLY <==
 * *************************************************************************/
void adcparams_init(void)
{
	/* Load parameters, either hard coded, (or later implement from high flash). */
	adc_idx_v_struct_hardcode_params(&adc1.lc);

	/* Init working struct for ADC function. */
	adcparamsinit_init(&adc1);

	return;
}

/* *************************************************************************
 * float adcparams_calibbms(uint16_t x, uint8_t i);
 *	@brief	: calibrate 
 *  @param  : x = raw ADC count.
 *  @param  : i = index of reading (0 - 19) 20 readings
 *  @return : calibrated value (float volts)
 * *************************************************************************/
float adcparams_calibbms(uint16_t a, uint8_t i)
{
	float x = a; // Convert int to float
	float y = (x*x*adc1.lc.cabsbms[i].coef[2] +
			     x*adc1.lc.cabsbms[i].coef[1] +
			       adc1.lc.cabsbms[i].coef[0] );
	return y;
}

/* *************************************************************************
 * void adcparams_calibadc(void);
 *	@brief	: calibrate and filter ADC channel readings (from ADCTask.c)
 * *************************************************************************/
void adcparams_calibadc(void)
{
	struct ADCFUNCTION* p = &adc1; // Convenience pointer
	struct ADCABS* pabs = &p->abs[0]; // Absolute readings
	struct ADCABS* pabs_end = &p->abs[ADCDIRECTMAX]; // Absolute readings
	struct ADCCALABS* pcabsadc = &p->lc.cabsadc[0]; // Absolute readings [2]

	/* First update temperature compensated internal reference readings
	   Internal voltage reference and temperature are the first two in
	   the scan seqeunce. */

	/* Calibrate Vref and filter */
	pabs->f = (float)(pabs->sum + pcabsadc->ioffset) * pcabsadc->scale;
	pabs->filt = iir_f1_f(&pcabsadc->iir_f1,pabs->f);

	/* Calibrate internal temperature reading and filter. */
	(pabs+1)->f = (float)((pabs+1)->sum + (pcabsadc+1)->ioffset) * (pcabsadc+1)->scale;
	(pabs+1)->filt = iir_f1_f(&(pcabsadc+1)->iir_f1,(pabs+1)->f);

	/* Compute temperature */
	p->common.degC = p->common.ts_calrate * ((pabs+1)->filt - p->common.ts_cal1) + 30.0f;

	/* Vref temperature compensation TODO */
	p->common.fvrefajd = 1.0f; // Adjust calibration based on Vref for temperature

	(pabs+0)->sumsave = (pabs+0)->sum;
	(pabs+0)->sum = 0;

	(pabs+1)->sumsave = (pabs+1)->sum;
	(pabs+1)->sum = 0;

	/* Calibrate remainder of ADC channel readings */
	// Skip over Vref and temperature
	pabs     += 2;
	pcabsadc += 2;

	while (pabs != pabs_end)
	{
		/* Calibrate: apply offset, convert to float, scale, and filter. */
		pabs->f = (float)(pabs->sum + pcabsadc->ioffset) * pcabsadc->scale * p->common.fvrefajd;
		pabs->filt = iir_f1_f(&pcabsadc->iir_f1,pabs->f);

		pabs->sumsave = pabs->sum; // Save so other's can ruminate on the data
		pabs->sum = 0; // Yes!. We are ready for next round of accumulation.
		/* Point to next reading and calibration. */
		pabs += 1; 
		pcabsadc += 1;
	}
}	
