/******************************************************************************
* File Name          : adcfastsum16.c
* Date First Issued  : 03/16/2019
* Description        : Fast sum: ADC DMA buffering--16 sequences, 'N' channels
*******************************************************************************/
#include "adcfastsum16.h"
/* *************************************************************************
 * void adcfastsum16(struct ADCCHANNEL* pchan, uint16_t* pdma);
 *	@brief	: Inline fast summation: ASSUMES 16 ADC =>sequences<=: channels = ADC1IDX_ADCSCANSIZE
 * @param	: pchan = pointer to stuct array for adc1 channels
 * @param	: pdma  = pointer to dma buffer
 * *************************************************************************/
void adcfastsum16(struct ADCCHANNEL* pchan, uint16_t* pdma)
{
	struct ADCCHANNEL* pend = pchan + ADC1IDX_ADCSCANSIZE;
	do
	{
		pchan->sum = *pdma
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 0)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 1)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 2)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 3)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 4)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 5)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 6)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 7)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 8)
		 + *(pdma + ADC1IDX_ADCSCANSIZE * 9)
		 + *(pdma + ADC1IDX_ADCSCANSIZE *10)
		 + *(pdma + ADC1IDX_ADCSCANSIZE *11)
		 + *(pdma + ADC1IDX_ADCSCANSIZE *12)
		 + *(pdma + ADC1IDX_ADCSCANSIZE *13)
		 + *(pdma + ADC1IDX_ADCSCANSIZE *14)
		 + *(pdma + ADC1IDX_ADCSCANSIZE *15);

		pchan += 1;
		pdma  += 1;
	} while (pchan != pend);
	return;
}
