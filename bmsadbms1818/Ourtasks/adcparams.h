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
/*
					         Clock		   Repeat	
					            80	  us	 16	
IN4		1	2.5	    12.5	15	80	0.1875 	 3.0000	FET 0.1
IN5		2	24.5	12.5	37	80	0.4625	 7.4000	FET RC
IN8		3	24.5	12.5	37	80	0.4625	 7.4000	OPA
IN12	4   640.5	12.5	653	80	8.1625 130.6000	HV
IN14	5	247.5	12.5	260	80	3.2500	52.0000 Therm PC5
IN14	6	247.5	12.5	260	80	3.2500	52.0000 Therm PC5
IN0		7	247.5	12.5	260	80	3.2500	52.0000 Vref
IN17	8	247.5	12.5	260	80	3.2500	52.0000 Temp
IN9		9	247.5	12.5	260	80	3.2500	52.0000 DC-DC volt		
						           25.5250  408.4000	
						              MS 2.4485798237	
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
	float f;         // Calibrated, not filtered
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
	struct ADCABS abs[ADCDIRECTMAX]; // Absolute readings calibrated
	struct ADCCHANNEL chan[ADCDIRECTMAX]; //
	uint32_t ctr; // Running count of updates.
	uint8_t sumctr; // DMA summation counter
};


/* *************************************************************************/
void adcparams_init(void);
/*	@brief	: Copy parameters into structs
 * NOTE: => ASSUMES ADC1 ONLY <==
 * *************************************************************************/
 float adcparams_caltemp(void);
/*	@brief	: calibrate processor internal temperature reading
 *  @return : Internal temperature (deg C)
 * *************************************************************************/

/* Calibration values common to all ADC modules. */
extern struct ADCCALCOMMON adcommon;

/* Everything for ADC1. */
extern struct ADCFUNCTION adc1;


#endif
