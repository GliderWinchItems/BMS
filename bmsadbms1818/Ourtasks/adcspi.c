/******************************************************************************
* File Name          : adcspi.c
* Board              : bmsbms1818: STM32L431
* Date First Issued  : 06/17/2022
* Description        : SPI management with ADBMS1818 board
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

void adcspi_rw_cmd(uint16_t* pcmd, uint16_t* pdata, unit8_t rw);

#define CSBDELAYFALL (5*16) // CSB falling delay: 5 us with 16 MHz sysclock
#define CSBDELAYRISE (3*16) // CSB rising delay: 3 us with 16 MHz sysclock

static uint8_t rwtype; // code: command, command + data, etc.

static uint32_t noteval1;

#define READOUTSIZE_ADVAX 7 // Number of registers to read 
// Read commands for registers to be read
static const uint16_t cmdv[READOUTSIZEADVAX] = {
	RDCVA, /* 0x004  // Read Cell Voltage Register Group A */
	RDCVB, /* 0x006  // Read Cell Voltage Register Group B*/
	RDCVC, /* 0x008  // Read Cell Voltage Register Group C*/
	RDCVD, /* 0x00A  // Read Cell Voltage Register Group D*/
	RDCVE, /* 0x009  // Read Cell Voltage Register Group E*/
	RDCVF, /* 0x00B  // Read Cell Voltage Register Group F*/
	RDAUXA,/* 0x00C  // Read Auxiliary Register Group A   */
};
#define READOUTSIZE_AUX 4
static const uint16_t cmdaux[READOUTSIZE_AUX = {
	RDAUXA,/*   0x00C  // Read Auxiliary Register Group A */
    RDAUXB,/*   0x00E  // Read Auxiliary Register Group B */
    RDAUXC,/*   0x00D  // Read Auxiliary Register Group C */
    RDAUXD,/*   0x00F  // Read Auxiliary Register Group D */
};
#define READOUTSIZE_STAT 2
static const uint16_t cmdstat[READOUTSIZE_STAT] = {
	RDSTATA,/*  0x010  // Read Status Register Group A */
	RDSTATB,/*  0x012  // Read Status Register Group B */
};
#define READOUTSIZE_CONFIG	 2
static const uint16_t cmdconfig[READOUTSIZE_STAT] = {
	RDCFGA,/*  0x010  // Read Configuration Register Group A */
	RDCFGB,/*  0x012  // Read Configuration Register Group B */
};
static const uint16_t cmdcmd[4] = {
	ADCVAX, /* Start Combined Cell Voltage and GPIO1 GPIO2 Conversion */
	ADAX,   /* Start GPIOs ADC Conversion */
	ADSTAT, /* Start Status Group ADC Conversion */
	NULL,   /* Skip sending a "start" command. */
};

uint8_t readbmsflag; // Let main know a BMS reading was made

/* *************************************************************************
 * void adcspi_preinit(void);
 * @brief	: Save some things used later.
 * *************************************************************************/
 void adcspi_preinit(void)
{

	/* DMA1 CH3 (SPI write) peripheral aand memory addresses: */
	hdma_spi1_tx.Instance->CPAR = (uint32_t)hspi1.Instance + 0x0C; // SPI DR adddress
	hdma_spi1_tx.Instance->CMAR = (uint32_t)&adcspiall.spitx12.uc[0]; // DMA stores from this array
//	hdma_spi1_tx.Instance->CCR |= (1 << 1); // TCIE: enable DMA interrupt

	/* DMA1 CH2 (SPI read) peripheral and memory addresses: */
	hdma_spi1_rx.Instance->CPAR = (uint32_t)hspi1.Instance + 0x0C; // SPI DR address
	hdma_spi1_rx.Instance->CMAR = (uint32_t)&adcspiall.spirx12.uc[0]; // DMA stores from this array

	/* MX may have these setup */
	hdma_spi1_rx.Instance->CCR |=  0x2;  // Enable DMA read channel interrupt
	hdma_spi1_tx.Instance->CCR &= ~0x2;  // Disable DMA write channel interrupt

	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB (/CS) pin set high

	// Bit 1 TXDMAEN: Tx buffer DMA enable
	// Bit 0 RXDMAEN: Rx buffer DMA enable
 	SPI1->CR2 |= (1<<1) | (1 << 0);

	// Bits 5:3 BR[2:0]: Baud rate control
 	SPI1->CR1 &= ~(0x7 << 3); // Clear existing
 	SPI1->CR1 |=  (0x3 << 3); // Clock divider: 011: fPCLK/16

	// Bit 0 CPHA: Clock phase: 1: The second clock transition is the first data capture edge 	
	// Bit 1 CPOL: Clock polarity 0: CK to 0 when idle 1: CK to 1 when idle
 	SPI1->CR1 &= ~(0x3 << 0); // Clear existing
 	SPI1->CR1 |=  (0x3 << 0); // CPHA|CPOL = 1|1

 	/* Init CRC peripheral. */
 	pec15_reg_init ();

	/* EXTI4 ('1818 SDO conversion complete) interrupt initialize*/
  	HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ  (EXTI4_IRQn);

	return;
}
/* *************************************************************************
 * static uint16_t readreg(uint16_t* p, uint16_t* pcmdr, uint8_t n);
 * @brief	: Read register, convert endianness
 * @param   : p = pointer to output array (little endian)
 * @param   : pcmdr = pointer to read command
 * @param   ; n = number of register reads
 * @return  : 0 = no PEC15 error
 * *************************************************************************/
static unint16_t readreg(uint16_t* pcmdcmd, uint16_t* p, uint16_t* pcmdr, uint8_t n)
{
	uint16_t i;

	if (pcmdcmd != NULL)
	{ /* Send a "Start conversion" command. */
		adcspi_rw_cmd(pcmdcmd, NULL, 3); 

		// Wait for conversions to complete
		xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
		if (noteval1 == 0) morse_trap(254);
	}

	/* Read registers holding conversion results. */
	for (i = 0; i < n; i++)
	{
	 	adcspi_rw_cmd(pcmd, NULL, 2);
	 	pcmd += 1;

	 	// Wait for SPI reading sequence to complete
		xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
		if (noteval1 == 0) morse_trap(259);

		// Convert big endian to little endian and store in array
		*p++ = (uint16_t)__REV16 (adcspiall.spirx12.u16[2]);
		*p++ = (uint16_t)__REV16 (adcspiall.spirx12.u16[3]);
		*p++ = (uint16_t)__REV16 (adcspiall.spirx12.u16[4]);
	}
	return 0;
}
/* *************************************************************************
 * void adcspi_readstuff(uint8_t readstuff);
 * @brief	: Do conversion, then read registers with results
 * @brief   : readstuff = code for selection
 * *************************************************************************/
void adcspi_readstuff(uint8_t readstuff)
{
	uint16_t cmd1;

	switch (readstuff)
	{
	case READCELLSGPIO12: // Read cell voltages + GPIO1 & GPIO2
		readreg(&cmdcmd[READCELLSGPIO12], adcspiall.cellreg, cmdv, 6);
		readreg(adcspiall.auxreg, &cmdv[6], 1);
		break;

	case READGPIO: // Read all 9 GPIOs voltage registers: A-D
		readreg(&cmdcmd[READGPIO], adcspiall.auxreg,cmdaux,READOUTSIZE_AUX);
		break;

	case READSTAT: // Read status
		readreg(&cmdcmd[READSTAT], adcspiall.statreg,cmdstat,READOUTSIZE_STAT);
		break;

	case READCONFIG: // Read configuration
		readreg(NULL, adcspiall.configreg,cmdconfig,READOUTSIZE_CONFIG);
		break;		

	default: 
		morse_trap (253);
	}

	/* Signal 'main.c' that there is a bms reading. */
	readbmsflag = 1;
	// Upon this return ADCTask will notify requester.

	//adcspi_setfets();
	return;
}

/* *************************************************************************
 * void adcspi_rw_cmd(uint16_t* pcmd, uint16_t* pdata, unit8_t rw);
 * @brief	: Send command  and write data (little endian)
 * @param   : pcmd = pointer to 2 byte command (little endian)
 * @brief   : pdata = pointer to six bytes to be written (little endian),
 *          :    ignore pdata for read commands.
 * @brief   : rw = type of command sequence
 *          : 0 = Send 2 byte command  + pec
 *          : 1 = Send command+pec, plus 6 bytes data+pec
 *          : 2 = Send command+pec, read 6 bytes data+pec into spirx12.uc[4]-[11]
 *          : 3 = Send 2 byte command  + pec. Switch '1818 SDO to interrupt conversion completion
 * *************************************************************************/
void adcspi_rw_cmd(uint16_t* pcmd, uint16_t* pdata, unit8_t rw)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	uint32_t dmact; // Number of bytes for DMA

	/* Do this early since time delay involved. */
	CSB_GPIO_Port->BSRR = (CSB_Pin<<16); // Reset: CSB pin set low

	rwtype = rw; // Save for interrupt routines.

	/* Build byte array for a single SPI/DMA operation. */
	// Command 
    CRC->CR = 0x9; // 16b poly, + reset CRC computation
	p->spitx12.u16[0] = (uint16_t)__REV16 (pcmd);	// Command: big endian
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (p->spitx12.u16[0]); 
	p->spitx12.u16[1] = (uint16_t)__REV16 (CRC->DR); // Big endian: Store PEC for command bytes

	switch (rw)
	{
	case 0: // Send command only
	case 3: // Send command only, switch '1818 SDO to interrupt 
		dmact = 4;
		break;

	case 1: // Send command and write 6 data+2 pec
if (pdata == NULL)	 morse_trap(259);
		dmact = 12;
		CRC->CR = 0x9; // 16b poly, + reset CRC
		//* Set data as big endian 1/2 words, plus pec big endian
		p->spitx12.u32[1] = (uint32_t)__REV   (*(uint32_t*)(pdata+2)); // Load 4 bytes
		*(__IO uint32_t*)CRC_BASE = (uint32_t)__REV (p->spitx12.u32[1]);

		p->spitx12.u16[4] = (uint16_t)__REV16 (*(uint16_t*)(pdata+4)); // Load 2 bytes
    	*(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (p->spitx12.u16[4]);

    	p->spitx12.u16[5] = (uint16_t)__REV16 (CRC->DR); // Big endian: Store PEC for command bytes
    	break;

    case 3: // Send command that reads 8 bytes into spirx12
		dmact = 12;
		break;

	default: 
		morse_trap(255);		
	}

	/* Setup DMA read and write. */
	hdma_spi1_rx.Instance->CCR &= ~1; // Disable dma spi rx channel
	hdma_spi1_rx.Instance->CNDTR = dmact; // Number to DMA transfer
	hdma_spi1_rx.Instance->CCR |= 1;  // Enable channel

	hdma_spi1_tx.Instance->CCR &= ~1; // Disable dma spi tx channel
	hdma_spi1_tx.Instance->CNDTR = dmact; // Number to DMA transfer
	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel 	

	adcspiall.timstate = TTIMSTATE_1;

	/* Allow 5 us CSB (chip select) delay. Before enabling SPI. */
	TIM15->CCR1 = TIM15->CNT + CSBDELAYFALL; 
	adcspiall.timstate = TIMSTATE_1;

	return;
}	
/* #######################################################################
 * void adcspi_tim15_IRQHandler(void);
 * @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
void adcspi_tim15_IRQHandler(void)
{
	struct ADCSPIALL* p = &adcspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;

	/* Clear interrupt flag. */
	TIM15->SR = ~(1 << 1); // Bit 1 CC1IF: Capture/Compare 1 interrupt flag

	switch (p->timstate)
	{
	case TIMSTATE_IDLE: // Idle. 
		CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high
 	 	return;

 	case TIMSTATE_1: // CSB falling delay expired. Read to send
 	 	// Start sending write command + data
		// Bit 6 SPE: SPI enable
		SPI1->CR1 |= (1 << 6);
		// SPI rx interrupt expected next.	
 	 	return;

 	case TIMSTATE_2: // CSB rising delay expired
		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify ADCTask.c
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );		
		return;

	case TIMSTATE_3: // EXTI wait for rising EXTI timed out
		CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

	  morse_trap(258); // EXTI interrupt wait timed out

		p->timstate = TIMSTATE_IDLE; // Set to idle, jic
		// Notify ADCTask.c
		xTaskNotifyFromISR(ADCTaskHandle, TSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );		
		return;
	
	default: morse_trap(810); 
		break; // Debug jic
	}
	return;
}
/* #######################################################################
 * SPI rx dma transfer complete
   ####################################################################### */
void adcspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx)
{
	SPI1->CR1 &= ~(1 << 6); // Disable SPI

	// Reset DMA interrupt flags	
	hdma_spi1_rx.DmaBaseAddress->IFCR = 0xfff;

	if (rwtype == 3)
	{ // Here, end of command that starts conversions. 
		/* Setup '1818 SDO to interrupt when it goes high. */
		// Enable interrupt & event
		EXTI->IMR1 |= (1<<4);
		EXTI->EMR1 |= (1<<4);
		EXTI->PR1  |= (1<<4); // Reset Pending JIC

		/* Timeout: JIC, and debugging, and whatever... */
		adcspiall.timstate = TIMSTATE_3;
		TIM15->CCR1 = TIM15->CNT + (16*4000);  // 4 ms is enough

		// Expect next interrupt is SPI SDI pin going high
		return;
	}

	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high
	
	// Assure CSB minimum pulse width before next SPI operation.
	adcspiall.timstate = TIMSTATE_2;
	TIM15->CCR1 = TIM15->CNT + CSBDELAYRISE; 

	return;
}
/* #######################################################################
 * SPI tx dma transfer complete (if enabled!)
   ####################################################################### */
void adcspi_spidmatx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_tx)
{
	morse_trap (281); // Bogus tx interrupt
	return;
}
/* #######################################################################
 * PB4 SPI MISO pin-1818 SDO pin: interrupt upon rising edge
   ####################################################################### */	
void EXTI4_IRQHandler(void)
{
	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

	// Disable interrupt & event
	EXTI->IMR1 &= ~(1<<4);
	EXTI->EMR1 &= ~(1<<4);
	EXTI->PR1   =  (1<<4); // Reset request: PB4

	// Assure CSB minimum pulse width before next SPI operation.
	adcspiall.timstate = TIMSTATE_2;
	TIM15->CCR1 = TIM15->CNT + CSBDELAYRISE; 

	// Next TIM15 interrupt signals completion
	return;
}