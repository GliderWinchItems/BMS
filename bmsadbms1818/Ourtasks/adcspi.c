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
uint32_t dbbmst[24];

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
//	float x;

	// Initialize and start read sequence
	adcbms_startreadbms();

dbadcspit1 = DTWTIME; 

	// Wait for sequence to complete
	// Once started, sequence driven by interrupts until complete
	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);

	

extern osThreadId_t defaultTaskHandle;	
xTaskNotify(defaultTaskHandle, 0x1, eSetBits );

// Total time (cells, thermistors, top-of-stack)
//dbadcspit3 = DTWTIME - dbadcspit1;

//	SELECT_GPIO_Port->BSRR = (SELECT_Pin << 16); // Set pin low
//	SELECT_GPIO_Port->BSRR = SELECT_Pin; // Set pin high

	// Debugging trap for timeout (3 sec) (interrupt sequence hangs)
	if (noteval1 != TSKNOTEBIT00) morse_trap(814); // JIC debug

	/* Calibrate and store readings in requester's array (float). */
	// Adjust cells 1-16 order to account for up or down readout. */
	for (i = 0; i < 16; i++)
	{
		// Readout "up" (cells 1->16) = 1; Down (cells 16->1) = 0
		if (p->updn == 0)
		{ // Here, cell number readout sequence was 16->1
			*(pssb->taskdatai16+i) = adcspiall.raw[15-i];
			*(pssb->taskdata+i) = adcparams_calibbms(*(pssb->taskdatai16+i),i);
		}
		else
		{ // Here, cell sequence readout was 1->16
			*(pssb->taskdatai16+i) = adcspiall.raw[i];
			*(pssb->taskdata+i) = adcparams_calibbms(*(pssb->taskdatai16+i),i);
		}
	}
	// Thermistors and top-of-stack
	for (i = 16; i < ADCBMSMAX; i++)
	{
		*(pssb->taskdatai16+i) = adcspiall.raw[i];
		*(pssb->taskdata+i) = adcparams_calibbms(*(pssb->taskdatai16+i),i);			
	}

	/* Signal 'main.c' that there is a bms reading. */
	readbmsflag = 1;
	// Upon this return ADCTask will notify requester.

	//adcspi_setfets();
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
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset pin: /CS set low

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
	p->spitx24.uc[2] = 0x40; // Set DIAG: 10 ua discharge on all cells.

	p->timstate = TIMSTATE_OPENCELL1;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset pin: /CS set low
	/* Loading starts SPI to set DIAG bit for 10 ua discharge. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel

	/* Approximately 500 ms will complete sending plus delay. */
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
		*(pssb->taskdata+i) = adcparams_calibbms(adcspiall.raw[i],i);
		if (*(pssb->taskdata+i) < bqfunction.lc.cellopenlo)
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
	p->spitx24.uc[2] = 0x80; // Set LOPW: 1 ua

	p->timstate = TIMSTATE_LOWPOWER;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset pin: /CS set low

	/* Loading DMA count starts transfer. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3;  // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR  |= 1;  // Enable channel

	TIM15->CCR1 = TIM15->CNT + SPIDELAY; // Set SPI xmit duration

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
	p->cellbitssave  = pssb->cellbits;

	// Set command code
	p->spitx24.uc[2] = 0;

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset pin: /CS set low

	/* Loading starts transfer. */
	hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3;  // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR  |= 1;  // Enable channel

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

	/* All states start with zero. */
	p->timstate = TIMSTATE_IDLE;
	return;
}
/* #######################################################################
 * void adcspi_tim15_IRQHandler(void);
 * @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */

uint32_t dbisr15;
uint32_t dbisr1;
uint32_t dbdier1;
uint32_t dbtrap;
uint32_t dbcapt;

void adcspi_tim15_IRQHandler(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;

	/* Clear interrupt flag. */
	TIM15->SR = ~(1 << 1); // Bit 1 CC1IF: Capture/Compare 1 interrupt flag

	switch (p->timstate)
	{
	case TIMSTATE_IDLE: // Idle. 
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set pin: /CS set high
 	 	return;


	case SPISTATE_CALIB1: // Command to clear registers complete

		// Set /CS high to latch the command
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set pin: /CS set high

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

		notCS_GPIO_Port->BSRR = (notCS_Pin<<16); // Reset: /CS pin set low
		p->spitx24.ui = 0; // Clear BMS register

		/* Loading starts SPI transfer. */
		hdma_spi1_tx.Instance->CCR  &= ~1; // Disable channel
		hdma_spi1_tx.Instance->CNDTR = 3;  // Set number to DMA transfer
		hdma_spi1_tx.Instance->CCR  |= 1;  // Enable channel

		TIM15->CCR1 = TIM15->CNT + SPIDELAY; // Set SPI xmit duration
		p->timstate = SPISTATE_CALIB2;
		break;		

	case SPISTATE_CALIB2: // 

		notCS_GPIO_Port->BSRR = notCS_Pin; // Set pin: /CS set high

		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify ADCTask.c that self-calibration is complete.
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );		
		break;

	case SPISTATE_FETS: // Set discharge FETs
		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Cause command values to latch
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set pin: /CS set high
		// Notify 'adcspi_setfets' FET word has been sent and latched.
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );	
		portYIELD_FROM_ISR( xHPT);		
		break;

	case TIMSTATE_OPENCELL1:
		// Raise /CR to cause command values become active
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set pin: /CS set high

		// Set time delay for DIAG 10ua current to discharge flying caps.
		// 16b time counter requires multiple timeout interrupts
		p->tim15ctr = ((500 * 1000)/ (65536 / TIM15MHZ)) + 1; // Approximately 503 ms.

		p->timstate = TIMSTATE_OPENCELL2; // Set timer interrupt handling.
		TIM15->SR = ~(1 << 1); // Reset TIM15CH1:OC interrupt flag: CC1IF if had come on.
		break;

	case TIMSTATE_OPENCELL2: // Wait for DIAG to discharge completed
		p->tim15ctr -= 1;
		if (p->tim15ctr > 0) break;
		// Here, 500+ ms delay for 10ua discharge completed.
		// Start a BMS readout cycle 
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );
		break;		

// NOTE: This is the same as SPISTATE_FETS...
	case TIMSTATE_LOWPOWER:
		notCS_GPIO_Port->BSRR = notCS_Pin; // Set pin: /CS set high
		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify 'adcspi_lowpower' command complete
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );	
		portYIELD_FROM_ISR( xHPT );
		break;			

default: morse_trap(810); 
	break; // Debug jic
	}
	return;
}
/* #######################################################################
 * SPI tx & rs dma transfer complete (if enabled!)
   ####################################################################### */
uint32_t dbspidma;
uint32_t dbspidma2;
uint32_t dbspidiff;

void adcspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx)
{
dbspidma2 = DTWTIME;
dbspidiff = dbspidma2 - dbspidma;

	morse_trap(841); // Debug JIC}

/* The following will not execute if the morse-trap above is not commented out. */
// Reset interrupt flags	
hdma_spi1_rx.DmaBaseAddress->IFCR = 0xfff;

if ((GPIOA->ODR & (1<<15)) != 0)
  GPIOA->BSRR = (1<<(15+16)); // Reset /CS
else
  GPIOA->BSRR = (1<<(15+ 0)); // Set /CS

	hdma_spi1_rx.Instance->CCR &= ~1; // Disable channel
	hdma_spi1_rx.Instance->CNDTR = 3;//3; // Number to DMA transfer
	hdma_spi1_rx.Instance->CCR |= 3;  // Enable channel

	hdma_spi1_tx.Instance->CCR &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;//1;  // Enable channel
return;
}

void adcspi_spidmatx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_tx)
{
dbspidma2 = DTWTIME;
dbspidiff = dbspidma2 - dbspidma;	
	morse_trap(842); // Debug JIC}

// Debug: Constant SPI for 'scoping
hdma_spi1_tx.DmaBaseAddress->IFCR = 0x00f;

volatile uint32_t spin;
for (spin = 0; spin < 30; spin++);

if ((GPIOA->ODR & (1<<15)) != 0)
  GPIOA->BSRR = (1<<(15+16)); // Reset /CS
else
  GPIOA->BSRR = (1<<(15+ 0)); // Set /CS

	hdma_spi1_rx.Instance->CCR &= ~1; // Disable channel
	hdma_spi1_rx.Instance->CNDTR = 3;//3; // Number to DMA transfer
	hdma_spi1_rx.Instance->CCR |= 1;  // Enable channel

	hdma_spi1_tx.Instance->CCR &= ~1; // Disable channel
	hdma_spi1_tx.Instance->CNDTR = 3; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 3;//1;  // Enable channel
return;	
}
