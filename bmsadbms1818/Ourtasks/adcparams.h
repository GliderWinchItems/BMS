/******************************************************************************
* File Name          : adcparams.h
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : Parameters for ADC app configuration
*******************************************************************************/
/* 
03/03/2022 revised for bmsmax14921.c
*/

#ifndef __ADCPARAMS
#define __ADCPARAMS

//#include "iir_filter_lx.h"
#include "iir_f1.h"
#include "adc_idx_v_struct.h"
#include "ADCTask.h"


#define ADCSCANSUM 4         // Number of ADC/DMA scans summed between computations
#define ADCSCALEFACTOR (ADCSCANSUM*16) // Number of scans summed *oversampling number

/* ADC reading sequence/array indices                                           */
/* These indices -=>MUST<= match the hardware ADC scan sequence in STM32CubeMX. */
#define ADC1IDX_INTERNALVREF  0	// IN0   247.5  1   Internal voltage reference
#define ADC1IDX_INTERNALTEMP  1	// IN17  247.5  2   Internal temperature sensor
#define ADC1IDX_PA4_DC_DC     2 // IN9   247.5  3   Isolated DC-DC 15v supply
#define ADC1IDX_PA7_HV_DIV    3	// IN12  640.5  4   HV divider (FET side of diode)
#define ADC1IDX_PC1_BAT_CUR   4	// IN2    47.5  5   Battery current sense op-amp
#define ADC1IDX_PC4_THERMSP1  5	// IN13  247.5  6   Spare thermistor 1: 10K pullup
#define ADC1IDX_PC5_THERMSP2  6	// IN14  247.5  7   Spare thermistor 2: 10K pullup
#define ADC1IDX_PC3_OPA_OUT   7	// IN4     2.5  8   FET current sense 0.1 ohm:COMP2_INP
#define ADC1IDX_PA0_OPA_INP	  8 // IN5    24.5  9   FET current sense RC 1.8K|0.1u
#define ADC1IDX_PA3_FET_CUR1  9 // IN8    12.5  10  OPA_OUT (PA0 amplified) 
#define ADC1IDX_PC0_BMS	     10 // IN1    24.5  11  Not included in adc dma scan

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

/* Everything for the ADC in one struct. */
struct ADCFUNCTION
{
	struct ADCLC lc;    // Local Copy of parameters
	struct ADCCALCOMMON common; // Vref & temp stuff
	struct ADCABS abs[ADCDIRECTMAX]; // Absolute readings
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
float adcparams_calibbms(uint16_t x, uint8_t i);
/*	@brief	: calibrate 
 *  @param  : x = raw ADC count.
 *  @param  : i = index of reading (0 - 19) 20 readings
 *  @return : calibrated value (float volts)
 * *************************************************************************/
void adcparams_calibadc(void);
/*	@brief	: calibrate and filter ADC channel readings (from ADCTask.c)
 * *************************************************************************/

/* Calibration values common to all ADC modules. */
extern struct ADCCALCOMMON adcommon;

/* Everything for ADC1. */
extern struct ADCFUNCTION adc1;


#endif
