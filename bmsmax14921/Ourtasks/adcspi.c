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
extern DMA_HandleTypeDef hdma_adc1;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

// Debuggers
uint32_t dbadcspit1;
uint32_t dbadcspit2;
uint32_t dbadcspit3;
uint8_t dbisrflag;

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



static uint32_t noteval1;
uint8_t readbmsflag; // Let main know a BMS reading was made
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
uint32_t dbbmst[21];

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

	/* Wait for DMA interrupt to signal completion. */
	xTaskNotifyWait(0,0xffffffff, &noteval1, 4000);
	if (noteval1 != TSKNOTEBIT00) morse_trap(825); // JIC debug

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
uint32_t dbbms1; // Debug: check timing
uint32_t dbbms2;
uint32_t dbbms3;

void adcspi_readbms(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	int i;

	// Initialize and start read sequence
	adcbms_startreadbms();

dbadcspit1 = DTWTIME; 

	// Wait for sequence to complete
	// Once started, sequence driven by interrupts until complete
	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);

// Total time (cells, thermistors, top-of-stack)
dbadcspit3 = DTWTIME - dbadcspit1;

	// Debugging trap for timeout (3 sec) (interrupt sequence hangs)
	if (noteval1 != TSKNOTEBIT00) morse_trap(814); // JIC debug

	/* Calibrate and store readings in requester's array (float). */
	// Adjust cells 1-16 order to account for up or down readout. */
	for (i = 0; i < 16; i++)
	{
		// Readout "up" (cells 1->16) = 1; Down (cells 16->1) = 0
		if (p->updn == 0)
		{ // Here, cell number readout sequence was 16->1
			*(pssb->taskdata+i) = adcparams_calibbms(15-i);
		}
		else
		{ // Here, cell sequence readout was 1->16
			*(pssb->taskdata+i) = adcparams_calibbms(i);
		}
	}
	// Thermistors and top-of-stack
	for (i = 16; i < ADCBMSMAX; i++)
	{
			*(pssb->taskdata+i) = adcparams_calibbms(i);
	}

	/* Signal 'main.c' that there is a bms reading. */
	readbmsflag = 1;
	// Upon this return ADCTask will notify requester.
	return;
}
/* *************************************************************************
 * void adcspi_calib(void);
 * @brief	: Execute a self-calib sequence: nulls BMS output buffer offset.
 * *************************************************************************/
static const uint32_t spiclear = 0;
void adcspi_calib(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	
	/* Set MX14921 command to initiate self-calibration. */
	p->spitx24.ui = 0x00400000; // Initiate offset calibration upon /CS lo->hi transition

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */	
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low

	/* Loading starts transfer. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel

	/* Set time delay for SPI to send 3 bytes. */
	TIM15->CCR1 = TIM15->CNT + SPIDELAY; // Set SPI xmit duration	

	p->timstate = SPISTATE_CALIB1; // Self-calibration sequence before readouts

	/* SPI DMA interrupt callback routine (SPISTATE_CALIB) will cause /CS lo->hi 
	   transition AND set an 8 ms delay in TIM15CH1 OC. The OC interrupt will 
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
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	// Load FET bits (lower two bytes)
	p->spitx24.ui    = (uint32_t)(pssb->taskdata);
	// Set command code
	p->spitx24.uc[2] = 0x2; // Set DIAG: 10 ua discharge on all cells.

	p->timstate = SPISTATE_OPENCELL;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low
	/* Loading starts transfer. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel

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
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	// Clear bits
	p->spitx24.ui    = 0;
	// Set command code
	p->spitx24.uc[2] = 0x1; // Set LOPW: 1 ua

	p->timstate = SPISTATE_LOWPOWER;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low

	/* Loading starts transfer. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel

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
SPI sends bytes MSB. MAX14921 expects LSB of the 
24b command first.
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
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer

	// Load FET bits (lower two bytes) from requester's struct
	p->spitx24.us[0] = pssb->cellbits;
	// Set command code
	p->spitx24.uc[2] = 0;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low

	/* Loading starts transfer. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel

	/* Set time delay for SPI to send 3 bytes. */
	TIM15->CCR1 = TIM15->CNT + SPIDELAY; // Set SPI xmit duration	

	p->timstate = SPISTATE_FETS;

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
	p->timstate = TIMSTATE_IDLE;
	return;
}
/* #######################################################################
 * void adcspi_tim15_IRQHandler(void);
 * @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
// In the following two lookup tables, the last item, bitorderxx[20],
//   clears the /SMPL bit which returns the flying caps to the cell inputs.
// ECS plus cell selection bits for cells 1-> 16,
// selection bits for T1, T2, T3, TOS, ZERO, given indices [0]->[20] and
static const uint8_t bitorderUP[21] = {0x84,0XC4,0XA4,0XE4,0X94,0XD4,0XB4,0XE4,
	                                   0X8C,0XCC,0XAC,0XEC,0X9C,0XDC,0XBC,0XFC,
	                                   0x58,0x38,0x78,0x78, 0x00};
// ECS plus cell selection bits for cells 16-> 1,
// selection bits for T1, T2, T3, TOS, ZERO, given indices [0]->[20]
static const uint8_t bitorderDN[21] = {0xFC,0XBC,0XDC,0X9C,0XEC,0XAC,0XCC,0X8C,
	                                   0XF4,0XB4,0XD4,0X94,0XE4,0XA4,0XC4,0X84,
	                                   0x58,0x38,0x78,0x78, 0x00};
uint32_t dbisr15;
uint32_t dbisr1;
uint32_t dbdier1;
uint32_t dbtrap;

void adcspi_tim15_IRQHandler(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;

	/* Clear interrupt flag. */
	TIM15->SR = ~(1 << 1); // Bit 1 CC1IF: Capture/Compare 1 interrupt flag

	switch (p->timstate)
	{
	case TIMSTATE_IDLE: // Idle. Ignore interrupt
 	 return;

	case TIMSTATE_SMPL: // Command to switch flying caps to ground has been sent.
		// Raise /CS to latch command bits and begin ground reference transfer
		// and set a 50 us settline time delay.
dbbms2 = DTWTIME;
dbbms3 = dbbms2 - dbbms1;	

		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high to latch bits

dbt1 = DTWTIME;	
	
		// Set time delay for settling
		TIM15->CCR1 = TIM15->CNT + (DELAYUS * 50); // Set 50 us timeout duration
		p->timstate = TIMSTATE_2; // Set next timer interrupt routing
		break;	 

	case TIMSTATE_2: // SMPL settling time complete. Send first selection command
				// Cell select: ECS = 1, SC0,1,2,3 selects cell number

		// Allow for selecting up or down readout sequence (to check "slump")
		if (p->updn == 0)
		{ // Preferred seq: read highest cells first
			p->spitx24.uc[2] = 0xFC; // bitorderDN[0];
		}
		else
		{ // Use to compare slump: read lowest cells first
			p->spitx24.uc[2] = 0x84; // bitorderUP[0];
		}		
		/* Start SPI sending command for selecting cell #1 (spiidx = 0) */
		notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low

		/* Loading starts SPI transfer. */
		hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
		hdma_spi1_tx.Instance->CNDTR = 3;  // Set number to DMA transfer
		hdma_spi1_tx.Instance->CCR  |= 1;  // Enable channel

		/* Set time delay for SPI to send 3 bytes. */
		TIM15->CCR1 = TIM15->CNT + SPIDELAY; // Set SPI xmit duration

		// Next TIMSTATE_N interrupt will prepare command to select cell #2.
		p->spiidx = 1; // Index for cell #2
		p->timstate = TIMSTATE_3;
		break;

	case TIMSTATE_3: // First cell selection sent. Set settling duration
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high to latch bits

		TIM15->CCR1 = TIM15->CNT + (DELAYUS * 5);
		p->timstate = TIMSTATE_N;
dbbmst[p->adcidx] = DTWTIME;
		break;

	case TIMSTATE_N: // Here, selection command timeout complete
		/* Start ADC converting cell selection ready for readout */
dbbmst[p->adcidx] = DTWTIME;	
		hadc1.Instance->CR |= (0x1 << 2);
	
		notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low

		/* Prepare next selection command. */
		// Allow for selecting up or down readout sequence (to check "slump")
		if (p->updn == 0)
		{ // Preferred seq: read highest cells first
			p->spitx24.uc[2] = bitorderDN[p->spiidx];
		}
		else
		{ // Use to compare slump: read lowest cells first
			p->spitx24.uc[2] = bitorderUP[p->spiidx];
		}			

		/* SPI sends next command while ADC does conversions on latched command. */
		// 
		/* Loading starts SPI transfer. */
		hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
		hdma_spi1_tx.Instance->CNDTR = 3;  // Set number to DMA transfer
		hdma_spi1_tx.Instance->CCR  |= 1;  // Enable channel		

		/* End of sequence command restores discharge FETs, if applicable. */
		// 0 = clear fets; fets remain clear after read sequence
		// 1 = fet setting remains in place during read sequence
		// 2 = new fet setting (from cellbits) applied after read sequence
		// 3 = save fet setting; clear for read; restore after read
		if ((p->readbmsfets > 1) && (p->spiidx == 20))
		{
			p->spitx24.us[0] = p->cellbitssave;
		}

		p->spiidx += 1; // Index to select cells (and others) is "one ahead"

		// ADC interrupt will start next SPI and TIM
		break;

	case TIMSTATE_OPENCELL: // Wait for DIAG to discharge completed
		// Start a BMS readout cycle 
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );
		break;

	case SPISTATE_CALIB1: // Command to clear registers complete

		// Set /CS high to latch the command
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high

		// Set timout delay for calibration to approx 8 ms.
		p->tim15ctr    = 2; // Count TIM15 OC rollovers (4096 us per rollover)
		TIM15->CCR1    = TIM15->CNT; // Set for max OC count from "now"
		TIM15->SR     &= ~0x02; // Reset TIM15CH1 interrupt flag if CC1IF had come on 
		p->timstate    = TIMSTATE_8MSTO; // Set state for next timer interrupt 		
		break;

	case TIMSTATE_8MSTO: // 8 ms timeout w 16b timer
		// Count rollover timeouts (65536/16MHz = 4096 us)
		p->tim15ctr -= 1;
		if (p->tim15ctr > 0) 
			return;

		notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS set low
		p->spitx24.ui = 0; // Clear BMS register

		// Here, slightly more than 8 milliseconds.
		/* Loading starts SPI transfer. */
		hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
		hdma_spi1_tx.Instance->CNDTR = 3;  // Set number to DMA transfer
		hdma_spi1_tx.Instance->CCR  |= 1;  // Enable channel

		TIM15->CCR1 = TIM15->CNT + SPIDELAY; // Set SPI xmit duration
		p->timstate = SPISTATE_CALIB2;
		break;		

	case SPISTATE_CALIB2: // Self-calibration command has been sent.
		// Setting /CS high initiates self-calibration
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high

		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify ADCTask.c that self-calibration is complete.
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );		
		break;

	case SPISTATE_FETS: // Set discharge FETs
		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Cause command values to latch
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high
		// Notify 'adcspi_setfets' FET word has been sent and latched.
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );	
		portYIELD_FROM_ISR( xHPT);		
		break;

	case SPISTATE_OPENCELL:
		// Raise /CR to cause command values just sent to latch
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high

		// Set time delay for DIAG 10ua current to discharge flying caps.
		TIM15->CCR1 = TIM15->CNT + (DELAYUS * 1000 * 500); // Set 500 ms timeout duration
		p->timstate = TIMSTATE_OPENCELL; // Set timer interrupt handling.
		TIM15->SR = ~(1 << 1); // Reset TIM15CH1:OC interrupt flag: CC1IF if had come on.
		break;

// NOTE: This is the same as SPISTATE_FETS...
	case SPISTATE_LOWPOWER:
		// Cause command values to latch
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set: /CS set high
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

	/* SPI sent command long ago. Set: /CS set high to latch bits. */
	notCS_GPIO_Port->BSRR = notCS_Pin; // 

	// ISR: Bit 2 EOC: End of conversion flag
	if ((pADCbase->ISR & (1 << 2)) != 0) // Bit 2 EOC: End of conversion flag
	{ // Here, conversion complete
		pADCbase->ISR = (1 << 2); // Reset interrupt flag

		p->raw[p->adcidx] = pADCbase->DR; // Store data

dbbmst[p->adcidx] = DTWTIME - dbbmst[p->adcidx];	

		p->adcidx += 1; 

#ifdef FIRSTSIXTEEN
if (p->adcidx == 15)
{
	dbadcspit3 = DTWTIME - dbadcspit1;
}
#endif
		
		if (p->adcidx >= ADCBMSMAX)
		{ // Here end of 16 cells, 3 thermistors, 1 top-of-stack			
			xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
			portYIELD_FROM_ISR( xHPT );
			p->timstate = TIMSTATE_IDLE;
		}
		else
		{ // More readings to be taken.
			p->timstate = TIMSTATE_N; // Many a mile to go.

			if (p->spiidx == 19) 
			{ // Top of stack selection requires longer settling time
				TIM15->CCR1 = TIM15->CNT + (DELAYUS * 60); // The end is near!
			}
			else
			{ // Cell and thermistors selection settling time
				// 16 results in 90 machine cycles for settling
				TIM15->CCR1 = TIM15->CNT + 16;//(DELAYUS * 5); // More miles to travel.
			}
		}
	}
	else
		morse_trap(736); // JIC debug
	return;
}
/* #######################################################################
 * DMA CH1 interrupt (from stm32l4xx_it.c)
 * void  adcspi_dma_handler(void);
 * non-BMS ADC channel scan completion interrupt
   ####################################################################### */
 /* DMA1_Channel1_IRQn 0 */
void  adcspi_dma_handler(void)
{
	// Bit 4 ADSTP: ADC stop of regular conversion command
	hadc1.Instance->CR |= (1 << 4); 

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
	hdma_adc1.DmaBaseAddress->IFCR = 0xf;

	/* Disable DMA1:CH1. */
	hdma_adc1.Instance->CCR &= ~0x1;	
 	hdma_adc1.Instance->CCR &= ~0x2;

	/* Notify ADCTask.c: adcspi.c: adcspi_readadc operation is complete. */
	xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
	portYIELD_FROM_ISR( xHPT );
//morse_trap(668);	

	return;
}
/* #######################################################################
 * SPI tx & rs dma transfer complete (if enabled!)
   ####################################################################### */
void adcspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx)
{
	morse_trap(841); // Debug JIC}
}

void adcspi_spidmatx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_tx)
{
	morse_trap(842); // Debug JIC}

	/* Clear the transfer complete flag */
	// Bit 8 CGIF3: global interrupt flag clear for channel 3
    phdma_spi1_tx->DmaBaseAddress->IFCR = (0xF << 8);
    phdma_spi1_tx->Instance->CCR &= ~0x1; // Disable channel
}
