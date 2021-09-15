/******************************************************************************
* File Name          : iir_f2.c
* Date First Issued  : 04/16/2019
* Board              : DiscoveryF4
* Description        : IIR filter: second order, float
*******************************************************************************/

#include "iir_f2.h"
#include <math.h>

/* *************************************************************************
 * void iir_f2_coefficients(struct FILTERIIRF2* pfc, float Fc, float Q, uint16_t skipct);
 * @brief	: Compute coefficients for Butterworth 2nd order IIR
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: Fc = cutoff freq as ratio, e.g. 0.1
 * @param	: Q = e.g. .707
 * @param	: skipct = Number of initial readings to not filter
 * *************************************************************************/
void iir_f2_coefficients(struct FILTERIIRF2* pfc, float Fc, float Q, uint16_t skipct)
{
	float K    = tanf(Fc * 3.14159265);
	float norm = 1 / (1 + K / Q + K * K);
   pfc->b1    = 2 * (K * K - 1) * norm;
   pfc->b2    = -(1 - K / Q + K * K) * norm;
	pfc->gain  = 4 * (K * K * norm);
	pfc->z1    = 0;
	pfc->z2    = 0;
	pfc->skipctr = skipct; // Number of initial readings to not filter
	return;
}
/* *************************************************************************
 * float iir_f2_f(struct FILTERIIRF2* pfc, float flt);
 * @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: flt = float new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
/*
		 out = in + z1;
   	 z1  = in * a1 + z2 - b1 * out;
   	 z2  = in - b2 * out;
		outf = out * KK;
*/
float iir_f2_f(struct FILTERIIRF2* pfc, float flt)
{
	float out;

	if (pfc->skipctr > 0)
	{ // Here, skip starting filter until a few readings
		pfc->skipctr -= 1;
		out = 0;
	}
	else
	{
		 out      = flt + pfc->z1;
   	 pfc->z1  = pfc->z2 - pfc->b1 * out;
   	 pfc->z2  = pfc->b2 * out;
	}
	return (out * pfc->gain);
}
/* *************************************************************************
 * float iir_f2_64b(struct FILTERIIRF2* pfc, uint64_t* pval);
 * @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: pval = Pointer to 64b new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f2_64b(struct FILTERIIRF2* pfc, uint64_t* pval)	
{
	return iir_f2_f( pfc, (float)(*pval) );
}
/* *************************************************************************
 * float iir_f2_32b(struct FILTERIIRF2* pfc, uint32_t val);
 * @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: pval = Pointer to 32b new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f2_32b(struct FILTERIIRF2* pfc, uint32_t val)
{
	return iir_f2_f( pfc, (float)(val) );
}

