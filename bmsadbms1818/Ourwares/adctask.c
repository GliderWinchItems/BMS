/******************************************************************************
* File Name          : adctask.c
* Board              : BMScable: STM32L431
* Date First Issued  : 02/01/2019
* Description        : Handle ADC w DMA using FreeRTOS/ST HAL within a task
*******************************************************************************/

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "adctask.h"
#include "adcparams.h"
#include "adcfastsum16.h"
#include "ADCTask.h"

#include "DTW_counter.h"
#include "morse.h"

extern ADC_HandleTypeDef hadc1;

struct ADCDMATSKBLK adc1dmatskblk[ADCNUM];

/* *************************************************************************
 * struct ADCDMATSKBLK* adctask_init(ADC_HandleTypeDef* phadc,\
	 uint32_t  notebit1,\
	 uint32_t  notebit2,\
	 uint32_t* pnoteval);
 *	@brief	: Setup ADC DMA buffers and control block
 * @param	: phadc = pointer to 'MX ADC control block
 * @param	: notebit1 = unique bit for notification @ 1/2 dma buffer
 * @param	: notebit2 = unique bit for notification @ end dma buffer
 * @param	: pnoteval = pointer to word receiving notification word from OS
 * @return	: NULL = fail
 * *************************************************************************/
/*
   notebit1 notify at the halfway dma buffer point
     associates with pdma (beginning of dma buffer)
   notebit2 notify at the end of the dma buffer
     associates with pdma + dmact * phadc->Init.NbrOfConversion
*/
uint16_t* dbg_pdma;
uint16_t* dbg_pdma2;

struct ADCDMATSKBLK* adctask_init(ADC_HandleTypeDef* phadc,\
	uint32_t  notebit1,\
	uint32_t  notebit2,\
	uint32_t* pnoteval)
{
	uint16_t* pdma;
	int32_t ret;
	struct ADCDMATSKBLK* pblk = &adc1dmatskblk[0]; // ADC1 only for now

	/* 'adcparams.h' MUST match what STM32CubeMX set up. */
	if (ADCDIRECTMAX != (phadc->Init.NbrOfConversion))
		 morse_trap(61);//return NULL;

	/* ADC DMA summation length must match 1/2 DMA buffer sizing. */
//$	if (ADCFASTSUM16SIZE != ADC1DMANUMSEQ) morse_trap(62);

	/* length = total number of uint16_t in dma buffer */
	uint32_t length = ADCSEQNUM * 2 * ADCDIRECTMAX;


	/* Calibration sequence before enabling ADC. */
	ret = HAL_ADCEx_Calibration_Start(phadc, ADC_SINGLE_ENDED);
	if (ret == HAL_ERROR)  morse_trap(330);

taskENTER_CRITICAL();

	/* Initialize params for ADC. */
	adcparams_init();

	/* Get dma buffer allocated */
	pdma = (uint16_t*)calloc(length, sizeof(uint16_t));
	if (pdma == NULL) {taskEXIT_CRITICAL();morse_trap(63);}
dbg_pdma = pdma;
	/* Populate our control block */
/* The following reproduced for convenience--
struct ADCDMATSKBLK
{
	struct ADCDMATSKBLK* pnext;
	ADC_HandleTypeDef* phadc; // Pointer to 'MX adc control block
	uint32_t  notebit1; // Notification bit for dma half complete interrupt
	uint32_t  notebit2; // Notification bit for dma complete interrupt
	uint32_t* pnoteval; // Pointer to notification word
	uint16_t* pdma1;    // Pointer to first half of dma buffer
	uint16_t* pdma2;    // Pointer to second half of dma buffer
	osThreadId adctaskHandle;
	uint32_t* psum;     // Pointer summed 1/2 dma buffer
	uint16_t  dmact;    // Number of sequences in 1/2 dma buffer
};

*/
	pblk->phadc    = phadc;
	pblk->notebit1 = notebit1;
	pblk->notebit2 = notebit2;
	pblk->pnoteval = pnoteval;
	pblk->pdma1    = pdma;
	pblk->pdma2    = pdma + (ADCSEQNUM * phadc->Init.NbrOfConversion);
	pblk->adctaskHandle = ADCTaskHandle;
dbg_pdma2 = pblk->pdma2;
/**
  * @brief  Enables ADC DMA request after last transfer (Single-ADC mode) and enables ADC peripheral  
  * @param  hadc pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @param  pData The destination Buffer address.
  * @param  Length The length of data to be transferred from ADC peripheral to memory.
  * @retval HAL status
  */

taskEXIT_CRITICAL();
	
	HAL_ADC_Start_DMA(pblk->phadc, (uint32_t*)pblk->pdma1, length);
	return pblk;
}

/* #######################################################################
   ADC DMA interrupt callbacks
   ####################################################################### */
/* *************************************************************************
 * void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
 *	@brief	: Call back from stm32f4xx_hal_adc: Halfway point of dma buffer
 * *************************************************************************/
/* *************************************************************************
 * void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
 *	@brief	: Call back from stm32f4xx_hal_adc: Halfway point of dma buffer
 * *************************************************************************/
uint32_t dwt1,dwt2,dwtdiff;

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
//	morse_trap(222);
dwt1 = DTWTIME;

	adcommon.dmact += 1; // Running count
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct ADCDMATSKBLK* ptmp = &adc1dmatskblk[0];

	if( ptmp->adctaskHandle == NULL) return; // Skip task has not been created
	xTaskNotifyFromISR(ptmp->adctaskHandle, 
		ptmp->notebit1,	/* 'or' bit assigned to first half buffer to notification value. */
		eSetBits,      /* Set 'or' option */
		&xHigherPriorityTaskWoken ); 

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	return;
}
/* *************************************************************************
 * void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
 *	@brief	: Call back from stm32f4xx_hal_adc: End point of dma buffer
 * *************************************************************************/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
dwtdiff =  DTWTIME - dwt1;

	adcommon.dmact += 1; // Running count
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct ADCDMATSKBLK* ptmp = &adc1dmatskblk[0];

	if( ptmp->adctaskHandle == NULL) return; // Skip task has not been created
	xTaskNotifyFromISR(ptmp->adctaskHandle, 
		ptmp->notebit2,	/* 'or' bit assigned to last half DMA buffer to notification value. */
		eSetBits,      /* Set 'or' option */
		&xHigherPriorityTaskWoken ); 

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	return;
}
