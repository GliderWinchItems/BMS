/******************************************************************************
* File Name          : bmsspi.c
* Board              : bmsbms1818: STM32L431
* Date First Issued  : 06/17/2022
* Description        : SPI management with ADBMS1818 board
*******************************************************************************/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "malloc.h"
#include "bmsspi.h"
#include "adcparams.h"
#include "ADCTask.h"
//#include "bmsdriver.h"
#include "main.h"
#include "FanTask.h"
#include "BMSTask.h"
#include "DTW_counter.h"
#include "ADBMS1818_command_codes.h"
#include "fetonoff.h"
#include "pec15_reg.h"
#include "bms_items.h"

#include "morse.h"

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

//extern	TaskHandle_t bmsTaskHandle; // Calling task's task handle
extern struct BMSSPIALL bmsspiall;

 union SPI12 spitx12; // SPI command sent to '1818'
 union SPI12 spirx12; // SPI monitor received from '1818'
static uint8_t  timstate;   // State for ISR handling: TIM
static int16_t  tim15ctr;   // TIM15CH1:OC turnover counter

uint32_t dbgt1,dbgt2;

void bmsspi_readstuff(uint8_t code);
void bmsspi_rw_cmd(uint16_t cmd, uint16_t* pdata, uint8_t rw);


enum TIMSTATE
{
	TIMSTATE_IDLE,
	TIMSTATE_1,
	TIMSTATE_2,
	TIMSTATE_3,
	TIMSTATE_TO,
};

#define CSBDELAYFALL (12*16) // CSB falling delay: 6 us with 16 MHz sysclock
#define CSBDELAYRISE (10*16) // CSB rising delay: 5 us with 16 MHz sysclock

static uint8_t rwtype; // code: command, command + data, etc.

#define READOUTSIZE_ADVAX 7 // Number of registers to read 
// Read commands for registers to be read
static const uint16_t cmdv[READOUTSIZE_ADVAX] = {
	RDCVA, /* 0x004  // Read Cell Voltage Register Group A */
	RDCVB, /* 0x006  // Read Cell Voltage Register Group B*/
	RDCVC, /* 0x008  // Read Cell Voltage Register Group C*/
	RDCVD, /* 0x00A  // Read Cell Voltage Register Group D*/
	RDCVE, /* 0x009  // Read Cell Voltage Register Group E*/
	RDCVF, /* 0x00B  // Read Cell Voltage Register Group F*/
	RDAUXA,/* 0x00C  // Read Auxiliary Register Group A   */
};
#define READOUTSIZE_AUX 4
static const uint16_t cmdaux[READOUTSIZE_AUX] = {
	RDAUXA, /*   0x00C  // Read Auxiliary Register Group A */
    RDAUXB, /*   0x00E  // Read Auxiliary Register Group B */
    RDAUXC, /*   0x00D  // Read Auxiliary Register Group C */
    RDAUXD, /*   0x00F  // Read Auxiliary Register Group D */
};
#define READOUTSIZE_STAT 2
static const uint16_t cmdstat[READOUTSIZE_STAT] = {
	RDSTATA, /*  0x010  // Read Status Register Group A */
	RDSTATB, /*  0x012  // Read Status Register Group B */
};
#define READOUTSIZE_CONFIG	 2
static const uint16_t cmdconfig[READOUTSIZE_CONFIG] = {
	RDCFGA, /*  0x002  // Read Configuration Register Group A */
	RDCFGB, /*  0x026  // Read Configuration Register Group B */
};
#define READOUTSIZE_SCTRL	 1
static const uint16_t cmdsreg[READOUTSIZE_SCTRL] = {
	RDSCTRL /* 0x016  // Read S Control Register Group */
};

uint8_t readbmsflag; // Let main know a BMS reading was made

/* *************************************************************************
 * void bmsspi_readbms(void);
 * @brief	: Read BMS plus GPIO1,2
 * *************************************************************************/
void bmsspi_readbms(void)
{
	struct BQFUNCTION* pbq = &bqfunction;
	int i;
	struct BMSCAL* pf;
	float x;

// Debug/test spi w '1818'
//bmsspi_readstuff(READCONFIG);	
//while(1==1) {HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);osDelay(125);}
morse_trap(679);
	// Turn heater, dump, dump2, trickle chgr off
	fetonoff_status_set(0);

	// Read cells and GPIO1, GPIO2
	bmsspi_readstuff(READCELLSGPIO12);

	pf = &pbq->lc.bmscal[0];
	for (i = 0; i < NCELLMAX; i++)
	{
		// Copy register array into cell array
		pbq->cellv_latest[i] = bmsspiall.cellreg[i];
		// Calibration
		x = pbq->cellv_latest[i];
		pbq->cellv[i] = 
		             pf->coef[0] + 
				 x * pf->coef[1] +
			 x * x * pf->coef[2];
		pf +=1;
	}
	// Restore status of FETs
	bmsspi_setfets();

	return;
}
/* *************************************************************************
 * void bmsspi_gpio(void);
 * @brief	: Read BMS 9 GPIOs & calibrate temperature sensors
 * *************************************************************************/
void bmsspi_gpio(void)
{
	bmsspi_readstuff(READGPIO);

	// TODO: calibration
	return;
}
/* *************************************************************************
 * void bmsspi_setfets(void);
 * @brief	: Load discharge fet settings into '1818 & set discharge timer
 * *************************************************************************/
void bmsspi_setfets(void)
{

	/* Update selected Group A & B settings with FET settings. */
	// DCC12-DCC1 CFGA5 CFGAR4 | Discharge timer: DCTO
	bmsspiall.configreg[2] = ((bqfunction.cellbal & 0x0FFF) |
		(0x02 << 12)); // DCTO[0-3] code = 1 minute

	// DCC18-DCC12 CFGBR1 CFGBR0 | Dischg timer enable: DTMEN = 1
	bmsspiall.configreg[3] &= ~0x0BFF; // Clear bits to be set
	bmsspiall.configreg[3] |= (((bqfunction.cellbal >> 8) & 0x03FF) |
		(1 << 11)); // DTMEN bit

	bmsspiall.configreg[0] |= 0x00F8; // Assure GPIO pulldowns off

	/* Write-back configuration Groups A & B. */
	bmsspi_writereg(WRITECONFIG);

	return;
}
/* *************************************************************************
 * void bms_gettemp(void);
 * @brief	: Read AUX and get temperatures
 * *************************************************************************/
void bms_gettemp(void)
{
	bmsspi_readstuff(READAUX);
	return;
}
/* *************************************************************************
 * void bmsspi_wakeseq(void);
 * @brief	: Execute a non-interrupt driven '1818 wake up sequence.
 * *************************************************************************/
void bmsspi_wakeseq(void)
{
	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high
	osDelay(1); // Greater than 250 ns delay
	CSB_GPIO_Port->BSRR = (CSB_Pin<<16); // Reset: CSB pin set low
	osDelay(1);
	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high
	osDelay(6); // Startup delay
	return;
}

/* *************************************************************************
 * uint8_t bmsspi_keepawake(void);
 * @brief	: Execute a valid command to keep awake. Sequence through commands
 * @return  : state number of latest reading
 * *************************************************************************/
static uint8_t bmsspi_keepawake_state; // Step thru commands
uint8_t bmsspi_keepawake(void)
{
	uint8_t retx;
//bmsspi_keepawake_state = 0; // Debug
	switch(bmsspi_keepawake_state)
	{
	case 0: /* Read config registers A & B. */
		bmsspi_readstuff(READCONFIG);
		break;
	case 1: /* Read status registers A & B. */
		bmsspi_readstuff(READSTAT);
		break;
	case 2: // Read S Control register. */
		bmsspi_readstuff(READSREG);
		break;
	case 3: // Write config register A */
		bmsspi_writereg(WRITECONFIG);
		break;
	case 4:	// Read cells and GPIO1 GPIO2
		bmsspi_readstuff(READCELLSGPIO12); 
		break;
	case 5:
		bmsspi_readstuff(READAUX);
		break;
	}

	// main.c uses bmsspi_keepawake_state change
	// to display registers, and uses it as an
	// index for yprintf.
	retx = bmsspi_keepawake_state;

	/* Step to next state. */
	bmsspi_keepawake_state += 1;
	if (bmsspi_keepawake_state > 5)
	bmsspi_keepawake_state = 0;
	return retx;
}
/* *************************************************************************
 * void bmsspi_preinit(void);
 * @brief	: Initialization
 * *************************************************************************/
 void bmsspi_preinit(void)
{
	/* DMA1 CH3 (SPI write) peripheral aand memory addresses: */
	hdma_spi1_tx.Instance->CPAR = (uint32_t)hspi1.Instance + 0x0C; // SPI DR adddress
	hdma_spi1_tx.Instance->CMAR = (uint32_t)&spitx12.u8[0]; // DMA stores from this array
//	hdma_spi1_tx.Instance->CCR |= (1 << 1); // TCIE: enable DMA interrupt

	/* DMA1 CH2 (SPI read) peripheral and memory addresses: */
	hdma_spi1_rx.Instance->CPAR = (uint32_t)hspi1.Instance + 0x0C; // SPI DR address
	hdma_spi1_rx.Instance->CMAR = (uint32_t)&spirx12.u8[0]; // DMA stores from this array

	/* MX may have these setup */
	hdma_spi1_rx.Instance->CCR |=  0x2;  // Enable DMA read channel interrupt
	hdma_spi1_tx.Instance->CCR &= ~0x2;  // Disable DMA write channel interrupt

	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB (/CS) pin set high

	SPI1->CR1 &= ~(1 << 6); // Disable SPI (jic)

	// Bit 1 TXDMAEN: Tx buffer DMA enable
	// Bit 0 RXDMAEN: Rx buffer DMA enable
 	SPI1->CR2 |= ((1<<1) | (1 << 0));

	// Bits 5:3 BR[2:0]: Baud rate control
 	SPI1->CR1 &= ~(0x7 << 3); // Clear existing
 	SPI1->CR1 |=  (0x3 << 3); // Clock divider: 011: fPCLK/16 1 MHz

	// Bit 0 CPHA: Clock phase: 1: The second clock transition is the first data capture edge 	
	// Bit 1 CPOL: Clock polarity 0: CK to 0 when idle 1: CK to 1 when idle
 	SPI1->CR1 &= ~(0x3 << 0); // Clear existing
 	SPI1->CR1 |=  (0x3 << 0); // CPHA|CPOL = 1|1

 	/* Init CRC peripheral. */
 	pec15_reg_init();

 	/* EXTI SDO/MISO PB4 end of conversion detection interrupt. */
 	EXTI->RTSR1 |= (1<<4); // Rising edge

	/* EXTI4 ('1818 SDO conversion complete) interrupt initialize*/
  	HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ  (EXTI4_IRQn);  	

  	/* Make use of TIM15:CH1 OC for timing BMS delays. */
	// ARR default is 0xFFFF (max count-1)
	timstate    = TIMSTATE_IDLE; // Set to idle, jic
	TIM15->SR   = 0;           // Clear any interrupt flags, jic
	TIM15->DIER = (1 << 1);  // Bit 1 CC1IE: Capture/Compare 1 interrupt enable
	TIM15->CR1 |= 1;       // Start counter

	return;
}
/* *************************************************************************
 * static uint16_t readreg(uint16_t cmdx, uint16_t* p, const uint16_t* pcmdr, uint8_t n);
 * @brief	: Read register, convert endianness
 * @param   : cmdx = 1st command
 * @param   : p = pointer to in-memory register array
 * @param   : pcmdr = pointer to read command (0 = skip conversion command)
 * @param   ; n = number of register reads
 * @return  : 0 = no PEC15 error
 * *************************************************************************/
uint16_t dbcrc1;
uint16_t dbcrc2;
uint8_t dbcthi = 0;
uint8_t dbctlo = 10;
uint16_t dbhicmd;
uint16_t dblocmd;
uint8_t actr = 0;
uint32_t dbstat1;
uint32_t dbstat2;

static uint16_t readreg(uint16_t cmdx, uint16_t* p, const uint16_t* pcmdr, uint8_t n)
{
	uint16_t i;

	if (cmdx != 0)
	{ /* Send a "Start conversion" command. */
dbstat1 = DTWTIME;		
		bmsspi_rw_cmd(cmdx, 0, 3); 
dbstat2 = DTWTIME - dbstat1;		
//morse_trap(4);
	}

	/* Read registers with latest results. */
	for (i = 0; i < n; i++)
	{

uint8_t ctr = 0;
#define QQQCTR 10 // Repetition limit count
do
{	
		/* Send command (with wait for completion) */
	 	bmsspi_rw_cmd(*pcmdr, 0, 2);

  // Repeat if all one's was returned
	ctr += 1; // Limit loops
} while( (spirx12.u32[1] == 0xffffFFFF) && 
	     (spirx12.u32[2] == 0xffffFFFF) && 
	     (ctr < QQQCTR) );
if (ctr >= QQQCTR) morse_trap(58); // Trap never completed

/* Check number of loops w command. */
actr += 1;
if (actr > (5*3))
{
	actr = 0;
	dbctlo = 10;
	dbcthi = 0;
	dbhicmd = 0x1234;
	dblocmd = 0;
}
if (ctr <= dbctlo)
{
	dbctlo = ctr;
	dblocmd = *pcmdr;
}
if (ctr >= dbcthi)
{
	dbcthi = ctr;
	dbhicmd = *pcmdr;
}

	/* Check PEC15 matches. */
	CRC->CR = 0x9; // 16b poly, + reset CRC computation
	*(__IO uint32_t*)CRC_BASE = (uint32_t)__REV (spirx12.u32[1]);
	*(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (spirx12.u16[4]); 
	dbcrc1 = spirx12.u16[5];
	dbcrc2 = (uint16_t)__REV16 (CRC->DR);

//	if ( ((uint16_t)__REV16 (CRC->DR)) != spitx12.u16[5])
	if (dbcrc2 != dbcrc1) morse_trap(68); // This "works"
	
		/* Copy 6 byte spi rx to memory array. */
		*p++ = (spirx12.u16[2]);
		*p++ = (spirx12.u16[3]);
		*p++ = (spirx12.u16[4]);	
		/* Point to next command. */
	 	pcmdr += 1;
	}
	return 0;
}
/* *************************************************************************
 * void bmsspi_writereg(uint8_t code);
 * @brief   : Write register group(s)
 * @param   : code = code for selection of register group
 * *************************************************************************/
void bmsspi_writereg(uint8_t code)
{
	switch(code)
	{
	case WRITECONFIG: // Write configuration register groups A & B
//bmsspiall.configreg[0] |= 0x00F8;	// Why are bits going to zero?
		bmsspi_rw_cmd(WRCFGA, &bmsspiall.configreg[0],1);
//bmsspiall.configreg[3] |= 0x000F;			
		bmsspi_rw_cmd(WRCFGB, &bmsspiall.configreg[3],1);
		break;

	case WRITESREG: // Write S Control Register Group
		bmsspi_rw_cmd(WRSCTRL, &bmsspiall.sreg[0],1);
		break; 

	default:
		morse_trap(252);
	}
	return;
}
/* *************************************************************************
 * void bmsspi_readstuff(uint8_t code);
 * @brief	: Do conversion, then read registers with results
 * @brief   : code = code for selection of register group
 * *************************************************************************/
void bmsspi_readstuff(uint8_t code)
{
	switch (code)
	{
	case READCELLSGPIO12: // Read cell voltages + GPIO1 & GPIO2
		readreg(ADCVAX, bmsspiall.cellreg, cmdv, 6);
		readreg(0, bmsspiall.auxreg, &cmdv[6], 1);
		break;

	case READGPIO: // Read all 9 GPIOs voltage registers: A-D
		readreg(ADAX, bmsspiall.auxreg, cmdaux, 4);
		break;

	case READSTAT: // Read status
		readreg(ADSTAT, bmsspiall.statreg, cmdstat, 2);
		break;

	case READCONFIG: // Read configuration
		readreg(0, bmsspiall.configreg,cmdconfig,2);

	case READSREG: // Read S register
		readreg(0, bmsspiall.sreg,cmdsreg,1);
		break;	

	case READAUX:
		readreg(ADAX, bmsspiall.auxreg, cmdaux,4);
		break;			

	default: 
		morse_trap (253);
	}

	/* Signal 'main.c' that there is a bms reading. */
	readbmsflag = 1;
	// Upon this return ADCTask will notify requester.
	return;
}
/* *************************************************************************
 * void bmsspi_rw_cmd(uint16_t cmd, uint16_t* pdata, uint8_t rw);
 * @brief	: Send command  and write data (little endian)
 * @param   : cmd = 2 byte command (little endian)
 * @brief   : pdata = pointer to six bytes to be written (little endian),
 *          :    ignore pdata for read commands.
 * @brief   : rw = type of command sequence
 *          : 0 = Send 2 byte command  + pec
 *          : 1 = Send command+pec, plus 6 bytes data+pec
 *          : 2 = Send command+pec, read 6 bytes data+pec into spirx12.uc[4]-[11]
 *          : 3 = Send 2 byte command  + pec. Switch '1818 SDO to interrupt conversion completion
 * *************************************************************************/
void bmsspi_rw_cmd(uint16_t cmd, uint16_t* pdata, uint8_t rw)
{
	uint32_t noteval; // Wait notification bits

	// Number of bytes for DMA transfer
	uint32_t dmact = 12; // Default size: cmd/pec + 6bytes/pec

	/* Do this early since time delay involved. */
	CSB_GPIO_Port->BSRR = (CSB_Pin<<16); // Reset: CSB pin set low

	rwtype = rw; // Save for interrupt routine use.

	/* Build byte array for a single SPI/DMA operation. */
	// Command: 2 bytes plus two byte pec15
    CRC->CR = 0x9; // 16b poly, + reset CRC computation
	spitx12.u16[0] = (uint16_t)__REV16 (cmd);	// Command: big endian
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 ( spitx12.u16[0]); 
	spitx12.u16[1] = (uint16_t)__REV16 (CRC->DR); // Big endian: Store PEC for command bytes

	/* Set dma count (dmact), and register to write (endian and pec15) */
	switch (rw)
	{
	case 0: // Send command only
	case 3: // Send command and wait for conversion complete (SDO or timeout)
		dmact = 4;
		break;

	case 1: // Send command and write 6 data+2 pec
if (pdata == NULL)	 morse_trap(260);
		//* Set data as big endian 1/2 words, plus PEC15 big endian
 		CRC->CR = 0x9; // 16b poly, + reset CRC computation

 		spitx12.u16[2] =  (*(uint16_t*)(pdata+0)); // Load 2 bytes
    	*(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 ( spitx12.u16[2]);

		 spitx12.u16[3] = (*(uint16_t*)(pdata+1)); // Load 2 bytes
    	*(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 ( spitx12.u16[3]);

		 spitx12.u16[4] = (*(uint16_t*)(pdata+2)); // Load 2 bytes
    	*(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 ( spitx12.u16[4]);

    	 spitx12.u16[5] = (uint16_t)__REV16 (CRC->DR); // Big endian: Store data bytes PEC
    	break;

    case 2: // Send command that continues with reading 8 bytes into spirx12
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
//	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel 	

	timstate = TIMSTATE_1;

	/* Allow 5 us CSB (chip select) delay. Before enabling SPI. */
	TIM15->CCR1 = TIM15->CNT + CSBDELAYFALL; 

	/* Wait for interrupt driven sequence to complete. */
	xTaskNotifyWait(0,0xffffffff, &noteval,5000);
	if (noteval != BMSTSKNOTEBIT00) morse_trap(751);

	return;
}	
/* #######################################################################
 * void bmsspi_tim15_IRQHandler(void);
 * @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
void bmsspi_tim15_IRQHandler(void)
{
	struct BMSSPIALL* p = &bmsspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;

	/* Clear interrupt flag. */
	TIM15->SR = ~(1 << 1); // Bit 1 CC1IF: Capture/Compare 1 interrupt flag

	switch (timstate)
	{
	case TIMSTATE_IDLE: // Idle. 
		CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high
 	 	return;

 	case TIMSTATE_1: // CSB falling delay expired. Read to send
 	 	// Start sending write command + data
 		hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel 	
		// Bit 6 SPE: SPI enable
		SPI1->CR1 |= (1 << 6);
		// SPI rx interrupt expected next.	

timstate = TIMSTATE_TO; 
 	 	return;

case TIMSTATE_TO:
morse_trap(41);
	break; 	 	

 	case TIMSTATE_2: // CSB rising delay expired
		timstate = TIMSTATE_IDLE; // Set to idle, jic	
//morse_trap(9);		
		// Notify ADCTask.c
		xTaskNotifyFromISR(BMSTaskHandle, BMSTSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );		
		return;

	case TIMSTATE_3: // EXTI wait for rising EXTI timed out
		tim15ctr -= 1;
		if (tim15ctr > 0) break;
		p->err |= 1; 

//		GPIOB->MODER |= (2 << 8); 

		CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

		// Disable EXTI4 interrupting
		EXTI->IMR1 &= ~(1<<4);
		EXTI->EMR1 &= ~(1<<4);
		EXTI->PR1   =  (1<<4); // Reset request: PB4

//morse_trap(8); // EXTI interrupt wait timed out

		// Notify ADCTask.c
		xTaskNotifyFromISR(BMSTaskHandle, BMSTSKNOTEBIT00, eSetBits, &xHPT );
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
void bmsspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx)
{
//	SPI1->CR1 &= ~(1 << 6); // Disable SPI

	// Reset DMA interrupt flags	
	hdma_spi1_rx.DmaBaseAddress->IFCR = 0xfff;

	if (rwtype == 3)
	{ // Here, end of commands that starts conversions. 
		/* Setup SPI MISO ('1818 SDO) to interrupt when it goes high. */
		// Enable interrupt & event
		EXTI->IMR1  |= (1<<4); // Interrupt mask register
		EXTI->EMR1  |= (1<<4); // Event mask register
		EXTI->PR1   |= (1<<4); // Reset Pending register JIC

		/* Change PB4 to input mode. */
//		GPIOB->MODER &= ~(3 << 8); 
		
	/* Timeout: JIC, and debugging, and whatever... */
		timstate = TIMSTATE_3; // Count TIM15 16b urnovers
		tim15ctr = 2;// ~(2 * 4.096)ms (16 MHz clock)

		// Expect next interrupt is SPI MISO pin going high
		// CSB remains low and '1818 pulls SDO (MISO) low
		// until conversion is complete, then raises SDO
		// which will cause a rising edge EXTI interrupt.

		// Expect EXTI4 interrupt when conversion completes
		// TIM15 (TIMSTATE_3) timeout if there is a failure.
		return;
	}

	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

	// Assure CSB minimum pulse width before next SPI operation.
	timstate = TIMSTATE_2;
	TIM15->CCR1 = TIM15->CNT + CSBDELAYRISE; 

	return;
}
/* #######################################################################
 * SPI tx dma transfer complete (if enabled!)
   ####################################################################### */
void bmsspi_spidmatx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_tx)
{
	morse_trap (281); // Bogus tx interrupt
	return;
}
/* #######################################################################
 * void EXTI4_IRQHandler(void);
 * PB4 SPI MISO pin-1818 SDO pin: interrupt upon rising edge
   ####################################################################### */	
void EXTI4_IRQHandler(void)
{
// Conversion time?	
dbgt2 = DTWTIME - dbgt1;
morse_trap(888);	

	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

	// Disable interrupt & event
	EXTI->IMR1 &= ~(1<<4);
	EXTI->EMR1 &= ~(1<<4);
	EXTI->PR1   =  (1<<4); // Reset request: PB4

	// Assure CSB minimum pulse width before next SPI operation.
	timstate = TIMSTATE_2;
	TIM15->CCR1 = TIM15->CNT + CSBDELAYRISE; 

	// Next TIM15 interrupt signals completion
	return;
}