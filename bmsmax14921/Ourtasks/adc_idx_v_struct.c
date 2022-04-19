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
	p->cabsbms[0].scale     =	8.2017977E-05f;
	p->cabsbms[0].offset    =	-0.0747246309f;
		
	p->cabsbms[1].scale     =	7.975498E-05f;
	p->cabsbms[1].offset    =	4.453443E-03f;
		
	p->cabsbms[2].scale     =	8.009272E-05f;
	p->cabsbms[2].offset    =	4.860598E-03f;
		
	p->cabsbms[3].scale     =	8.009272E-05f;
	p->cabsbms[3].offset    =	4.860598E-03f;
		
	p->cabsbms[4].scale     =	8.009272E-05f;
	p->cabsbms[4].offset    =	4.860598E-03f;
		
	p->cabsbms[5].scale     =	8.009272E-05f;
	p->cabsbms[5].offset    =	4.860598E-03f;
		
	p->cabsbms[6].scale     =	8.009272E-05f;
	p->cabsbms[6].offset    =	4.860598E-03f;
		
	p->cabsbms[7].scale     =	8.009272E-05f;
	p->cabsbms[7].offset    =	4.860598E-03f;
		
	p->cabsbms[8].scale     =	8.009272E-05f;
	p->cabsbms[8].offset    =	4.860598E-03f;
		
	p->cabsbms[9].scale     =	8.009272E-05f;
	p->cabsbms[9].offset    =	4.860598E-03f;
		
	p->cabsbms[10].scale    =	8.009272E-05f;
	p->cabsbms[10].offset   =	4.860598E-03f;
		
 	p->cabsbms[11].scale    =	8.009272E-05f;
 	p->cabsbms[11].offset   =	4.860598E-03;
		
 	p->cabsbms[12].scale    =	8.009272E-05f;
 	p->cabsbms[12].offset   =	4.860598E-03f;
		
 	p->cabsbms[13].scale    =	8.009272E-05f;
 	p->cabsbms[13].offset   =	4.860598E-03f;
		
 	p->cabsbms[14].scale    =	8.009272E-05f;
 	p->cabsbms[14].offset   =	4.860598E-03f;
		
 	p->cabsbms[15].scale    =	7.911967E-05f;
 	p->cabsbms[15].offset   =	5.479308E-04f;


/* Cell # 1 */
p->cabsbms[ 0].coef[0] = -1.1729743E-01f;
p->cabsbms[ 0].coef[1] = 8.5339489E-05f;
p->cabsbms[ 0].coef[2] = -5.5499357E-11f;

/* Cell # 2 */
p->cabsbms[ 1].coef[0] = 6.3814115E-03f;
p->cabsbms[ 1].coef[1] = 7.9754241E-05f;
p->cabsbms[ 1].coef[2] = 1.3467795E-12f;

/* Cell # 3 */
p->cabsbms[ 2].coef[0] = 5.3961362E-03f;
p->cabsbms[ 2].coef[1] = 7.9881731E-05f;
p->cabsbms[ 2].coef[2] = -3.3040039E-13f;

/* Cell # 4 */
p->cabsbms[ 3].coef[0] = 5.4650269E-03f;
p->cabsbms[ 3].coef[1] = 7.9828712E-05f;
p->cabsbms[ 3].coef[2] = -2.4937277E-13f;

/* Cell # 5 */
p->cabsbms[ 4].coef[0] = 6.0534917E-03f;
p->cabsbms[ 4].coef[1] = 7.9841237E-05f;
p->cabsbms[ 4].coef[2] = 1.1430347E-13f;

/* Cell # 6 */
p->cabsbms[ 5].coef[0] = 6.0401129E-03f;
p->cabsbms[ 5].coef[1] = 7.9807511E-05f;
p->cabsbms[ 5].coef[2] = 3.1293674E-13f;

/* Cell # 7 */
p->cabsbms[ 6].coef[0] = 5.5433207E-03f;
p->cabsbms[ 6].coef[1] = 7.9867302E-05f;
p->cabsbms[ 6].coef[2] = -4.3752388E-13f;

/* Cell # 8 */
p->cabsbms[ 7].coef[0] = 5.2515543E-03f;
p->cabsbms[ 7].coef[1] = 7.9836976E-05f;
p->cabsbms[ 7].coef[2] = -7.5202478E-13f;

/* Cell # 9 */
p->cabsbms[ 8].coef[0] = 5.2353259E-03f;
p->cabsbms[ 8].coef[1] = 7.9942722E-05f;
p->cabsbms[ 8].coef[2] = -1.1770495E-12f;

/* Cell #10 */
p->cabsbms[ 9].coef[0] = 4.4327351E-03f;
p->cabsbms[ 9].coef[1] = 7.9793480E-05f;
p->cabsbms[ 9].coef[2] = -3.0795516E-12f;

/* Cell #11 */
p->cabsbms[10].coef[0] = 6.0888414E-03f;
p->cabsbms[10].coef[1] = 7.9740490E-05f;
p->cabsbms[10].coef[2] = 6.9097887E-13f;

/* Cell #12 */
p->cabsbms[11].coef[0] = 5.5766088E-03f;
p->cabsbms[11].coef[1] = 7.9802967E-05f;
p->cabsbms[11].coef[2] = -5.1594747E-14f;

/* Cell #13 */
p->cabsbms[12].coef[0] = 4.5999072E-03f;
p->cabsbms[12].coef[1] = 7.9927937E-05f;
p->cabsbms[12].coef[2] = -1.5410180E-12f;

/* Cell #14 */
p->cabsbms[13].coef[0] = 4.8298955E-03f;
p->cabsbms[13].coef[1] = 7.9904123E-05f;
p->cabsbms[13].coef[2] = -1.2769071E-12f;

/* Cell #15 */
p->cabsbms[14].coef[0] = 5.5602980E-03f;
p->cabsbms[14].coef[1] = 7.9763098E-05f;
p->cabsbms[14].coef[2] = -8.7080830E-14f;

/* Cell #16 */
p->cabsbms[15].coef[0] = 5.3164966E-03f;
p->cabsbms[15].coef[1] = 7.9950536E-05f;
p->cabsbms[15].coef[2] = -1.2622491E-12f;


/* " " */
/* 	f  */

/* Thermistors */
#define GENERICTHERM_SCALE   -3.559596E-03f
#define GENERICTHERM_OFFSET	  1.874780E+02f 

	p->cabsbms[16].scale    = GENERICTHERM_SCALE; 
	p->cabsbms[16].offset   = GENERICTHERM_OFFSET;

	p->cabsbms[17].scale    = GENERICTHERM_SCALE; 
	p->cabsbms[17].offset   = GENERICTHERM_OFFSET;

	p->cabsbms[18].scale    = GENERICTHERM_SCALE; 
	p->cabsbms[18].offset   = GENERICTHERM_OFFSET;

/* Top-of-stack */
	p->cabsbms[19].scale    =  1.266541E-03f; 
	p->cabsbms[19].offset   =  2.386716E-02f;


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