/******************************************************************************
* File Name          : adc_idx_v_struct.c
* Date First Issued  : 10/27/2021
* Board              : bmsbmsbq: STM32L431
* Description        : Load sram local copy of parameters
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


#define DEFAULTSCALEBMS 7.801654E-05f  // 
#define DEFAULTOFFSETBMS 9.875408E-02f //
	
/* BMS cells (all read via ADC_IN1) */
	p->cabsbms[0].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[0].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[1].scale     = 7.801654E-05f; // Apply calibration below 
	p->cabsbms[1].offset    = 9.875408E-02f; // Offset before scale

	p->cabsbms[2].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[2].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[3].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[3].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[4].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[4].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[5].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[5].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[6].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[6].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[7].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[7].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[8].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[8].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[9].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[9].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[10].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[10].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[11].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[11].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[12].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[12].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[13].scale     = DEFAULTSCALEBMS; // Apply calibation below 
	p->cabsbms[13].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[14].scale     = DEFAULTSCALEBMS; // Apply calibration below 
	p->cabsbms[14].offset    = DEFAULTOFFSETBMS;  // Offset before scale

	p->cabsbms[15].scale     = 7.985509E-05f; // Apply calibration below 
	p->cabsbms[15].offset    = 1.082652E-02f;  // Offset before scale

/* Thermistors */
#define GENERICTHERM_SCALE   -3.559596E-03f
#define GENERICTHERM_OFFSET	  1.874780E+02f 

	p->cabsbms[16].scale    = GENERICTHERM_SCALE; // Apply calibration below 
	p->cabsbms[16].offset   = GENERICTHERM_OFFSET;  // Offset before scale

	p->cabsbms[17].scale    = GENERICTHERM_SCALE; // Apply calibration below 
	p->cabsbms[17].offset   = GENERICTHERM_OFFSET;  // Offset before scale

	p->cabsbms[18].scale    = GENERICTHERM_SCALE; // Apply calibration below 
	p->cabsbms[18].offset   = GENERICTHERM_OFFSET;  // Offset before scale

/* Top-of-stack */
	p->cabsbms[19].scale    =  1.265765E-03f; // Apply calibration below 
	p->cabsbms[19].offset   =  6.605322E-02f;  // Offset before scale


/* ADC channels (except for ADC_IN1 used with BMS).*/
#define CELLTC 0.70f // Default filter time constant
#define SKIPCT 3    // Ignore initial readings to filter
#define DEFAULTSCALE 1.2591575E-05f // Base on nominal values
// ADC1IDX_INTERNALVREF  0	// IN0	247.5	1	
	p->cabsadc[0].iir_f1.coef     = 0.99f; // Filter time constant
	p->cabsadc[0].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[0].scale     = 1.2591575E-05f; // Apply calibration below 
	p->cabsadc[0].ioffset    = 0;  // Offset before scale

// ADC1IDX_INTERNALTEMP  1	// IN17	247.5	2
	p->cabsadc[1].iir_f1.coef     = 0.99f; // Filter time constant
	p->cabsadc[1].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[1].scale     = (1.0f/(4.0f*16.0f)); // Apply calibration below 
	p->cabsadc[1].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA4_DC_DC     2 // IN9	247.5   3	6.8K|33K divider
	p->cabsadc[2].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[2].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[2].scale     = 7.3697748E-05f;
; // Apply calibration below 
	p->cabsadc[2].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA7_HV_DIV    3	// IN12	640.5	4
	p->cabsadc[3].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[3].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[3].scale     = 2.7205433E-04f; // Apply calibration below 
	p->cabsadc[3].ioffset    = 0;  // Offset before scale

// ADC1IDX_PC1_FET_CUR   4	// IN2	 47.5	5
	p->cabsadc[4].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[4].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[4].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabsadc[4].ioffset    = 0;  // Offset before scale

// ADC1IDX_PC4_THERMSP1  5	// IN13	247.5	6
	p->cabsadc[5].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[5].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[5].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabsadc[5].ioffset    = 0;  // Offset before scale

// ADC1IDX_PC5_THERMSP2  6	// IN14	247.5	7
	p->cabsadc[6].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[6].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[6].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabsadc[6].ioffset    = 0;  // Offset before scale

// ADC1IDX_PC3_OPA_OUT   7	// IN4	  2.5	8
	p->cabsadc[7].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[7].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[7].scale     = (1.0f/(8*16)); // 
	p->cabsadc[7].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA0_OPA_INP	  8 // IN5	 24.5	9
	p->cabsadc[8].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[8].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[8].scale     = 3.8156288E-03f; // 
	p->cabsadc[8].ioffset    = 0;  // Offset before scale

// ADC1IDX_PA3_FET_CUR1  9 // IN8    12.5  10  OPA_OUT (PA0 amplified) 
	p->cabsadc[9].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabsadc[9].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabsadc[9].scale     = 3.8156288E-03f; // 
	p->cabsadc[9].ioffset    = 0;  // Offset before scale

	/* Initialize iir filter. */
	for (int i = 0; i < ADCDIRECTMAX; i++)
	{
		p->cabsadc[i].iir_f1.onemcoef = 1 - p->cabsadc[i].iir_f1.coef;
	}

	return 0;	
}