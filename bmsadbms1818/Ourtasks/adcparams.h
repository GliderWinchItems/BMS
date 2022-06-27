/******************************************************************************
* File Name          : adcparams.h
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : Parameters for ADC app configuration
*******************************************************************************/
/* 
03/03/2022 revised for bmsmax14921.c
06/02/2022 revised for bmabms1818.c
*/

#ifndef __ADCPARAMS
#define __ADCPARAMS

//#include "iir_filter_lx.h"
#include "iir_f1.h"
#include "adc_idx_v_struct.h"
#include "ADCTask.h"





/* Timing
1	2.5	12.5	15						
2	24.5	12.5	37						
3	24.5	12.5	37						
4	640.5	12.5	653						
5	247.5	12.5	260						
6	247.5	12.5	260						
7	247.5	12.5	260						
8	247.5	12.5	260						
9	247.5	12.5	260						

Cycles	MHz	    us	over   us	DMA sum	Total ms
2042	80	25.525	16	408.4	16	    6.5344
2042	80	25.525	16	408.4	16	    6.5344
*/


/* Processor ADC reading sequence/array indices */
/* These indices -=>MUST<= match the hardware ADC scan sequence in STM32CubeMX. */
#define ADC1IDX_PC3_OPA_OUT   0	// IN4     2.5   FET current sense 0.1 ohm:COMP2_INP
#define ADC1IDX_PA0_OPA_INP	  1 // IN5    24.5   FET current sense RC 1.8K|0.1u
#define ADC1IDX_PA3_FET_CUR1  2 // IN8    24.5   OPA_OUT (PA0 amplified) 
#define ADC1IDX_PA7_HV_DIV    3	// IN12  640.5   HV divider (FET side of diode)
#define ADC1IDX_PC4_THERMSP1  4	// IN13  247.5   Spare thermistor 1: 10K pullup
#define ADC1IDX_PC5_THERMSP2  5	// IN14  247.5   Spare thermistor 2: 10K pullup
#define ADC1IDX_INTERNALVREF  6	// IN0   247.5   Internal voltage reference
#define ADC1IDX_INTERNALTEMP  7	// IN17  247.5   Internal temperature sensor
#define ADC1IDX_PA4_DC_DC     8 // IN9   247.5   Isolated DC-DC 15v supply

/* ADBMS1818 ADC reading sequence/array indices */
#define BMSAUX_1_NC           0 // GPIO 1 No Connection
#define BMSAUX_2_THERM1       1 // GPIO 2 Thermistor
#define BMSAUX_3_THERM3       2 // GPIO 3 Thermistor
#define BMSAUX_4_THERM2       3 // GPIO 4 Thermistor
#define BMSAUX_5_US6          4 // GPIO 5 Spare: U$6
#define BMSAUX_6_CUR_SENSE    5 // GPIO 6 Current sense op amp
#define BMSAUX_7_HALL         6 // GPIO 7 Hall effect sensor signal
#define BMSAUX_8_US9          7 // GPIO 8 Spare: U$9
#define BMSAUX_9_US10         8 // GPIO 9 Spare: U$10

/* This holds calibration values common to all ADC modules. 
     Some of these are not used. */
struct ADCCALCOMMON
{
	// Internal voltage reference
	float fvref;         // Vref: 1.18 min, 1.21 typ, 1.24 max
	float fadc;          // Float of struct ADC1CALINTERNAL adcvdd

	float tcoef;         // Vref: Temp coefficient (ppm/deg C: 30 typ; 50 max)
	float fvdd;          // Vdd: float (volts)
	float fvddfilt;      // Vdd: float (volts), filtered
	uint16_t ivdd;       // Vdd: fixed (mv)
	uint16_t ts_vref;
	uint32_t adccmpvref; // scaled vref compensated for temperature

	// Internal temperature sensor (floats)
	float ts_cal1;    // Vtemp: TS_CAL1 converted to float
	float ts_cal2;    // Vtemp: TS_CAL2 converted to float
	float ts_caldiff; // (ts_cal2 - ts_cal1)
	float ts_calrate; // (calibration temp diff)/ts_calrate
	float v25;           // Vtemp: 25 deg C (0.76v typ)
    float slope;         // Vtemp: mv/degC 
	float offset;        // Vtemp: offset
	float degC;          // Temperature: degrees C
	float degCfilt;      // Temperature: degrees C, filtered
 	uint32_t dmact;      // DMA interrupt running counter
 	uint32_t dmact2;      // DMA interrupt running counter 2nd 1/2

 	float fvrefajd; // Vref temerature adjustment factor (nominally 1.0000000f)

 	uint32_t vref_sum;
 	uint32_t vtemp_sum;

	// For integer computation (much faster)
	uint32_t uicaldiff;
	int64_t ll_80caldiff;
	uint32_t ui_cal1;
	uint32_t ui_tmp;
};

/* Working values for absolute voltages adjusted using Vref. */
struct ADCABS
{
	float filt;      // Filtered ADC reading
	float f;         // Not filtered
	int32_t sum;     // Working sum of multiple scans
	int32_t sumsave; // Saved sum for printf'ing
};
struct ADCCHANNEL	
{
	struct FILTERIIRF1 iir_f1;	// iir_f1 (float) filter
	float    fscale;  // Scale factor
	float    offset;  // Offset
	uint32_t sum;     // Fast Sum of ADC readings
};
/* Everything for the ADC in one struct. */
struct ADCFUNCTION
{
	struct ADCLC lc;    // Local Copy of parameters
	struct ADCCALCOMMON common; // Vref & temp stuff
	struct ADCABS abs[ADCDIRECTMAX]; // Absolute readings
	struct ADCCHANNEL chan[ADCDIRECTMAX]; // ADC sums, calibrated endpt
	uint32_t ctr; // Running count of updates.
	uint16_t dmabuf[ADCDIRECTMAX]; // Readings for one ADC scan w DMA
	uint8_t sumctr; // DMA summation counter
};


/* *************************************************************************/
void adcparams_init(void);
/*	@brief	: Copy parameters into structs
 * NOTE: => ASSUMES ADC1 ONLY <==
 * *************************************************************************/
void adcparams_internal(struct ADCCALCOMMON* pacom, struct ADCFUNCTION* padc1);
/*	@brief	: Update values used for compensation from Vref and Temperature
 * @param	: pacom = Pointer calibration parameters for Temperature and Vref
 * @param	: padc1 = Pointer to array of ADC reading sums plus other stuff
 * *************************************************************************/
void adcparams_chan(uint8_t adcidx);
/*	@brief	: calibration, compensation, filtering for channels
 * @param	: adcidx = index into ADC1 array
 * *************************************************************************/
void adcparams_cal(void);
/*	@brief	: calibrate and filter ADC readings (from ADCTask.c)
 * *************************************************************************/
void adcparams_calibadc(void);
/*	@brief	: calibrate and filter ADC channel readings (from ADCTask.c)
 * *************************************************************************/

/* Calibration values common to all ADC modules. */
extern struct ADCCALCOMMON adcommon;

/* Everything for ADC1. */
extern struct ADCFUNCTION adc1;


#endif
