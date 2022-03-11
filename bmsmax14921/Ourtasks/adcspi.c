/******************************************************************************
* File Name          : adcspi.c
* Board              : bms14921: STM32L431
* Date First Issued  : 02/19/2022
* Description        : ADC w SPI management with MAX1421 board
*******************************************************************************/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "malloc.h"
#include "adcspi.h"
#include "adcparams.h"
#include "ADCTask.h"
#include "adcbms.h"
#include "main.h"
#include "FanTask.h"
#include "BQTask.h"
#include "DTW_counter.h"

#include "morse.h"

extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_adc1;

/* 24 bit control word sent to MAX14921 via SPI as 3 bytes
MAX   BYTE:BIT 
CB1  = [0]:7  Cell #1 BA: 1 = high; 0 = low 
CB2  = [0]:6
CB3  = [0]:5
CB4  = [0]:4
CB5  = [0]:3
CB6  = [0]:2
CB7  = [0]:1
CB8  = [0]:0
CB9  = [1]:7
CB10 = [1]:6
CB11 = [1]:5
CB12 = [1]:4
CB13 = [1]:3
CB14 = [1]:2
CB15 = [1]:1
CB16 = [1]:0 Cell #16 BA: 1 = high; 0 = low
ECS  = [2]:7 1 = cell select enable
SC0  = [2]:6 LSB cell selection number
SC1  = [2]:5 
SC2  = [2]:4 
SC3  = [2]:3 MSB cell selection number
/SMPL= [2]:2 1 = hold; 0 = sample if SMPL pin high
DIAG = [2]:1 1 = 10 ua leakage on CV inputs
LOPW = [2]:0 1 = Low power mode: Vp = 1 ua
*/

#define TIM15MHZ 16  // Timer rate MHz
#define DELAY8MS  (TIM15MHZ * 1000 * 8) // 8 millisecond TIM2 OC increment
#define DELAYUS    TIM15MHZ // 1 microsecond TIM2 OC increment

static uint32_t noteval1;
/* *************************************************************************
 * void adcspi_readadc(void);
 * @brief	: Read selected ADCs (non-MAX14921)
 * *************************************************************************/
uint32_t dbd;
uint32_t dbt1;
uint32_t dbt2;
uint32_t dbt3;
uint32_t dbt4;
uint32_t dbt5;

void adcspi_readadc(void)
{
	struct ADCFUNCTION* p; // Convenience pointer
	struct ADCABS* pabs; ; // Absolute readings
	uint16_t* pdmabuf;     // DMA buffer
	/* Configure ADC for reading non-MAX14921 ADC channels w DMA. */
	adcbms_config_adc(); 
if ((hadc1.Instance->CR & 0x1) == 0) morse_trap(667);
	/* Start a one-shot scan w DMA. */
	hadc1.Instance->CR |= (1 << 2); // Bit 2 ADSTART: ADC start of regular conversion

if ((hadc1.Instance->CR & (1 << 2))	== 0) morse_trap(827);

	p = &adc1; // Convenience pointer
	pabs = &p->abs[0]; // Absolute readings
	pdmabuf = &p->dmabuf[0];
dbd += 1;
dbt1 = DTWTIME;
	/* Wait for DMA interrupt to signal completion. */
	xTaskNotifyWait(0,0xffffffff, &noteval1, 5000);
	if (noteval1 != TSKNOTEBIT00) morse_trap(825); // JIC debug
dbt2 = DTWTIME;
dbt3 = dbt2-dbt1;

	/* Accumulate readings before doing computations */
	(pabs+0)->sum += *(pdmabuf + 0);
	(pabs+1)->sum += *(pdmabuf + 1);
	(pabs+2)->sum += *(pdmabuf + 2);
	(pabs+3)->sum += *(pdmabuf + 3);
	(pabs+4)->sum += *(pdmabuf + 4);
	(pabs+5)->sum += *(pdmabuf + 5);
	(pabs+6)->sum += *(pdmabuf + 6);
	(pabs+7)->sum += *(pdmabuf + 7);
	(pabs+8)->sum += *(pdmabuf + 8);
	(pabs+9)->sum += *(pdmabuf + 9);

	p->sumctr += 1;
	if (p->sumctr >= ADCSCANSUM)
	{ // Here, time to calibrate and update.
		p->sumctr = 0;

		/* Calibrate and update array of struct ADCABSOLUTE. */
		// The array is a global variable. Values highly filtered.
		adcparams_calibadc();
	}
	return;
}
/* *************************************************************************
 * void adcspi_readbms(void);
 * @brief	: Do a sequence for reading MA14921 plus direct ADC inputs
 * *************************************************************************/
void adcspi_readbms(void)
{
	int i;

	// Initialize and start read sequence
	adcbms_startreadbms();

	// Wait for sequence to complete
	// Once started, sequence driven by interrupts until complete
	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
	// Debugging trap for timeout (3 sec)
	if (noteval1 != TSKNOTEBIT00) morse_trap(814); // JIC debug

	/* Calibrate and store readings in requester's array (float). */
	for (i = 0; i < ADCBMSMAX; i++)
	{
		*(pssb->taskdata+i) = adcparams_calibbms(i);
	}
	return;
}
/* *************************************************************************
 * void adcspi_calib(void);
 * @brief	: Execute a self-calib sequence: nulls BMS output buffer offset.
 * *************************************************************************/
static const uint32_t spiclear = 0;
void adcspi_calib(void)
{
	HAL_StatusTypeDef halret;
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	
	/* All states start with zero. */
	p->adcstate = ADCSTATE_IDLE;
	p->timstate = TIMSTATE_IDLE;
	p->spistate = SPISTATE_CALIB1; // Self-calibration sequence before readouts

	/* Set MX14921 command to initiate self-calibration. */
	p->spitx24.ui = 0x00400000; // Initiate offset calibration upon /CS lo->hi transition

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */	
	HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);

	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
	if (halret == HAL_ERROR) morse_trap(817);
	/* SPI DMA interrupt callback routine (SPISTATE_CALIB) will cause /CS lo->hi 
	   transition AND set an 8 ms delay in TIM2CH4 OC. The OC interrupt will 
	   cause the following wait for notifcation to exit.   */
	xTaskNotifyWait(0,0xffffffff, &noteval1, 10000);

	if (noteval1 != TSKNOTEBIT00) morse_trap(813); // JIC debug

	/* Return will be to ADCTask.c and beginning of for (;;) loop waiting
	   for others to load a readout request into the queue. */
	return;
}
/* *************************************************************************
 * void adcspi_opencell(void);
 * @brief	: Check for open cell wires
 * *************************************************************************/
void adcspi_opencell(void)
{
	int i;
	HAL_StatusTypeDef halret;
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	// Load FET bits (lower two bytes)
	p->spitx24.ui    = (uint32_t)(pssb->taskdata);
	// Set command code
	p->spitx24.uc[2] = 0x2; // Set DIAG: 10 ua discharge on all cells.

	p->spistate = SPISTATE_OPENCELL;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
	if (halret == HAL_ERROR) morse_trap(816);

	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
	if (noteval1 != TSKNOTEBIT00) morse_trap(815); // JIC debug

	/* 10 ua cischarge complete. Start a readout of the cells. */
	adcspi_readbms();

	/* Wait for cellreadout to complete. */
	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
	if (noteval1 != TSKNOTEBIT00) morse_trap(812); // JIC debug

	/* Copy readings to requester's array, and
	   check for cells with very low voltages. */
	pssb->cellbits = 0;
	for (i = 0; i < 16; i++)
	{
		// Calibrate 'raw'
		*(pssb->taskdata+i) = adcparams_calibbms(i);
		if (*(pssb->taskdata+i) < bqfunction.lc.cellopenv)
		{
			pssb->cellbits |= (1 << i);
		}
	}
	return;
}
/* *************************************************************************
 * void adcspi_lowpower(void);
 * @brief	: Place MAX14921 into low power mode
 * *************************************************************************/
void adcspi_lowpower(void)
{
	HAL_StatusTypeDef halret;
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	// Clear bits
	p->spitx24.ui    = 0;
	// Set command code
	p->spitx24.uc[2] = 0x1; // Set LOPW: 1 ua

	p->spistate = SPISTATE_LOWPOWER;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
	if (halret == HAL_ERROR) morse_trap(824);

	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
	if (noteval1 != TSKNOTEBIT00) morse_trap(823); // JIC debug
}
/* *************************************************************************
 * void adcspi_setfets(void);
 * @brief	: Read selected ADCs (non-MAX14921)
 * *************************************************************************/
/* 
Command 
Bits set to one turn on discharge FETs
SPI sends bytes MSB
.us[0] = 16 bit word with bits arranged for sending
 bit 0 = cell 8
 ...
 bit 7 = cell 1
 bit 8 = cell 16
 ...
 bit 15 = cell 9

 .uc[2] = byte with command codes
*/
void adcspi_setfets(void)
{
	HAL_StatusTypeDef halret;
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	// Load FET bits (lower two bytes)
	p->spitx24.ui = pssb->cellbits;
	// Set command code
	p->spitx24.uc[2] = 0;

	p->spistate = SPISTATE_FETS;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
	if (halret == HAL_ERROR) morse_trap(818);
	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
	if (noteval1 != TSKNOTEBIT00) morse_trap(815); // JIC debug

	return;
}
/* *************************************************************************
 * void adcspi_init(void);
 * @brief	: 
 * *************************************************************************/
void adcspi_init(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	/* SMPL pin via i/o port inverter set hi. GPIOB PIN_7. */
	HAL_GPIO_WritePin(SELECT_GPIO_Port,SELECT_Pin,GPIO_PIN_SET);

	/* All states start with zero. */
	p->adcstate = ADCSTATE_IDLE;
	p->timstate = TIMSTATE_IDLE;
	p->spistate = SPISTATE_CALIB1; // Self-calibration sequence before readouts

	p->delayct = (DELAYUS * 5); // 5 us delay

	return;
}
/* #######################################################################
 * void adcspi_tim15_IRQHandler(void);
 * @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
// ECS plus cell selection bits for cells 1-> 16, given indices [0]->[15]
static const uint8_t bitorderUP[16] = {0x84,0XC4,0XA4,0XE4,0X94,0XD4,0XB4,0XE4,
	                                   0X8C,0XCC,0XAC,0XEC,0X9C,0XDC,0XBC,0XFC};
// ECS plus cell selection bits for cells 16-> 1, given indices [0]->[15]
static const uint8_t bitorderDN[16] = {0xFC,0XBC,0XDC,0X9C,0XEC,0XAC,0XCC,0X8C,
	                                   0XF4,0XB4,0XD4,0X94,0XE4,0XA4,0XC4,0X84};
static const uint32_t spicalib2 = 0;

void adcspi_tim15_IRQHandler(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;
	HAL_StatusTypeDef halret;

/* Debug: only this flag should have brought us here. */
if ((TIM15->SR & (1 << 1)) == 0) morse_trap(831);

	/* Clear interrupt flag. */
	TIM15->SR = ~(1 << 1); // Bit 1 CC1IF: Capture/Compare 1 interrupt flag

	if (p->spiidx >= ADCBMSMAX)
	{ // Done with SPI sending commands & TIM2 delay for settling 
		p->timstate = TIMSTATE_IDLE; 
		return;
	}

	switch (p->timstate)
	{
	case TIMSTATE_IDLE: // Idle. Ignore interrupt
 	 return;

	case TIMSTATE_8MSTO: // 8 ms timeout w 16b timer
		// Count rollover timeouts (65536/16MHz = 4096 us)
		p->tim15ctr -= 1;
		if (p->tim15ctr > 0) return;
		// Here, slightly more than 8 milliseconds.
		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify ADCTask.c that self-calibration is complete.
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );
	 return;

	case TIMSTATE_2: // Here, selection command timeout complete
p->timstate = TIMSTATE_IDLE; // Set to idle, jic

		/* Prepare next MAX14921 command to select a cell for readout. */
		// Use spi index to select MAX14921 command for cell selection readout
		switch (p->spiidx)
		{ 
		// Select cells 			
		case  0: case  1: case  2: case  3:
		case  4: case  5: case  6: case  7:
		case  8: case  9: case 10: case 11:
		case 12: case 13: case 14: case 15:
			// Cell select: ECS = 1, SC0,1,2,3 selects cell number
			// Look up cell selection number since bit order in command is reversed.
			// Allow for comparing up and down readout sequence results
			if (p->updn == 0)
			{ // Preferred seq: read highest cells first
				p->spitx24.uc[2] = bitorderDN[p->spiidx];
			}
			else
			{ // Use to compare slump: read lowest cells first
				p->spitx24.uc[2] = bitorderUP[p->spiidx];
			}
			break;

		// Select Thermistor inputs and top-of-stack divider
		case 16: // Select T1
			p->spitx24.uc[2] = 0X4C;
			break;
		case 17: // Select T2
			p->spitx24.uc[2] = 0X2C;
			break;
		case 18: // Select T3
			p->spitx24.uc[2] = 0X6C;
			break;
		case 19: // Select top-of-stack divider		
			p->spitx24.uc[2] = 0X6C;
			// Top-of-stack requires 60 us settling time.
			p->delayct = (DELAYUS * 60); 
			break;

		// End of sending SPS commands for BMS readouts
		// ADC completion ends full BMS readout sequence
		case 20:
			/* Next: reconfigure ADC and readout non-BMS ADC inputs
			   after ADC has finished converting the top-of-stack
			   (and last) selection. */
			if (p->adcrestart == 0)
			{ /* Here, ADC was not busy when SPI completed sending the command. SPI interrupt 
	 			raised /CR to latch the command and started TIM2 timeout for settling.
	 			The TIM2 timeout ended up here and the index shows the last command was sent
	 			and the settling time completed. Start the last ADC conversion (for this
	 			MAX14921 cell, thermistor, and top-of-stack readout sequence). */
				if (p->adcflag != 0) morse_trap (805);
				p->adcflag = 1; // Show ADC started

				// Setting ADSTART starts regular conversion
				hadc1.Instance->CR |= (0x1 << 2);

				p->spistate= SPISTATE_IDLE; //JIC
			}
			break;

		default: morse_trap(801);
			break;
		}
	/***** TIMSTATE2 continues *****/
		// First SPI cell select command does not overlap ADC conversions. No need to
		// check if ADC is still busy with previous conversion.
		if (p->spiidx == 0)
		{ // Ground reference time delay completed, send first cell selection command
			p->spistate = SPISTATE_2;

			if (p->noverlap != 0)
			{ // Here, do not overlap SPI sending with ADC conversion
				// Enable /CS and start SPI sending first cell selection
				HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
				halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
				if (halret != HAL_OK) morse_trap (826);
			}
			else
			{ // Here, overlap sending SPI with ADC conversion
				// Setting ADSTART starts regular conversion
				p->adcflag = 1; // Show ADC started
				hadc1.Instance->CR |= (0x1 << 2); // Start ADC
			}
			// Next TIM2 will prepare command to select cell #2.
			p->spiidx = 1;
			break; // Break TIMESTATE_2
		}

		// Prepare and start next SPI selection command
		p->spiidx += 1; // Index is "one ahead"

		if (p->noverlap == 1)
		{ // Here, do not overlap SPI sending with ADC conversions
			// Setting ADSTART starts regular conversion
			p->adcflag = 1; // Show ADC started
			hadc1.Instance->CR |= (0x1 << 2); // Start ADC
		}
		else if (p->adcrestart == 0)
		{ /* Here, ADC was not busy when SPI completed sending the command. SPI interrupt 
		 	raised /CR to latch the command and started TIM2 timeout for settling.
		 	The TIM2 timeout ended up here. */
	if (p->adcflag != 0) morse_trap (804); // Debug

			// Setting ADSTART starts regular conversion
			p->adcflag = 1; // Show ADC started
			hadc1.Instance->CR |= (0x1 << 2); // Start ADC

			p->spistate= SPISTATE_2; // Necessary?

			// Enable: /CS low, then start SPI sending command
			HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
			halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
			if (halret != HAL_OK) morse_trap (828);
		}
		break; // Break TIMESTATE_2
	/***** TIMSTATE_2 ENDS ******/

	case TIMSTATE_OPENCELL: // Wait for DIAG to discharge completed
		// Start a BMS readout cycle 
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );
		break;

	default: morse_trap(809); // Debug JIC
		break;

	}
	return;
}
/* #######################################################################
 * SPI DMA RX complete callback
 * RX lags TX slightly, so RX is used
   ####################################################################### */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;
	HAL_StatusTypeDef halret;

	switch (p->spistate)
	{
	case SPISTATE_CALIB1: // Command to clear registers complete
		// Set /CS high to latch the command
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);

		/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
		halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
if (halret != HAL_OK)	morse_trap(831);
		p->spistate = SPISTATE_CALIB2;
		break;

	case SPISTATE_CALIB2: // Self-calibration command has been sent.
		// Setting /CS high initiates self-calibration
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);
		// Set timout delay for calibration to approx 8 ms.
		p->tim15ctr    = 2; // Count TIM15 OC rollovers (4096 us per rollover)
		TIM15->CCR1 = TIM15->CNT; // Set for max OC count from "now"
		TIM15->SR   &= ~0x02; // Reset TIM15CH1 interrupt flag if CC1IF had come on 
		p->timstate    = TIMSTATE_8MSTO; // Set state for next timer interrupt 
		break;

	case SPISTATE_SMPL: // Command to switch flying caps to ground has been sent.
		/* There is a 50 us timeout for it to settle. After this the first selection
			command will be sent. */
		// Raise /CS to latch command bits and begin ground reference transfer
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);

		// Set time delay for settling
		TIM15->CCR1 = TIM15->CNT + (DELAYUS * 50); // Set 50 us timeout duration
		p->timstate = TIMSTATE_2; // Set timer interrupt handling.
		TIM15->SR = ~(1 << 1); // Reset TIM15CH1:OC interrupt flag: CC1IF if it had come on.
		break;

	case SPISTATE_2: // Command has been sent
		/* If ADC is busy, then do not latch the command into the MAX14921 registers. 

		   Set adcflag so that the ADC completion interrupt will raise /CR to latch 
		the selection, and set a settling time timeout. 

		   The TIM2 timeout interrupt will then start the ADC conversions, and launch the
		 next SPI command, unless it is a non-overlapped request, in which case, the
		 ADC interrupt will start the next SPI.
		*/
		if (p->adcflag != 0)
		{ // Here, ADC coversion is not complete
			// ADC will start next spi command
			p->adcrestart = 1;
			break;
		}
		// 'else' Latch selection command and start time delay
		// Raise /CR to cause command values just sent to latch
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);
		// Set a 5 us time delay for MAX14921 mux switch and Aout settling
		TIM15->CCR1 = TIM15->CNT + (DELAYUS * 5); // Set timeout duration
//		TIM15->SR = ~(1 << 1); // Reset TIM15CH1:OC interrupt flag: CC1IF if had come on.
		p->timstate = TIMSTATE_2; // Set timer timeout state
		p->adcrestart = 0; // JIC
		break;

	case SPISTATE_FETS: // Set discharge FETs
		// Cause command values to latch
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);
		// Notify 'adcspi_setfets' FET word has been sent and latched.
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );	
		portYIELD_FROM_ISR( xHPT);		
		return;

	case SPISTATE_OPENCELL:
		// Cause command values to latch
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);

		// Set time delay for DIAG 10ua current to discharge flying caps.
		TIM15->CCR1 = TIM15->CNT + (DELAYUS * 1000 * 500); // Set 500 ms timeout duration
		p->timstate = TIMSTATE_OPENCELL; // Set timer interrupt handling.
		TIM15->SR = ~(1 << 1); // Reset TIM15CH1:OC interrupt flag: CC1IF if had come on.
		break;

// NOTE: This is the same as SPISTATE_FETS...
	case SPISTATE_LOWPOWER:
		// Cause command values to latch
		HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);
		// Notify 'adcspi_lowpower' command complete
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );	
		portYIELD_FROM_ISR( xHPT );
		break;			

	case SPISTATE_IDLE:
		break;

	case SPISTATE_TRAP: // Trap unexpected interrupts
		morse_trap(822);
		break;

default: morse_trap(810); 
	break; // Debug jic
	}
	return;
}
/* #######################################################################
 * ADC interrupt (from stm32l4xx_it.c)
 * void adcspi_adc_IRQHandler(ADC_HandleTypeDef* phadc1);
   ####################################################################### */
void adcspi_adc_IRQHandler(ADC_HandleTypeDef* phadc1)
{
	ADC_TypeDef* pADCbase = hadc1.Instance; // Convenience pointer
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;
	HAL_StatusTypeDef halret;

	p->adcflag = 0; // Show ADC complete

	// ISR Bit 2 EOC: End of conversion flag
	if ((pADCbase->ISR & (1 << 2)) != 0) // Bit 2 EOC: End of conversion flag
	{ // Here, conversion complete
		pADCbase->ISR = (1 << 2); // Reset interrupt flag

		p->raw[p->adcidx] = pADCbase->DR; // Store data
		p->adcidx += 1; 
		if (p->adcidx >= ADCBMSMAX)
		{ // Here end of 16 cells, 3 thermistors, 1 top-of-stack
			xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
			portYIELD_FROM_ISR( xHPT );
		}
		else
		{ // More readings to be taken.
			if (p->adcrestart != 0)
			{ /* SPI command was sent, but the command was not latched when the following
			    TIM2 timeout took place because the ADC was busy. TIM2 interrupt set adcrestart
			    so that the ADC completion will latch the command bits and set the TIM2 
			    timeout delay for the mux and Aout to settle.
			    The end of the TIM2 timeout will start the ADC, and start the next
			    SPI command sending (if there are more readings to be taken). */

			    p->adcrestart = 0; // Is this needed?

				// Latch command by raising /CR
				HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_SET);

				// Set settling time for MUX & Aout
				TIM15->CCR1 = TIM15->CNT + p->delayct; // Set 5 us (usually) timeout duration
				p->timstate = TIMSTATE_2;
			}
			else
			{
				if (p->noverlap == 1)
				{ // Here, end of ADC (and not TIM2) starts next spi
					// Enable: /CS low, then start SPI sending command
					HAL_GPIO_WritePin(notCS_GPIO_Port,notCS_Pin,GPIO_PIN_RESET);
					halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
					if (halret != HAL_OK) morse_trap (828);
					p->spistate = SPISTATE_2;					
				}
			}
		}
	}
	return;
}
/* #######################################################################
 * DMA CH1 interrupt (from stm32l4xx_it.c)
 * void  adcspi_dma_handler(void);
   ####################################################################### */
 /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */
void  adcspi_dma_handler(void)
{
dbt4 = DTWTIME;
dbt5 = dbt4-dbt1;
	BaseType_t xHPT = pdFALSE;

	/* Clear DMA Channel 1 interrupt. */

/*	Ref Manual: 11.6.2

	Bit 3 CTEIF1: transfer error flag clear for channel 1
		Bit 2 CHTIF1: half transfer flag clear for channel 1
		Bit 1 CTCIF1: transfer complete flag clear for channel 1
		Bit 0 CGIF1: global interrupt flag clear for channel 1

Setting any individual clear bit among CTEIFx, CHTIFx, CTCIFx in this DMA_IFCR register,
causes the DMA hardware to clear the corresponding individual flag and the global flag
GIFx in the DMA_ISR register, provided that none of the two other individual flags is set.
*/
	hdma_adc1.DmaBaseAddress->IFCR |= 0xf;

	/* Disable DMA1:CH1. */
	hdma_adc1.Instance->CCR &= ~0x1;	

	/* Notify ADCTask.c: adcspi.c: adcspi_readadc operation is complete. */
	xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
	portYIELD_FROM_ISR( xHPT );
//morse_trap(668);	

	return;
}