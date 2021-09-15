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
 * static void internal(struct ADCFUNCTION* padc1);
 *	@brief	: Update values used for compensation from Vref and Temperature
 * @param	: padc1 = Pointer to array of ADC reading sums plus other stuff
 * *************************************************************************/
uint32_t adcdbg1;
uint32_t adcdbg2;
static void internal(struct ADCFUNCTION* padc1)
{
/* 
   REPRODUCED from 'adcparamsinit.h' for convenience.
#define PVREFINT_CAL ((uint16_t*)0x1FFF7A2A))  // Pointer to factory calibration: Vref
#define PTS_CAL1     ((uint16_t*)0x1FFF7A2C))  // Pointer to factory calibration: Vtemp
#define PTS_CAL2     ((uint16_t*)0x1FFF7A2E))  // Pointer to factory calibration: Vtemp
#define VREFCALVOLT 3000  // Factory cal voltage (mv)
*/

adcdbg1 = DTWTIME;

struct ADCCALCOMMON* pacom = &padc1->common;

/* The following two computaions with ints uses 119 machines cycles. */
//	pacom->ivdd = pacom->fvref / (padc1->chan[ADC1IDX_INTERNALVREF].sum);

//	pacom->ui_tmp = (pacom->ivdd * padc1->chan[ADC1IDX_INTERNALTEMP].sum ) / VREFCALVOLT; // Adjust for Vdd not at 3.0v calibration
//	pacom->degC  = pacom->ll_80caldiff * (pacom->ui_tmp - pacom->ui_cal1) + (30 * SCALE1 * ADC1DMANUMSEQ);
//	pacom->degC *= ((float)1.0/(SCALE1*ADC1DMANUMSEQ)); 
//	pacom->degCfilt = iir_f1_f(&padc1->chan[ADC1IDX_INTERNALTEMP].iir_f1, pacom->degC);

	pacom->fvdd = pacom->ivdd;
	pacom->fvdd = pacom->fvdd + pacom->tcoef * (pacom->degC - (float)30);

	pacom->fvddfilt = iir_f1_f(&padc1->chan[ADC1IDX_INTERNALVREF].iir_f1, pacom->fvdd);

//	pacom->fvddcomp = pacom->fvddfilt * pacom->sensor5vcalVdd; // Pre-compute for multple uses later

//	pacom->fvddrecip = (float)1.0/pacom->fvddfilt; // Pre-compute for multple uses later

	/* Scale up for fixed division, then convert to float and descale. */
//	pacom->f5_Vddratio = ( (padc1->chan[ADC1IDX_INTERNALVREF].sum * (float)(1<<12)) / padc1->chan[ADC1IDX_5VSUPPLY].sum);
//	pacom->f5_Vddratio *= ((float)1.0/(float)(1<<12));

	/* 5v supply voltage. */
//	pacom->f5vsupply = padc1->chan[ADC1IDX_5VSUPPLY].sum * pacom->fvddfilt * pacom->f5vsupplyprecal + pacom->f5vsupplyprecal_offset;
//	pacom->f5vsupplyfilt = iir_f1_f(&padc1->chan[ADC1IDX_5VSUPPLY].iir_f1, pacom->f5vsupply);

adcdbg2 = DTWTIME - adcdbg1;

	return;
}
/* *************************************************************************
 * static void absolute(struct ADCFUNCTION* p, uint8_t idx);
 *	@brief	: Calibrate and filter absolute voltage readings
 * @param	: p = Pointer to array of ADC reading sums plus other stuff
 * @param	: idx = index into ADC sum for sensor for reading 'n'
 * *************************************************************************/
/*
Vn = Vref * (ADC[n]/ADC[vref]) * ((R1+R2)/R2);
  Where: ((R1+R2)/R2) is resistor divider scale factor 
*/
#ifdef USE_ABSOLUTE
static void absolute(struct ADCFUNCTION* p, uint8_t idx)
{
	if (adcmapabs[idx] < 0) morse_trap(57); // Illegal index: Coding problem
	struct ADCABSOLUTE* pa = &p->abs[adcmapabs[idx]];

	/* IIR filter adc reading. */
	pa->adcfil = iir_filter_lx_do(&pa->iir, &p->chan[idx].sum);

	pa->ival = ((1<<ADCSCALEbits) * pa->adcfil) / p->common.adccmpvref;
	return;
}
#endif
/* *************************************************************************
 * static void ratiometric5v(struct ADCFUNCTION* p, uint8_t idx);
 *	@brief	: Calibrate and filter 5v ratiometric (e.g. Hall-effect sensor) reading
 * @param	: p = Pointer
 * @param	: idx = index into ADC sum for sensor
 * *************************************************************************/
#ifdef FIVEVSUPPLYMEASUREMENT
uint32_t dbgadcfil;
uint32_t dbgadcratio;

void ratiometric5v(struct ADCFUNCTION* p, int8_t idx)
{
/* NOTE: Ratiometric is based on the ratio of the reading of the 5v supply 
   powering the sensor and the sensor reading.  The ratio is adjusted to 
   account for the differences in the resistor divider ratio for both
   inputs.

	The originating offset parameter in the 'lc struct is converted to a
   2^16 scaled fraction during the initialization of parameters.  Therefore,
   the offset is typically, either zero (sensor is positive going only), to
   0.5 (therefore 32767) when the offset is at 1/2 Vsensor (2.5v).
*/
	if (adcmapratio[idx] < 0) morse_trap(56); // Illegal index: Coding problem
	struct ADCRATIOMETRIC* pr = &p->ratio[adcmapratio[idx]];

	/* IIR filter adc reading. */
//$	pr->adcfil = iir_filter_lx_do(&pr->iir, &p->chan[idx].sum);

	/* Compute ratio of sensor reading to 5v supply reading. */
	uint16_t adcratio = (p->chan[idx].sum << ADCSCALEbits) / p->chan[ADC1IDX_5VSUPPLY].sum;

	/* Filter the ratio */
	pr->adcfil = iir_filter_lx_do(&pr->iir, &adcratio);

	/* Subtract offset (note result is now signed). */
	pr->iI = (pr->adcfil - pr->irko); 


dbgadcfil=pr->adcfil;
dbgadcratio=adcratio;

	return;
}
#endif

/* *************************************************************************
 * void adcparams_cal(void);
 *	@brief	: calibrate and filter ADC readings (from ADCTask.c)
 * *************************************************************************/
void adcparams_cal(void)
{
	struct ADCFUNCTION* p = &adc1; // Convenience pointer

	/* First: Update Vref used in subsequent computations. */
	internal(p);

//	absolute(p, ADC1IDX_STEPPERV);  // Stepper controller voltage
  
//	absolute(p, ADC1IDX_5VSUPPLY);  // 5v supply to sensors
  
//	absolute(p, ADC1IDX_SPARE);     // Spare input

/* Note: 5v supply should be processed before ratiometrics.  Otherwise,
   old readings will be used which is not a big deal for a slowly 
   changing 5v supply. */

//	ratiometric5v(p, ADC1IDX_SPARE); // Possible current sensor

	return;
}
