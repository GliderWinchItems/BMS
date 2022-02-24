/******************************************************************************
* File Name          : adcspi.c
* Board              : bms14921: STM32L431
* Date First Issued  : 02/19/2022
* Description        : ADC w SPI management with MAX1421 board
*******************************************************************************/

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "adcspi.h"
#include "adcparams.h"
#include "adcfastsum16.h"
#include "ADCTask.h"

#include "morse.h"

extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;


/* 24 bit control word sent to MAX14921 via SPI. 
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
SC0  = [2]:6 
SC1  = [2]:5 
SC2  = [2]:4 
SC3  = [2]:3 
/SMPL= [2]:2 1 = hold; 0 = sample if SMPL pin high
DIAG = [2]:1 1 = 10 ua leakage on CV inputs
LOPW = [2]:0 1 = Low power mode: Vp = 1 ua


*/
/*
struct ADCSPIALL
{
union SPI24 spitx24;
union SPI24 spirx24;
uint8_t cellnum;
uint8_t adcstate;
uint8_t timstate;
uint8_t spistate;
uint8_t readyflag;
};
struct ADCSPIALL adcspiall;
*/

#define TIM2MHZ 16  // Timer rate MHz
#define DELAY8MS  (TIM2MHZ * 1000 * 8) // 8 millisecond TIM2 OC increment
#define DELAYUS    TIM2MHZ // 1 microsecond TIM2 OC increment

static uint32_t noteval1;

/* *************************************************************************
 * void adcspi_readbms(void);
 * @brief	: Do a sequence for reading MA14921 plus direct ADC inputs
 * *************************************************************************/
void adcspi_readbms(void)
{
	// Initialize and start read sequence
	adcbms_startreadbms();

	// Wait for sequence to complete
	// Once started, sequence driven by interrupts until complete
	xTaskNotifyWait(0,0xffffffff, &noteval, 3000);
if (noteval1 != pssb->tasknote) morse_trap(814); // JIC debug

	/* Calibrate readings into requester's array. */
	for (i = 0; i < ADCBMSMAX; i++)
	{
		*(pssb->taskdata+i) = adcparams_calibbms(i);
	}

// Debugging trap for timeout (3 sec)
if (noteval != TSKNOTEBIT00) morse_trap(808);

	return;
}
/* *************************************************************************
 * void adcspi_calib(void);
 * @brief	: Execute a self-calibration sequence
 * *************************************************************************/
static const uint32_t spiclear = 0;
void adcspi_calib(void)
{
	HAL_StatusTypeDef halret;
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	uint32_t noteval1 = 0; // TaskNotifyWait notification word

	/* All states start with zero. */
	p->adcstate = ADCSTATE_IDLE;
	p->timstate = TIMSTATE_IDLE;
	p->spistate = SPISTATE_CALIB1; // Self-calibration sequence before readouts

	/* Set MX14921 command to initiate self-calibration. */
	p->spitx24.ui = 0x00400000; // Initiate offset calibration upon /CS lo->hi transition

	/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
	HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_RESET);
	halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spiclear, &p->spirx24.uc[0], 3);
	/* SPI DMA interrupt callback routine (SPISTATE_CALIB) will cause /CS lo->hi 
	   transition AND set an 8 ms delay in TIM2CH4 OC. The OC interrupt will 
	   cause the following wait for notifcation to exit.   */
	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);

if (noteval1 != pssb->tasknote) morse_trap(813); // JIC debug

	/* Return will be to ADCTask.c and beginning of for (;;) loop waiting
	   for others to load a readout request into the queue. */
	return.
}
/* *************************************************************************
 * void adcspi_opencell(void);
 * @brief	: Check for open cell wires
 
 * *************************************************************************/
void adcspi_opencell(void)
{
}
/* *************************************************************************
 * void adcspi_lowpower(void);
 * @brief	: Place MAX14921 into low power mode
 * *************************************************************************/
void adcspi_lowpower(void)
{
}
/* *************************************************************************
 * void adcspi_readadc(void);
 * @brief	: Read selected ADCs (non-MAX14921)
 * *************************************************************************/
void adcspi_readadc(void)
{
}
/* *************************************************************************
 * void adcspi_setfets(void);
 * @brief	: Read selected ADCs (non-MAX14921)
 * *************************************************************************/
void adcspi_setfets(void)
{
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
	p->spistate = SPISTATE_CALIB; // Self-calibration sequence before readouts

	p->delayct = DELAYUS * 5; // 5 us delay

	p->readyflag = 0; // Use for checking if TIM2 in FanTask is late.

	return.
}


/* #######################################################################
 * void adcspi_tim2(TIM_TypeDef  *pT2base);
 * @brief	: TIM2:CC4IF interrupt (arrives here from FanTask.c)
 * @param	: pT2base = pointer to TIM2 register base
   ####################################################################### */

// ECS plus cell selection bits for cells 1-> 16, given indices [0]->[15]
static const uint8_t bitorderUP[16] = {0x84,0XC4,0XA4,0XE4,0X94,0XD4,0XB4,0XE4,
	                                   0X8C,0XCC,0XAC,0XEC,0X9C,0XDC,0XBC,0XFC};

// ECS plus cell selection bits for cells 16-> 1, given indices [0]->[15]
static const uint8_t bitorderDN[16] = {0xFC,0XBC,0XDC,0X9C,0XEC,0XAC,0XCC,0X8C,
	                                   0XF4,0XB4,0XD4,0X94,0XE4,0XA4,0XC4,0X84};

static const uin32_t spicalib2 = 0;

void adcspi_tim2(TIM_TypeDef *pT2base)
{
	/* JIC FAN task starts ahead of ADC task. */
	if (p->readyflag == 0) return;

	if (p->spiidx >= ADCBMSMAX)
	{ // Done with SPI sending commands & TIM2 delay for settling 
		p->timstate = TIMSTATE_IDLE; 
		return;
	}

	switch (p->timstate)
	{
	case TIMSTATE_IDLE: // Idle. Ignore interrupt
 	 return;

	case TIMSTATE_8MSTO: // 8 ms timeout
		p->timestate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify ADCTask.c that self-calibration is complete.
		xTaskNotifyFromISR(ADCTaskHandle, 0x1, eSetBits, xHPT );
	 return;

	case TIMSTATE_2: // Here, selection command timeout complete
p->timestate = TIMSTATE_IDLE; // Set to idle, jic

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

			p->spistate= SPISTATE_IDLE;

			break;

		default: morse_trap(801);
			break;
		}

		// First SPI cell select command does not overlap ADC conversions. No need to
		// check if ADC is still busy with previous conversion.
		if (p->spiidx == 0)
		{ // Ground reference time delay completed, send first cell selection command
			p->spistate = SPISTATE_2;

			// Enable /CS and start SPI sending first cell selection
			HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_RESET);
			halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);

			// Next TIM2 will prepare command to select cell #2.
			p->spiidx = 1;

			break; // Break top level
		}

		// Prepare and start next SPI selection command
		p->spiidx += 1; // Index is "one ahead"

		if (p->adcrestart == 0)
		{ /* Here, ADC was not busy when SPI completed sending the command. SPI interrupt 
		 	raised /CR to latch the command and started TIM2 timeout for settling.
		 	The TIM2 timeout ended up here. */
	if (p->adcflag != 0) morse_trap (804);


			// Setting ADSTART starts regular conversion
			p->adcflag = 1; // Show ADC started
			hadc1.Instance->CR |= (0x1 << 2); // Start ADC

			p->spistate= SPISTATE_2; // Necessary?

			// Send SPI msg to clear everything
			HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_RESET);
			halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &spicalib2, &p->spirx24.uc[0], 3);
		}
		break; // Break top level switch statement

default: morse_trap(809); // Debug JIC
	break;

	}
	return;
}
/* #######################################################################
 * SPI DMA RX complete callback
 * RX lags TX slightly, so RX is used
   ####################################################################### */
void HAL_SPI_MasterRxCpltCallback(void)
{
	BaseType_t xHPT = pdFALSE;
	uint32_t tmp;

	// Show others that SPI is not busy
	p->spiflag = 0;

	switch (p->spistate)
	{
	case SPISTATE_CALIB1: // Command to clear registers complete
		// Set /CS high to latch the command
		HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_SET);

		// Set timout delay for ground reference transfer to complete & settle.
		// Doing this here allows some response time for the GPIO pin
		pT2base->CCR4 = pT2base->CNT + (DELAYUS * 50); // Set 50 us timeout duration
		p->timestate = 	TIMSTATE_8MSTO; // Set next step when timer interrupt takes place
		pT2base->SR = 0x10; // Reset interrupt flag: CC4IF if had come on.

		/* Set /CS (GPIOA PIN_15) low, then start SPI sending command. */
		HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_RESET);
		halret = HAL_SPI_TransmitReceive_DMA(&hspi1, &p->spitx24.uc[0], &p->spirx24.uc[0], 3);
		p->spistate = SPISTATE_CALIB2;
		break;

	case SPISTATE_CALIB2: // Self-calibration command completed except for /CS
		// Setting /CS high initiates self-calibration
		HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_SET);
		// TIM2 8MS timeout will complete the sequence.
		break;

	case SPISTATE_SMPL: // Command to switch flying caps to ground has been sent.
		/* There is a 50 us timeout for it to settle. After this the first selection
			command will be sent. */
		// Raise /CS to latch command bits and begin ground reference transfer
		HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_SET);

		// Set time delay for settling
		pT2base->CCR4 = pT2base->CNT + (DELAYUS * 50); // Set 50 us timeout duration
		p->timestate = TIMSTATE_2; // Set timer interrupt handling.
		pT2base->SR = 0x10; // Reset interrupt flag: CC4IF if it had come on.
		break;

	case SPISTATE_2: // Command has been sent
		/* If ADC is busy, then do not latch the command into the MAX14921 registers. 

		   Set adcflag so that the ADC completion interrupt will raise /CR to latch 
		the selection, and set a settling time timeout. 

		   The TIM2 timeout interrupt will then start the ADC conversions, and launch the
		 next SPI command.
		*/
		if (p->adcflag != 0)
		{ // Here, ADC coversion is not complete
			// ADC will start next spi command
			p->adcrestart = 1;
			break;
		}
		// 'else' Latch selection command and start time delay
		// Cause command values to latch
		HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_SET);

		p->adcrestart = 0; // JIC

		// Set a 5 us time delay (for most) for MAX14921 mux switch and Aout settling
		pT2base->SR = 0x10; // Reset interrupt flag: CC4IF if had come on.
		pT2base->CCR4 = pT2base->CNT + p->delayct; // Set timeout duration
		p->timstate = TIMSTATE_2; // Set timer timeout state
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
 * void adcspi_adc_Handler(ADC_HandleTypeDef* phadc1);
   ####################################################################### */
void adcspi_adc_Handler(ADC_HandleTypeDef* phadc1)
{

	ADC_TypeDef* pADCbase = hadc1.Instance; // Convenience pointer
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;

	p->adcflag = 0; // Show ADC complete

	// ISR Bit 2 EOC: End of conversion flag
	if ((pADCbase->ISR & (1 << 2)) != 0)
	{ // Here, conversion complete
		pADCbase->ISR = (1 << 2); // Reset interrupt flag

		p->raw[p->adcidx] = pADCbase->DR; // Store data
		p->adcidx += 1; 
		if (p->adcidx >= ADCBMSMAX)
		{ // Here end of 16 cells, 3 thermistors, 1 top-of-stack
			xTaskNotifyFromISR(ADCTaskHandle, 0x1, eSetBits, xHPT );
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
				HAL_GPIO_WritePin(SPI1_NSS__CS_GPIO_Port,SPI1_NSS__CS_Pin,GPIO_PIN_SET);

				// Set settling time for MUX & Aout
				pT2base->CCR4 = pT2base->CNT + p->delayct; // Set 5 us (usually) timeout duration
				p->timestate = TIMSTATE_2;
			}
		}
	}
	return;
}