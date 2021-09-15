/******************************************************************************
* File Name          : filters.c
* Date First Issued  : 03/09/2019
* Board              : DiscoveryF4
* Description        : Various filters for filtering readings
*******************************************************************************/

#include "filters.h"

/* *************************************************************************
 * void filters_do(union ADCFILTEREDVAL* pret, struct FILTERCOMPLETE* pfc, union ADCFILTEREDVAL* pval, uint8_t filtertype);
 * @brief	: filter input value 
 * @param	: pret = Pointer to union for return value
 * @param	: pfc = Pointer to struct holding fixed parameters and intermediate variables
 * @param	: pval = Pointer to union with new value input to filter (fix or float)
 * @param	: filtertype = code for type of filter
 * *************************************************************************/
void filters_do(union ADCFILTEREDVAL* pret, struct FILTERCOMPLETE* pfc, union ADCFILTEREDVAL* pval, uint8_t filtertype)
{
	union ADCFILTEREDVAL filtref;
	switch(filtertype)
	{
		case ADCFILTERTYPE_NONE: // Skip filtering
			break;

		case ADCFILTERTYPE_IIR1: // Single pole IIR
			if (pfc->skipctr > 0)
			{ // Here, skip starting filter until a few readings
				pfc->skipctr -= 1;
				pfc->work.f[ADCFILT_Z1] = *pval.f/pfc->coef.f[ADCFILT_COEF1];
			}
			else
			{
				pfc->work.f[ADCFILT_Z1] += (*pval.f - (pfc->work.f[ADCFILT_Z1] * pfc->coef.f[ADCFILT_COEF1]) );
			}
			pret->f = ( pfc->work.f[ADCFILT_Z1] * pfc->coef.f[ADCFILT_COEF1] );
			return 

		case ADCFILTERTYPE_IIR2: // Second order IIR
			break;
	}
	
	return;
}
