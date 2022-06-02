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

/* Cell # 1 */
p->cabsbms[ 0].coef[0] = 1.7924937E-03f;
p->cabsbms[ 0].coef[1] = 7.9808664E-05f;
p->cabsbms[ 0].coef[2] = -3.8067549E-12f;

/* Cell # 2 */
p->cabsbms[ 1].coef[0] = 6.6159880E-03f;
p->cabsbms[ 1].coef[1] = 7.9818936E-05f;
p->cabsbms[ 1].coef[2] = 1.5956986E-12f;

/* Cell # 3 */
p->cabsbms[ 2].coef[0] = 6.5167277E-03f;
p->cabsbms[ 2].coef[1] = 7.9883063E-05f;
p->cabsbms[ 2].coef[2] = 1.2177967E-12f;

/* Cell # 4 */
p->cabsbms[ 3].coef[0] = 6.2407913E-03f;
p->cabsbms[ 3].coef[1] = 7.9867785E-05f;
p->cabsbms[ 3].coef[2] = 7.1887061E-13f;

/* Cell # 5 */
p->cabsbms[ 4].coef[0] = 6.2358110E-03f;
p->cabsbms[ 4].coef[1] = 7.9932288E-05f;
p->cabsbms[ 4].coef[2] = 3.0219361E-13f;

/* Cell # 6 */
p->cabsbms[ 5].coef[0] = 6.3690845E-03f;
p->cabsbms[ 5].coef[1] = 7.9855569E-05f;
p->cabsbms[ 5].coef[2] = 9.7874198E-13f;

/* Cell # 7 */
p->cabsbms[ 6].coef[0] = 6.3182106E-03f;
p->cabsbms[ 6].coef[1] = 7.9896611E-05f;
p->cabsbms[ 6].coef[2] = 8.9827365E-13f;

/* Cell # 8 */
p->cabsbms[ 7].coef[0] = 6.2117114E-03f;
p->cabsbms[ 7].coef[1] = 7.9852063E-05f;
p->cabsbms[ 7].coef[2] = 8.5758924E-13f;

/* Cell # 9 */
p->cabsbms[ 8].coef[0] = 6.3688441E-03f;
p->cabsbms[ 8].coef[1] = 7.9949922E-05f;
p->cabsbms[ 8].coef[2] = 5.2131288E-13f;

/* Cell #10 */
p->cabsbms[ 9].coef[0] = 5.9435284E-03f;
p->cabsbms[ 9].coef[1] = 7.9766468E-05f;
p->cabsbms[ 9].coef[2] = -8.7212947E-13f;

/* Cell #11 */
p->cabsbms[10].coef[0] = 5.6803634E-03f;
p->cabsbms[10].coef[1] = 7.9912874E-05f;
p->cabsbms[10].coef[2] = 3.1915195E-15f;

/* Cell #12 */
p->cabsbms[11].coef[0] = 6.1138929E-03f;
p->cabsbms[11].coef[1] = 7.9890320E-05f;
p->cabsbms[11].coef[2] = 7.6416567E-13f;

/* Cell #13 */
p->cabsbms[12].coef[0] = 5.8436984E-03f;
p->cabsbms[12].coef[1] = 7.8946342E-05f;
p->cabsbms[12].coef[2] = 5.3193420E-13f;

/* Cell #14 */
p->cabsbms[13].coef[0] = 5.3614831E-03f;
p->cabsbms[13].coef[1] = 7.9986061E-05f;
p->cabsbms[13].coef[2] = -1.2912097E-13f;

/* Cell #15 */
p->cabsbms[14].coef[0] = 6.3191408E-03f;
p->cabsbms[14].coef[1] = 7.9789681E-05f;
p->cabsbms[14].coef[2] = 9.2801063E-13f;

/* Cell #16 */
p->cabsbms[15].coef[0] = -2.3186650E-03f;
p->cabsbms[15].coef[1] = 8.0538161E-05f;
p->cabsbms[15].coef[2] = -8.1326765E-12f;

/* Top-of-stack */
p->cabsbms[19].coef[0] = 8.8203303E-02f;
p->cabsbms[19].coef[1] = 1.2626728E-03f;
p->cabsbms[19].coef[2] = 7.9562712E-12f;


/* Thermistors */
#define GENERICTHERM_SCALE   -3.559596E-03f
#define GENERICTHERM_OFFSET	  1.874780E+02f 

/* Thermistor # 1 */
p->cabsbms[16].coef[0] = 2.0190213E+02f;
p->cabsbms[16].coef[1] = -4.8305505E-03f;
p->cabsbms[16].coef[2] = 2.4866130E-08f;

/* Thermistor # 2 */
p->cabsbms[17].coef[0] = 2.0162182E+02f;
p->cabsbms[17].coef[1] = -4.7452832E-03f;
p->cabsbms[17].coef[2] = 2.3172558E-08f;

/* Thermistor # 3 */
p->cabsbms[18].coef[0] = 2.0023062E+02f;
p->cabsbms[18].coef[1] = -4.6834758E-03f;
p->cabsbms[18].coef[2] = 2.2552685E-08f;


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