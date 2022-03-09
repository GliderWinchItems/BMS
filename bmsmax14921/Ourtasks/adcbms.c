/******************************************************************************
* File Name          : adcbms.c
* Board              : bms14921: STM32L431
* Date First Issued  : 02/17/2022
* Description        : ADC management with MAX1421 board
*******************************************************************************/

/*
'MX ADC assignments 02/25/2022
Port Descript Chan'l sample scan order
PC1	FETCUR-rc	IN2	     47.5	1
PC3	FETCUR-A2	IN4	      2.5	2
PA0	OA INP	    IN5	     24.5	3
PA3	OA OUT	    IN8	     24.5	4
PA7	Hv-div1	    IN12	640.5	5
PC4	Therm 1	    IN13	247.5	6
PC5	Therm2	    IN14	247.5	7
Vref		    IN0	    247.5	8
Vtemp		    IN17	247.5	9
PA4	dcdc_15v	IN9	    247.5	10
PC0	BMS	        IN1 	 24.5	N/A
*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "adcbms.h"
#include "adcparams.h"
#include "ADCTask.h"
#include "main.h"
#include "DTW_counter.h"

#include "morse.h"

extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_adc1;


/* Save Hardware registers setup with 'MX for later restoration. */
static uint32_t adc_smpr1;
static uint32_t adc_sqr1; 
static uint32_t adc_cfgr; 
static uint32_t adc_cfgr2;

/* *************************************************************************
 * void adcbms_preinit(void);
 * @brief	: Save some things used later.
 * *************************************************************************/
 void adcbms_preinit(void)
{
	//struct ADCFUNCTION* p = &adc1; // Convenience pointer

	/* Change number of channels scanned to 10 (from 11) */
 	// MX & HAL initialization sets length of scan sequence 
 	// to 11 conversions but the 11th in the sequence is ADC_IN1 
 	// which is used for the BMS and not needed for this readout.
 	hadc1.Instance->SQR1 &= ~(0xF << 0); // Clear old value
 	hadc1.Instance->SQR1 |= (ADCDIRECTMAX << 0); // Insert new

	/* Save registers configured by 'MX for quick & easy restoration.
	   when reconfiguring from BMS to ADC scan mode.  */
 	adc_smpr1 = hadc1.Instance->SMPR1;
	adc_sqr1  = hadc1.Instance->SQR1;
	adc_cfgr  = hadc1.Instance->CFGR;
	adc_cfgr2 = hadc1.Instance->CFGR2;

	// Bit 0 DMAEN: Direct memory access enable. 1: DMA enabled
	// Bit 1 DMACFG: Direct memory access configuration 0: DMA One Shot mode selected
	adc_cfgr  |= 0x1;

	/* DMA peripheral address. */
	if ((hdma_adc1.Instance->CCR & 0x1) != 0) morse_trap(844);
	hdma_adc1.Instance->CPAR = (uint32_t)hadc1.Instance + 0x40;

	return;
}
/* *************************************************************************
 * void adcbms_config_adc(void);
 * @brief	: Set up for STM32L431 ADC readouts (excluding ADC used with MAX14921)
 * *************************************************************************/
 void adcbms_config_adc(void)
 {
 	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

 	/* Skip if already configured for ADC scan with DMA. */
 //	if (p->config == 0) return;

 	/* Here, either not initially configured or configured for BMS. */
 	p->config = 0; // Show it is configured for DMA.

	// Cannot set registers if ADSTART is not 0.
 	if ((hadc1.Instance->CR & (0x1 << 2)) != 0) morse_trap(850);

	/* ADC nterrupt */
	hadc1.Instance->IER &= ~0x7FF; // Clear all interrupt enables

	// Enable ADC
 	hadc1.Instance->CR &= ~0x3F;
// Debug: check bit config required to allow setting enable bit 	
if ((hadc1.Instance->CR & (0x1 << 28)) == 0) morse_trap(867);
//if ((hadc1.Instance->CR & 0x3F) != 0) morse_trap(866);

	hadc1.Instance->CR |= 0x1; // Enable

 	/* Restore registers to 'MX & HAL initialized settings, saved in
 	   'adcbms_preinit' (with SQR1 modified for 1 less channel in scan). */
	hadc1.Instance->SMPR1 = adc_smpr1;
	hadc1.Instance->SQR1  = adc_sqr1;
	hadc1.Instance->CFGR  = adc_cfgr;
	hadc1.Instance->CFGR2 = adc_cfgr2;

	/* DMA: Memory Address Register. */
	hdma_adc1.Instance->CMAR  =  (uint32_t)&adc1.dmabuf[0]; // DMA stores into this array

	/* DMA_CNDTR: number of data to transfer register */
	// VERY IMPORTANT: DMA count be identical to ADC scan count. When the ADC
	// scan ends, the last store by the DMA must generate an interrupt. If the
	// DMA count is not zero, then where will be no interrupt.
	hdma_adc1.Instance->CNDTR  =  ADCDIRECTMAX;	// 

	/* DMA1 CH1 interrupt. */
	// DMA channel x configuration register (DMA_CCRx) */
	// Bit 0 EN: channel enable | Bit 1 TCIE: transfer complete interrupt enable
	hdma_adc1.Instance->CCR |= 0x3;

	return;
 }
/* *************************************************************************
 * void adcbms_config_bms(void);
 * @brief	: Set up for MAX14921 BMS cell readout
 * *************************************************************************/
 void adcbms_config_bms(void)
 {
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	/* Setup ADC registers for cell readouts. */
 	// Cannot set registers if ADSTART is not 0.
 	if ((hadc1.Instance->CR & (0x1 << 2)) == 0) morse_trap(840);

  	/* Set oversample count */
 	// Set: oversample enabled | oversample count = 32 | right shift count = 1;
 	hadc1.Instance->CFGR2 = ((0x1 << 0) | (0x4 << 2) | (0x1 << 5)); 

 	// DMAEN: Direct memory access disable
 	// CONT:  Single continuous conversion mode for regular conversions
	hadc1.Instance->CFGR = 0x80000000; // I think this is all that is needed!

	// Enable ADC interrupt
	// Bit 2 EOCIE: End of regular conversion interrupt enable
	// Bit 0 ADRDYIE: ADC ready interrupt enable
	hadc1.Instance->ISR = 0x7FF; // Clear all ADC interrupt flags
	hadc1.Instance->IER = (1 << 2); // Enable the one we want

	/* Set number of sampling cycles for channel scanned first. */
	hadc1.Instance->SMPR1 &= ~(0X7 << 0); // Clear old value
	hadc1.Instance->SMPR1 |= ~(0X3 << 0); // 011: 24.5 ADC clock cycles

 	/* Scan sequence. */
 	// Set Length of scan sequence to 1 conversion. (code = 0)
 	//   Bits 3:0 L[3:0]: Regular channel sequence length
 	// Set SQ1 (1st in sequence) to ADC_IN1 (code = 1)
 	//   Bits 10:6 SQ1[4:0]: 1st conversion in regular sequence
 	hadc1.Instance->SQR1 = (1 << 6); 

	p->config = 1; // Show ADC is configured for BMS.
	
	return;
 }
 
/* *************************************************************************
 * void adcbms_startreadbms(void);
 * @brief	: Set up and start a readout sequence
 * *************************************************************************/
 // Setup SPI command ECS = 1; Cell 0; /SMPL = 1
	// Beware little endian of word!
 static const uint32_t spi24smpl = 0x00040000; // /SMPL bit set high

 void adcbms_startreadbms(void)
 {
 	HAL_StatusTypeDef halret;
 	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

 	p->adcflag = 0; // ADC is idle

 	p->adcidx = 0; 
 	p->spiidx = 0;

 	p->adcstate = ADCSTATE_IDLE;
 	p->timstate = TIMSTATE_IDLE;

 	p->adcrestart = 0; // ADC conversion start initiated by: 0 = TIM2; 1= ADC

 	// Save (for fast ISR use) 
 	p->updn = pssb->updn; //up/down cell readout order requested
 	p->noverlap = pssb->noverlap; // not overlap spi with adc

 	// Configure ADC registers for BMS readout
 	adcbms_config_bms();

 	// SPI interrupt will set a 50 us TIM2 delay to allow switch & settling.
	p->spistate = SPISTATE_SMPL; // 

	// uc[0] = 0; uc[1] = 0; uc[2] = 0x04 uc[3] = not used
	p->spitx24.ui = 0x00040000; // /SMPL bit set high

 	// Set /CS (GPIOA PIN_15) low to enable MAX14921.
	HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);

	// Start transmission of command: set /SMPL bit high
	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
	if (halret != HAL_OK) morse_trap(832);

	// SPI interrupt will continue sequence.
	// see: adcspi.c : HAL_SPI_MasterRxCpltCallback : SPISTATE_SMPL
	return;
 }

 