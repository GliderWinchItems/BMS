/******************************************************************************
* File Name          : adc_idx_v_struct.h
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
                     Min  Typ  Max 
Internal reference: 1.16 1.20 1.24 V
Vref temperautre co.  --   --  100 ppm/DegC

Temperature sensor specs
                 Min  Typ  Max
Average slope    4.0  4.3  4.6 mV/°C
Voltage at 25 °C 1.34 1.43 1.52 

*/
#include <stdint.h>
#include "common_can.h"
#include "iir_filter_lx.h"
#include "adcparams.h"
#include "gevcu_idx_v_struct.h"

#ifndef __ADC_IDX_V_STRUCT
#define __ADC_IDX_V_STRUCT

/* These set the sizes of in "struct ADCFUNCTION" below */
#define ADCNUMABS   18  // Number of Absolute calibrated readings
#define ADCNUMRATIO 1  // Number of Ratiometric calibrated readings



/* Parameters for ADC reading */
// Flointing pt params are converted to scaled integers during initialization

/* Internal sensor calibration. (Only applies to ADC1) */
struct ADC1CALINTERNAL
{
	struct IIR_L_PARAM iiradcvref; // Filter: adc readings: Vref 
	struct IIR_L_PARAM iiradctemp; // Filter: adc readings: temperature
	float fvref;      // Internal reference voltage fixed
	float frmtemp;    // (float) Room temp for reading (deg C)
	float fvtemp;     // (float) Voltage of temp sensor at rm temperature
	float fvdd;       // (float) measured Vdd (volts)
	float fslope;     // (float) mv/degC temperature sensor slope
	float fvreftmpco; // (float) Vref temperature coefficient (ppm/degC)
	uint32_t adcvdd;   // (ADC reading) for calibrating Vdd (3.3v)
	uint32_t adcrmtmp; // (ADC reading) room temperature temp sensor reading
};

//* Absolute (non-ratiometric) sensor calibration. */
/*
The calibrated results are adjusted for Vdd variations by using the
internal voltage reference, and the internal voltage reference is
adjusted for temperature by using the internal temperature reference.
*/
struct ADCCALABS
{
	struct FILTERIIRF1 iir_f1; // Filter: Time constant, integer scaling
	float  scale2;  // X^2
	float  scale;   // X^1
	float  offset;  // X^0
};

/* Parameters for ADC. */
// LC = Local (sram) Copy of parameters
 struct ADCGEVCULC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	uint32_t hbct;       // heartbeat count (ms)
	struct ADC1CALINTERNAL calintern; // Vref and Temp internal sensors
	struct ADCCALABS cabs[ADCNUMABS];      // Absolute readings
		/* Limits for enabling/disabling external charger. */
	float flim_cellhi; // Individual cell max
	float flim_celllo; // Individual cell min (special charging required)
	float flim_packhi; // Total string max
	float flim_relaylo; // Relay switched wire: less than is OK
	float flim_relayhi; // Relay switched wire: greater than is some other cell
 };

/* **************************************************************************************/
int adc_idx_v_struct_hardcode_params(struct ADCGEVCULC* p);
/* @brief	: Hard-code load local copy with parameters
 * @return	: 0
 * ************************************************************************************** */


#endif

