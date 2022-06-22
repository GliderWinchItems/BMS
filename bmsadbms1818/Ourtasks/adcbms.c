/******************************************************************************
* File Name          : adcbms.c
* Board              : bms14921: STM32L431
* Date First Issued  : 02/17/2022
* Description        : ADC management with MAX1421 board
*******************************************************************************/

/*
'MX ADC assignments 06/02/2022
Port Descript Chan'l sample scan order
PC3	FETCUR-A2	IN4	      2.5	1
PA0	OA INP	    IN5	     24.5	2
PA3	OA OUT	    IN8	     24.5	3
PA7	Hv-div1	    IN12	640.5	4
PC4	Therm 1	    IN13	247.5	5
PC5	Therm2	    IN14	247.5	6
Vref		    IN0	    247.5	7
Vtemp		    IN17	247.5	8
PA4	dcdc_15v	IN9	    247.5	9
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
extern DMA_HandleTypeDef hdma_adc1;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

extern TIM_HandleTypeDef htim15;

uint32_t dbdma1;
uint32_t dbdma2;
uint32_t dbadc1;
uint32_t dbadc2;



/* Save Hardware registers setup with 'MX for later restoration. */
static uint32_t adc_smpr1;
static uint32_t adc_sqr1; 
static uint32_t adc_cfgr; 
static uint32_t adc_cfgr2;

/* *************************************************************************
 * void adcbms_config_bms(void);
 * @brief	: Set up for MAX14921 BMS cell readout
 * *************************************************************************/
 void adcspi_config(void)
 {
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	
	return;
 }
 
/* *************************************************************************
 * void adcbms_startreadbms(void);
 * @brief	: Set up and start a readout sequence
 * *************************************************************************/
 void adcbms_startreadbms(void)
 {
 	struct ADCSPIALL* p = &adcspiall; // Convenience pointer


//	SELECT_GPIO_Port->BSRR = SELECT_Pin<<16; // Set pin low
//	SELECT_GPIO_Port->BSRR = SELECT_Pin; // Set pin high
	
	/* Start transmission of command: set /SMPL bit high */
	// DMA_CNDTR: number of data to transfer register
//	hdma_spi1_rx.Instance->CCR &= ~1; // Disable channel
//	hdma_spi1_rx.Instance->CNDTR = 3; // Number to DMA transfer
//	hdma_spi1_rx.Instance->CCR |= 1;  // Enable channel

	hdma_spi1_tx.Instance->CCR &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel 	

	// Bit 6 SPE: SPI enable
	SPI1->CR1 |= (1 << 6);	

// Debug
//extern uint32_t dbspidma;	
//dbspidma = DTWTIME;	
//osDelay(1000*400);

	// Set time duration for SPI to send 24b command
//	TIM15->CCR1 = TIM15->CNT + SPIDELAY + 16; // Set SPI xmit duration

	// Set state for timer interrupt. (It will set a 50 us TIM2 delay to allow switch & settling.)
	p->timstate = TIMSTATE_SMPL; //

	return;
 }

 