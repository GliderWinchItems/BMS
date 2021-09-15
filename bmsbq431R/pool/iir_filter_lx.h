/******************************************************************************
* File Name          : iir_filter_lx.h
* Date First Issued  : 05/18/2016
* Board              : --
* Description        : Single pole IIR filter using int32_t
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IIR_FILTER_LX
#define __IIR_FILTER_LX

#include <stdint.h>


/*
Basic--
Z = Z - Z/k + input

But with scaling--
Z = Z + (input * scale - Z)/k

output unscaled = Z/scale

Corner freq (radians): Omega = Fsample/K

*/

// IIR filter (int) parameters
struct IIR_L_PARAM
{
	int32_t k;		// Filter parameter for setting time constant
	int32_t scale;		// Scaling improve spare bits with integer math
};

struct IIRFILTERL
{
	int32_t z;		// Z^(-1)
	float f_out;		// output as float
	struct IIR_L_PARAM* pprm; // Pointer to k and scale for this filter
	uint8_t sw;		// Init switch
};

/******************************************************************************/
int32_t iir_filter_lx_do(struct IIRFILTERL* pfil, uint16_t* pval);
/* @brief	: Pass an input value through the filter.
 * @param	: pfil = pointer to struct with filter stuff
 * @param	: pval = pointer to reading
 * @return	: latest filtered value
*******************************************************************************/
void iir_filter_lx_float(struct IIRFILTERL* pfil);
/* @brief	: Convert unscaled int to scaled float
 * @param	: pfil = pointer to struct with filter stuff
*******************************************************************************/


#endif 
