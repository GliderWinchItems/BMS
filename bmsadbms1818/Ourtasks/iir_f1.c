/******************************************************************************
* File Name          : iir_f1.c
* Date First Issued  : 03/10/2019
* Board              : DiscoveryF4
* Description        : IIR filter: singled pole, float
*******************************************************************************/

#include "iir_f1.h"

/* *************************************************************************
 * float iir_f1_f(struct FILTERIIRF1* pfc, float flt);
 * @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: flt = float new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f1_f(struct FILTERIIRF1* pfc, float flt)
{
	if (pfc->skipctr > 0)
	{ // Here, skip starting filter until a few readings
		pfc->skipctr -= 1;
		if (flt == 0)
		{
			return 0;
		}
		pfc->z1 = flt / pfc->onemcoef;
	}
	else
	{
		pfc->z1 = (flt + pfc->z1 * pfc->coef);
	}
	return ( pfc->z1 * pfc->onemcoef);
}
/* *************************************************************************
 * float iir_f1_64b(struct FILTERIIRF1* pfc, uint64_t* pval);
 * @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: pval = Pointer to 64b new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f1_64b(struct FILTERIIRF1* pfc, uint64_t* pval)	
{
	return iir_f1_f( pfc, (float)(*pval) );
}
/* *************************************************************************
 * float iir_f1_32b(struct FILTERIIRF1* pfc, uint32_t val);
 * @brief	: filter input value 
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: pval = Pointer to 32b new value input to filter
 * @param	: filter output, given new input
 * *************************************************************************/
float iir_f1_32b(struct FILTERIIRF1* pfc, uint32_t val)
{
	return iir_f1_f( pfc, (float)(val) );
}

