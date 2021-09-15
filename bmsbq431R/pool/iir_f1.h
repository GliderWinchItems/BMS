/******************************************************************************
* File Name          : iir_f1.h
* Date First Issued  : 03/10/2019
* Board              : DiscoveryF4
* Description        : IIR filter: singled pole, float
*******************************************************************************/

#ifndef __IIR_F1
#define __IIR_F1

#include <stdint.h>

/* With this struct one pointer will convey everything necessary. */
struct FILTERIIRF1
{
	float coef;      // coefficient
	float onemcoef;  // 1 - coef 
	float z1;        // Z^-1
	uint16_t skipctr; // Number of initial readings to not filter
};

/* *************************************************************************/
float iir_f1_f(struct FILTERIIRF1* pfc, float flt);
/* @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: flt = float new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f1_64b(struct FILTERIIRF1* pfc, uint64_t* pval);
/* @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: pval = Pointer to 64b new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f1_32b(struct FILTERIIRF1* pfc, uint32_t val);
/* @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: val = 32b new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
#endif
