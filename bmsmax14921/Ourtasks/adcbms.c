/******************************************************************************
* File Name          : adcbms.c
* Board              : bms14921: STM32L431
* Date First Issued  : 02/17/2022
* Description        : ADC management with MAX1421 board
*******************************************************************************/

/*
				   ADC clock	Oversample  Delay	
SMP	ADC	   Fixed Total	MHz 	us	64	    5	   16 cells
0	2.5	    12.5	15	80	0.1875	12.0	17.0	272.0
1	6.5	    12.5	19	80	0.2375	15.2	20.2	323.2
2	12.5	12.5	25	80	0.3125	20.0	25.0	400.0
3	24.5	12.5	37	80	0.4625	29.6	34.6	553.6
4	47.5	12.5	60	80	0.7500	48.0	53.0	848.0
5	92.5	12.5	105	80	1.3125	84.0	89.0	1424.0
6	247.5	12.5	260	80	3.2500	208.0	213.0	3408.0
7	640.5	12.5	653	80	8.1625	522.4	527.4	8438.4

*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "adcbms.h"
#include "adcparams.h"
#include "adcfastsum16.h"
#include "ADCTask.h"

#include "morse.h"

extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;
extern uint32_t adc_smpr1; // Saved HAL settings of ADC_SMPR1 ADC register.



static uint32_t adc_smpr1; // Save HAL settings of register.
static uint32_t adc_sqr1;  // Save HAL settings of register.

/* Parameters used for readout sequence. */
static struct ADCREADOUTCTL adcctl;

/* *************************************************************************
 * void adcbms_config_bms(void);
 * @brief	: Set up for MAX14921 BMS cell readout
 * *************************************************************************/
 void adcbms_config_bms(void)
 {
	/* Setup ADC registers for cell readouts. */
 	// Cannot set registers if ADSTART is not 0.
 	if (hadc1.Instance->CR & (0x1 << 2) == 0) morse_trap(400);

  	/* Set oversample count */
 	hadc1.Instance->CFGR2 = 0; // Clear previous options
 	// oversample enabled; oversample count = 32; right shift count = 1;
 	hadc1.Instance->CFGR2 |= ((0x1 << 0) | (0x4 << 2) | (0x1 << 5)); 

 	// DMAEN: Direct memory access disable
 	// CONT:  Single continuous conversion mode for regular conversions
//??	hadc1.Instance->CFGR & = ~((0x1 << 1) | (0x1 << 13)); // Clear: DMAEN: Direct memory access enable
	hadc1.Instance->CFGR = 0x80000000; // I think this is all that is needed!

	// Enable ADC interrupt
	// Bit 2 EOCIE: End of regular conversion interrupt enable
	// Bit 0 ADRDYIE: ADC ready interrupt enable
	hadc1.Instance->ISR = 0x7FF; // Clear all interrupt flags
	hadc1.Instance->IER = (1 << 2); // Enable the one we want

 	// Set number for first ADC channel in scan sequence
 	// Set Length of scan sequence to 1 conversion
 	// Set SQ1 (MX rank 1 channel) to ADC_IN1 (BMS ADC: MX rank 10)
 	hadc1.Instance->SQR1 ~= ~((0xF << 0) | (0xF << 6)); 
 	// First channel in sequence that MX originally set as rank 10
	hadc1.Instance->SQR1 |= (hadc1.Instance->SQR3 & 0XF); 

	
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
 	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

 	p->adcflag = 0; // ADC is idle

 	p->adcidx = 0; 
 	p->spiidx = 0;

 	p->adcstate = ADCSTATE_IDLE;
 	p->timstate = TIMSTATE_IDLE;

 	p->adcrestart = 0; // ADC conversion start initiated by: 0 = TIM2; 1= ADC

 	// Save (for fast ISR use) up/down cell readout order requested
 	p->updn = pssb->updn;

 	// Configure ADC registers for BMS readout
 	adcbms_config_bms();

 	// SPI interrupt will set a 50 us TIM2 delay to allow switch & settling.
	p->spistate = SPISTATE_SMPL; // 

	// uc[0] = 0; uc[1] = 0; uc[2] = 0x84 uc[3] = not used
	p->spitx24.ui = 0x00040000; // /SMPL bit set high


 	// Set /CS (GPIOA PIN_15) low to enable MAX14921.
	HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_RESET);

	// Start transmission of command: set /SMPL bit high
	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &spi24smpl, &p->spirx24.uc[0], 3);

	// SPI interrupt will continue sequence.
	// see: adcspi.c : HAL_SPI_MasterRxCpltCallback : SPISTATE_SMPL
	return;
 }
