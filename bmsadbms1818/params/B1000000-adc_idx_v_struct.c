/******************************************************************************
* File Name          : B1000000-adc_idx_v_struct.c
* Date First Issued  : 03/06/2023
* Board              : bmsadbms1818
* Description        : Load ADC parameter struct: B1000000 ADBMS1818 board #4
*******************************************************************************/
/* 10/23/2020: Revised for Levelwind */
/* 03/01/2022: Revised for bmsmax14921 */

#include "adc_idx_v_struct.h"

/*
The radian frequency cutoff is Fs/K where Fs is the IIR filter rate. 
Ks is about 500 Hz so the radian bandwidth is 167 Sz or about 26.6 Hz. 
The time constant is the reciprocal of the radian bandwidth so about 6 ms. The 10-90 risetime would then be 13.2 ms. 
Since those outputs are at a 64 Hz rate (15.6 ms period), that seems like a pretty reasonable value for K.
*/

/* **************************************************************************************
 * int adc_idx_v_struct_hardcode_params(struct ADCGEVCULC* p);
 * @brief	: Hard-code load local copy with parameters
 * @return	: 0
 * ************************************************************************************** */
int adc_idx_v_struct_hardcode_params(struct ADCLC* p)
{
	p->calintern.iiradcvref.k     = 20;    // Filter time constant
	p->calintern.iiradcvref.scale = 64;

	p->calintern.iiradctemp.k     = 100;    // Filter time constant
	p->calintern.iiradctemp.scale = 4;

	// Internal voltage ref: ADC1IDX_INTERNALVREF // IN18     - Internal voltage reference
	p->calintern.fvdd   = 3.298f;   // Vdd for following Vref ADC reading
	p->calintern.adcvdd = 27093;   //(16*1495.5) ADC reading (DMA sum) for above Vdd
	p->calintern.fvref  = 1.223f;  // reference voltage

	// Internal temperature: ADC1IDX_INTERNALTEMP // IN17     - Internal temperature sensor
	p->calintern.adcrmtmp  = 17838; // Room temp ADC (DMA sum) reading
	p->calintern.frmtemp   = 25.0f;  // Room temp for ADC reading     
	p->calintern.fslope    =  4.3f;   // mv/degC slope of temperature sensor
	p->calintern.fvreftmpco= 15.0f;    // Vref temp coefficient (15 is based on similar parts)
	p->calintern.fvtemp    =  1.40f;  // Vtemp voltage at 25 degC

	p->calintern.vcc = 3.3033; // 3.3v regulator measured voltage
	p->powergone = 13.0f; // Below this dc-dc converter voltge assume CAN power is gone



/* ADC channels (except for ADC_IN1 used with BMS).*/
#define CELLTC 0.70f // Default filter time constant
#define SKIPCT 3    // Ignore initial readings to filter
#define DEFAULTSCALE 1.2591575E-05f // Base on nominal values

// ADC1IDX_PC3_OPA_OUT   0	// IN4	  2.5	8
	p->cabs[ADC1IDX_PC3_OPA_OUT].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PC3_OPA_OUT].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PC3_OPA_OUT].scale     = (1.0f/(8*16)); // 
	p->cabs[ADC1IDX_PC3_OPA_OUT].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA0_OPA_INP	  1 // IN5	 24.5	9
	p->cabs[ADC1IDX_PA0_OPA_INP].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PA0_OPA_INP].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PA0_OPA_INP].scale     = 3.8156288E-03f; // 
	p->cabs[ADC1IDX_PA0_OPA_INP].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA3_FET_CUR1  2 // IN8    12.5  10  OPA_OUT (PA0 amplified) 
	p->cabs[ADC1IDX_PA3_FET_CUR1].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PA3_FET_CUR1].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PA3_FET_CUR1].scale     = 3.8156288E-03f; // 
	p->cabs[ADC1IDX_PA3_FET_CUR1].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA7_HV_DIV    3	// IN12	640.5	4
	p->cabs[ADC1IDX_PA7_HV_DIV].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PA7_HV_DIV].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PA7_HV_DIV].scale     = 6.531239E-4f;//5.424129E-04f; // Apply calibration below 
	p->cabs[ADC1IDX_PA7_HV_DIV].ioffset    = 0;  // Offset before scale

// ADC1IDX_PC4_THERMSP1  4	// IN13	247.5	6
	p->cabs[ADC1IDX_PC4_THERMSP1].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PC4_THERMSP1].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PC4_THERMSP1].scale     = 2.520833E-05f; // Apply calibration below 
	p->cabs[ADC1IDX_PC4_THERMSP1].ioffset    = 0;  // Offset before scale

// ADC1IDX_PC5_THERMSP2  5	// IN14	247.5	7
	p->cabs[ADC1IDX_PC5_THERMSP2].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PC5_THERMSP2].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PC5_THERMSP2].scale     = 2.520833E-05f; // Apply calibration below 
	p->cabs[ADC1IDX_PC5_THERMSP2].ioffset    = 0;  // Offset before scale

// ADC1IDX_INTERNALVREF  6	// IN0	247.5	1	
	p->cabs[ADC1IDX_INTERNALVREF].iir_f1.coef     = 0.99f; // Filter time constant
	p->cabs[ADC1IDX_INTERNALVREF].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_INTERNALVREF].scale     = 2.520833E-05f; // Apply calibration below 
	p->cabs[ADC1IDX_INTERNALVREF].ioffset    = 0;  // Offset before scale

// ADC1IDX_INTERNALTEMP  7	// IN17	247.5	2
	p->cabs[ADC1IDX_INTERNALTEMP].iir_f1.coef     = 0.99f; // Filter time constant
	p->cabs[ADC1IDX_INTERNALTEMP].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_INTERNALTEMP].scale     = 0.03125; //  1/32 scale factor
	p->cabs[ADC1IDX_INTERNALTEMP].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA4_DC_DC     8 // IN9	247.5   3	6.8K|33K divider
	p->cabs[ADC1IDX_PA4_DC_DC].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[ADC1IDX_PA4_DC_DC].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[ADC1IDX_PA4_DC_DC].scale     = 1.478422E-04f;
	p->cabs[ADC1IDX_PA4_DC_DC].ioffset    = 0;  // Offset before scale

	/* Initialize iir filter. */
	for (int i = 0; i < ADCDIRECTMAX; i++)
	{
		p->cabs[i].iir_f1.onemcoef = 1 - p->cabs[i].iir_f1.coef;
	}

	return 0;	
}