/******************************************************************************
* File Name          : adcfastsum16.h
* Date First Issued  : 03/16/2019
* Description        : Fast sum for summimng ADC DMA buffering
*******************************************************************************/

#ifndef __ADCFASTSUM16
#define __ADCFASTSUM16

#include <stdint.h>
#include "adcparams.h"

#define ADCFASTSUM16SIZE 16 // Number of seqs hard-coded. Use for checks

/* *************************************************************************/
void adcfastsum16(struct ADCCHANNEL* pchan, uint16_t* pdma);
/*	@brief	: Inline fast summation: ASSUMES 16 ADC sequences: channels: ADC1IDX_ADCSCANSIZE
 * @param	: pchan = pointer to stuct array for adc1 channels
 * @param	: pdma  = pointer to dma buffer
 * *************************************************************************/

#endif
