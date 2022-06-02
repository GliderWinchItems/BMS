/******************************************************************************
* File Name          : adcextendsum.h
* Date First Issued  : 07/16/2019
* Description        : Sum sums from adcfastsum16.c for long term smoothing and display
*******************************************************************************/

#ifndef __ADCEXTENDSUM
#define __ADCEXTENDSUM

#include <stdint.h>
#include "adcparams.h"

/* *************************************************************************/
void adcextendsum(struct ADCFUNCTION* pcf);
/*	@brief	: Sum the 1/2 DMA sums for greater averaging
 * @param	: pcf = pointer to stuct array for ADCs
 * *************************************************************************/

#endif
