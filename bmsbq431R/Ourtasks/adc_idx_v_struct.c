/******************************************************************************
* File Name          : adc_idx_v_struct.c
* Date First Issued  : 10/27/2021
* Board              : bmsbmsbq: STM32L431
* Description        : Load sram local copy of parameters
*******************************************************************************/
/* 10/23/2020: Revised for Levelwind */
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
/* Copy for convenience--
 struct ADCGEVCULC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	uint32_t hbct;       // heartbeat count (ms)
	struct ADC1CALINTERNAL calintern; // Vref and Temp internal sensors
	struct ADCCALABS cabs[ADCNUMABS];      // Absolute readings
	struct ADCCALHE  cratio[ADCNUMRATIO];  // Ratometric readings
 };
*/

/* Reproduced for convenience 
  Internal sensor calibration. (Only applies to ADC1) 
struct ADC1CALINTERNAL
{
	struct IIR_L_PARAM iiradcvref; // Filter: adc readings: Vref 
	struct IIR_L_PARAM iiradctemp; // Filter: adc readings: temperature
	float frmtemp;    // (float) Room temp for reading (deg C)
	float fvtemp;     // (float) Voltage of temp sensor at rm temperature
	float fvdd;       // (float) measured Vdd (volts)
	float fslope;     // (float) mv/degC temperature sensor slope
	float fvreftmpco; // (float) Vref temperature coefficient (ppm/degC)
	uint32_t adcvdd;   // (ADC reading) for calibrating Vdd (3.3v)
	uint32_t adcrmtmp; // (ADC reading) room temperature temp sensor reading
};
*/
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



// Battery module cell - (sixteen) ADC0 -ADC15
#define CELLTC 0.70f // Filter time constant
#define SKIPCT 3    // Ignore initial readings to filter
#define DEFAULTSCALE 1.253600E-04f // Base on nominal values
	
	p->cabs[0].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[0].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[0].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[0].offset    = 0.0;  // Offset before scale

	p->cabs[1].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[1].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[1].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[1].offset    = 0.0;  // Offset before scale

	p->cabs[2].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[2].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[2].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[2].offset    = 0.0;  // Offset before scale

	p->cabs[3].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[3].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[3].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[3].offset    = 0.0;  // Offset before scale

	p->cabs[4].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[4].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[4].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[4].offset    = 0.0;  // Offset before scale

	p->cabs[5].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[5].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[5].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[5].offset    = 0.0;  // Offset before scale

	p->cabs[6].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[6].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[6].scale     = DEFAULTSCALE; // Apply calibration below 
	p->cabs[6].offset    = 0.0;  // Offset before scale

	p->cabs[7].iir_f1.coef     = CELLTC; // Filter time constant
	p->cabs[7].iir_f1.skipctr  = SKIPCT;  // Initial skip
	p->cabs[7].scale     = (1.0f/(8*16)); // 
	p->cabs[7].offset    = 0.0;  // Offset before scale

/* Unless the board number is defined the default
   calibrations (above) will be used.

   To assist calibration the spreadsheet 
   ../calibration.boards.ods
   can be used to generate the following, by copying
   the readings from a copy & paste of minicom output.

   NOTE: The right arrow from the spreadsheet requires
   is an escaped character \342 and requiresreplacement 
   with '->'
*/
#define BOARD_NUMBER_1
/* Calibration: Copy & paste from spreadsheet */
#ifdef BOARD_NUMBER_1
p->cabs[ 0].scale =		8.553717E-04; //5.036630E-04;
p->cabs[ 1].scale =		2.772659E-04;
p->cabs[ 2].scale =		1.324014E-03; //1.355241E-03;
p->cabs[ 3].scale =		5.036630E-05;
p->cabs[ 4].scale =		1.007326E-04;
p->cabs[ 5].scale =		1.007326E-04;
p->cabs[ 6].scale =		1.215939E-04;
p->cabs[ 7].scale =		1.292131E-04;

#else
	#ifdef BOARD_NUMBER_2
p->cabs[ 0].scale =		8.553717E-04; //5.036630E-04;
p->cabs[ 1].scale =		2.772659E-04;
p->cabs[ 2].scale =		1.324014E-03; //1.355241E-03;
p->cabs[ 3].scale =		5.036630E-05;
p->cabs[ 4].scale =		1.007326E-04;
p->cabs[ 5].scale =		1.007326E-04;
p->cabs[ 6].scale =		1.215939E-04;
p->cabs[ 7].scale =		1.292131E-04;


	#else
	#error BOARD_1 or BOARD_2 (calibration selection) not specified
	#endif
#endif	

	for (int i = 0; i < 18; i++)
	{
		p->cabs[i].iir_f1.onemcoef = 1 - p->cabs[i].iir_f1.coef;
	}

	return 0;	
}
