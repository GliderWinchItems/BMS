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
#include "main.h"
#include "FanTask.h"
#include "BMSTask.h"
#include "DTW_counter.h"
#include "ADBMS1818_command_codes.h"
#include "fetonoff.h"
#include "pec15_reg.h"
#include "bms_items.h"
#include "morse.h"

/* Uncomment to enable EXTI4 MISO/SDO conversion detection code. */
#define USESDOTOSIGNALENDCONVERSION

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

//extern	TaskHandle_t bmsTaskHandle; // Calling task's task handle
extern struct BMSSPIALL bmsspiall;

 union SPI12 spitx12; // SPI command sent to '1818'
 union SPI12 spirx12; // SPI monitor received from '1818'
static uint8_t  timstate;   // State for ISR handling: TIM

uint32_t dbgt1,dbgt2;

void bmsspi_readstuff(uint8_t code);
void bmsspi_rw_cmd(uint16_t cmd, uint16_t* pdata, uint8_t rw);
static uint16_t readreg(uint16_t cmdx, uint32_t wait, uint16_t* p, const uint16_t* pcmdr, uint8_t n);

enum TIMSTATE
{
	TIMSTATE_IDLE,
	TIMSTATE_1,
	TIMSTATE_2,
	TIMSTATE_3OVR,
	TIMSTATE_3,
	TIMSTATE_TO,
};

#define EXDLY 0 // Extra delay increment (debugging/test)
#define CSBDELAYFALL (6+EXDLY) // CSB falling delay: 6 us with 1 MHz timer clock
#define CSBDELAYRISE (5+EXDLY) // CSB rising delay: 5 us with 1 MHz timerclock
#define DELAYCONVERTMIN (5+EXDLY) // Minimum end of conversion command delay

static uint8_t rwtype; // code: command, command + data, etc.

#define READOUTSIZE_ADVAX 7 // Number of registers to read 
// Read commands for registers to be read
static const uint16_t cmdv[READOUTSIZE_ADVAX] = {
	RDCVA, /* 0x004  // 0 Read Cell Voltage Register Group A */
	RDCVB, /* 0x006  // 1 Read Cell Voltage Register Group B*/
	RDCVC, /* 0x008  // 2 Read Cell Voltage Register Group C*/
	RDCVD, /* 0x00A  // 3 Read Cell Voltage Register Group D*/
	RDCVE, /* 0x009  // 4 Read Cell Voltage Register Group E*/
	RDCVF, /* 0x00B  // 5 Read Cell Voltage Register Group F*/
	RDAUXA,/* 0x00C  // 6 Read Auxiliary Register Group A   */
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

/* Conversion delays for various commands. */
static const uint32_t conv_delayADCV[8] = {
	12816,   /*  422 Hz */
	7230,    /*  1KHz   */
	1121,    /* 27KHz   */
	1296,    /* 14KHz   */
	2343,    /*  7KHz   */
	3041,    /*  3KHz   */
	206325,  /*   26 Hz */
	4437,    /*  2KHz   */
};
static const uint32_t conv_delayADAX[8] = {
	21316,   /*  422 Hz */
	12007,   /*  1KHz   */
	1825,    /* 27KHz   */
	2116,    /* 14KHz   */
	3862,    /*  7KHz   */
	5025,    /*  3KHz   */
	335498,  /*   26 Hz */
	7353,    /*  2KHz   */
};
static const uint32_t conv_delayADCVAX[8] = {
	17104,   /*  422 Hz */
	9657,    /*  1KHz   */
	1511,    /* 27KHz   */
	1744,    /* 14KHz   */
	3140,    /*  7KHz   */
	4071,    /*  3KHz   */
	268450,  /*   26 Hz */
	5933,    /*  2KHz   */
};
static const uint32_t conv_delayADSTAT[8] = {
	8538,   /*  422 Hz 000 */
	4814,    /*  1KHz  001 */
	 742,    /* 27KHz  010 */
	 858,    /* 14KHz  011 */
	1556,    /*  7KHz  100 */
	2022,    /*  3KHz  101 */
  134211,    /*  26 Hz 110 */
	2953,    /*  2KHz  111 */
};


/* Skip readouts that are too close together. */
#define MINREADOUT (16*9000) // 9.00 ms
// System ticks of last reading. 
uint32_t time_read_cells; 
uint32_t time_read_gpio;  
uint32_t time_read_aux; 

uint8_t readbmsflag; // Let main know a BMS reading was made

/* *************************************************************************
 * void bmsspi_setrateADOPT(void);
 * @brief	:
 * *************************************************************************/
void bmsspi_setrateADOPT(void)
{
//	if (pssb->rate != rate_last)
	{ // Here, request changes the rate
//		if ((pssb->rate & 0x1) != (rate_last & 0x1))
		{ // Here, ADCOPT bit in CFGAR0 needs a re-write
			rate_last = (pssb->rate & 0x1);
			bmsspiall.configreg[0] &= ~(0x1 << 0);
			bmsspiall.configreg[0] |= (pssb->rate & 0x1);
			bmsspi_rw_cmd(WRCFGA, &bmsspiall.configreg[0],1);
		}
	}
	return;
}

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

	/* Skip readout if previous request was too soon. */
	if ((int)(DTWTIME - time_read_cells) < 0)
		return;

	/* Turn heater, dump, trickle chgr off,
		if dump2 (external charger) is on, leave
		it turned on. */
	if ((bqfunction.fet_status & FET_DUMP2) != 0)
	{ // DUMP2 stays on. Turn all others off.
		fetonoff_status_set(FET_DUMP2);
	}
	else
	{ // DUMP2 is off.
		fetonoff_status_set(0);
	}

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
//	bmsspi_setfets();

	// Restore heater, dump, dump2, trickle chgr
	fetonoff_status_set(bqfunction.fet_status);

	// Update the time of the last reading
	time_read_cells = DTWTIME + MINREADOUT;
	return;
}
/* *************************************************************************
 * void bmsspi_readstuff(uint8_t code);
 * @brief	: Do conversion, then read registers with results
 * @param   : code = code for selection of register group
 * *************************************************************************/
/*
The time conversion delays below (second argument in readreg call) are based 
on the "normal" setting of 7 KHz ADC rate (and system clock of 16 MHz).
*/
uint32_t dbgread1;
uint32_t dbgread2;

void bmsspi_readstuff(uint8_t code)
{
	uint32_t cnvdly;
	uint16_t cmdz;
	uint32_t tmp;

//pssb->rate = 0; // DEBUG: set rate for all requests. 0 = 422 Hz

	/* ADOPT bit is low ord bit of rate selection. */
	bmsspi_setrateADOPT(); // Update ADOPT in CFGRAR0 if necessary
	tmp = ((pssb->rate & 0x6) << 6);

	switch (code)
	{
	/* readreg args:
	  1 - command that "starts" a conversion;, 0 = skip
	  2 - TIM15 wait time (us) for a conversion command to complete
	  3 - pointer to store, or send, 6 byte register group
	  4 - pointer to command array for read or write group
	  5 - number of reads or writes of a group
	*/
	case READCELLSGPIO12: // ADC cell voltages + GPIO1 & GPIO2

dbgread1 = DTWTIME;

		cmdz = ((ADCVAX & ~0x180) | tmp); // Set rate MD bits in command
		cnvdly = conv_delayADCVAX[pssb->rate] + 100; // Look up conversion delay 
		readreg(cmdz, cnvdly, bmsspiall.cellreg, cmdv, 6); // ADC and Read cell volts
//readreg(cmdz, 30000, bmsspiall.cellreg, cmdv, 6); // Debug: experiment with delay
dbgread2 = DTWTIME - dbgread1;
		readreg(     0,       0,bmsspiall.auxreg, &cmdv[6], 1); // Read AUX reg A GPIOs 1-3
		break;

	case READGPIO: // ADC and Read all 9 GPIOs voltage registers: A B C D 
		cmdz = ((ADAX & ~0x180) | tmp); // Set rate MD bits in command
		cnvdly = conv_delayADAX[pssb->rate] + 100;
		readreg(cmdz, cnvdly, bmsspiall.auxreg, cmdaux, 4);
		break;

	case READSTAT: // ADC and Read status regs: A B
		cmdz = ((ADSTAT & ~0x180) | tmp); // Set rate MD bits in command
		cnvdly = conv_delayADSTAT[pssb->rate] + 100;
		readreg(cmdz, cnvdly, bmsspiall.statreg, cmdstat, 2);
		break;

	case READCONFIG: // Read configuration (original delay: 0)
		readreg(     0,        0,bmsspiall.configreg,cmdconfig,2);

	case READSREG: // Read S register (original delay: 0)
		readreg(     0,        0,bmsspiall.sreg,cmdsreg,1);
		break;	

	case READAUX: // ADC and Read all 9 GPIOs voltage registers: A B C D 
		cmdz = ((ADAX & ~0x180) | tmp); // Set rate MD bits in command
		cnvdly = conv_delayADAX[pssb->rate] + 100;
		readreg(cmdz, cnvdly,bmsspiall.auxreg, cmdaux,4);
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
 * void bmsspi_gpio(void);
 * @brief	: Read BMS 9 GPIOs & calibrate temperature sensors
 * *************************************************************************/
void bmsspi_gpio(void)
{
	if ((int)(DTWTIME - time_read_gpio) < 0)
		return;
	bmsspi_readstuff(READGPIO);
	time_read_gpio = DTWTIME + MINREADOUT;

	// TODO: calibration
	return;
}
/* *************************************************************************
 * void bmsspi_setfets(void);
 * @brief	: Load discharge fet settings into '1818 & set discharge timer
 * *************************************************************************/
void bmsspi_setfets(void)
{
	/* Get latest configuration registers A & B */
	readreg(     0,        0,bmsspiall.configreg,cmdconfig,2);

	/* Update selected Group A & B settings with FET settings. */
	// DCC12-DCC1 CFGA5 CFGAR4 | Discharge timer: DCTO
	bmsspiall.configreg[2] = ((pssb->setfets & 0x0FFF) |
		(0x02 << 12)); // DCTO[0-3] code = 1 minute

	// DCC18-DCC12 CFGBR1 CFGBR0 | Dischg timer enable: DTMEN = 1
	bmsspiall.configreg[3] &= ~0x03F0; // Clear bits to be set
	bmsspiall.configreg[3] |= (((pssb->setfets & 0x3F000) >> 8) |
		(1 << 11)); // DTMEN bit

	bmsspiall.configreg[0] |= 0x00F8; // Assure GPIO pulldowns off
	bmsspiall.configreg[3] |= 0x000F;

	/* Write-back configuration Groups A & B. */
	bmsspi_writereg(WRITECONFIG);

	/* Readback FET settings (for debugging and testing) */
	readreg(     0,        0,bmsspiall.configreg,cmdconfig,2);

	return;
}
/* *************************************************************************
 * void bms_gettemp(void);
 * @brief	: Read AUX and get temperatures
 * *************************************************************************/
void bms_gettemp(void)
{
	if ((int)(DTWTIME - time_read_aux) < 0)
		return;
	bmsspi_readstuff(READAUX);
	time_read_aux = DTWTIME + MINREADOUT;
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
 * @brief	: Execute valid commands to keep awake. Sequence through commands
 * @return  : state number of latest reading
 * *************************************************************************/
static uint8_t bmsspi_keepawake_state; // Step thru commands
uint8_t bmsspi_keepawake(void)
{
	uint8_t retx;
	/* Set up dummy request. */
	pssb = &req_ka; 

//bmsspi_keepawake_state = 0; // Debug
	switch(bmsspi_keepawake_state)
	{
	case 0: /* Read config registers A & B. */
		req_ka.reqcode = READCONFIG;
		bmsspi_readstuff(READCONFIG);
		break;
	case 1: /* Read status registers A & B. */
		req_ka.reqcode = READSTAT;
		bmsspi_readstuff(READSTAT);
		break;
	case 2: // Read S Control register. */
		req_ka.reqcode = READSREG;
		bmsspi_readstuff(READSREG);
		break;
	case 3: // Write config register A */
		req_ka.reqcode = WRITECONFIG;
		bmsspi_writereg(WRITECONFIG);
		break;
	case 4:	// Read cells and GPIO1 GPIO2
		req_ka.reqcode = READCELLSGPIO12;
		bmsspi_readstuff(READCELLSGPIO12); 
//		bmsspi_readbms();
		break;
	case 5:
		req_ka.reqcode = READAUX;	
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
for (int i = 0; i < 6; i++) bmsspiall.debugbuffer[i] = 0xdead; 	

	 bmsspiall.err1ct = 0; // Extra loop err ct

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

 #ifdef USESDOTOSIGNALENDCONVERSION 
	/* PC4 EXTI */
//	GPIOC->MODER &= ~(0x3 << 8); // Mode = input

 	/* EXTI SDO/MISO PB4 end of conversion detection interrupt. */
 	EXTI->RTSR1 |= (1<<4); // Rising edge line 4

 	/* Disable interrupt masks. */
	EXTI->IMR1 &= ~(1<<4);
	EXTI->EMR1 &= ~(1<<4);
	EXTI->PR1   =  (1<<4); // Reset pending request: EXTI 4	 	

	/* EXTI4 ('1818 SDO conversion complete) interrupt initialize*/
//  	HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
//  	HAL_NVIC_EnableIRQ  (EXTI4_IRQn);  	
#endif

  	/* Make use of TIM15:CH1 OC for timing BMS delays. */
	// ARR default is 0xFFFF (max count-1)
	timstate    = TIMSTATE_IDLE; // Set to idle, jic
	TIM15->SR   = 0;           // Clear any interrupt flags, jic
	TIM15->DIER = (1 << 0); // Update interrupt enable
//	TIM15->DIER = (1 << 1);  // Bit 1 CC1IE: Capture/Compare 1 interrupt enable
//	TIM15->CR1 |= 1;       // Start counter

	/* Set port/pin for EXTI interrupt line 4. */
	SYSCFG->EXTICR[1] &= ~(0x7 << 0);	
//	SYSCFG->EXTICR[1] |=  (0x1 << 0); // PB4 EXTI
	SYSCFG->EXTICR[1] |=  (0x2 << 0); // PC4 EXTI	

	/* Last reading time */
	time_read_cells = DTWTIME; 
	time_read_gpio  = time_read_cells;  
	time_read_aux   = time_read_gpio; 

	return;
}
/* *************************************************************************
 * static uint16_t readreg(uint16_t cmdx, uint32_t wait, uint16_t* p, const uint16_t* pcmdr, uint8_t n);
 * @brief	: Read register, convert endianness
 * @param   : cmdx = 1st command
 * @param   : p = pointer to in-memory register array
 * @param   : pcmdr = pointer to read command (0 = skip conversion command)
 * @param   ; n = number of registers to read in register group
 * @return  : 0 = no PEC15 error
 * *************************************************************************/
uint32_t dbstat1;
uint32_t dbstat2;
#define QQQCTR 10 // Repetition limit count
uint16_t loopctr;
uint32_t loopctr_save;

uint32_t wait_isr;
uint32_t wait_isr_ovr;

static uint16_t readreg(uint16_t cmdx, uint32_t wait, uint16_t* p, const uint16_t* pcmdr, uint8_t n)
{
	uint16_t i;

	/* Check if a start conversion command */
	if (cmdx != 0)
	{ // "Start conversion" command (waits for conversion)
GPIOC->BSRR = (1<<4); // Set PC4 high
dbstat1 = DTWTIME;	

		wait_isr = wait; // Save for spi rx dma interrupt 	
		if ((wait_isr == 0) && (n == 0))
		{ // Here, command with no wait or readback
			bmsspi_rw_cmd(cmdx, NULL, 0); 
		}
		else
		{ // Here command with or without wait and register readings
			bmsspi_rw_cmd(cmdx, NULL, 3); 
		}
dbstat2 = DTWTIME - dbstat1;		
//morse_trap(4);
	}

	/* Read register group with latest results. */
	for (i = 0; i < n; i++)
	{
		loopctr = 0;
		do
		{	
			/* Send command (with wait for completion) */
			bmsspi_rw_cmd(*pcmdr, NULL, 2);

			// Repeat if all one's was returned
			loopctr += 1; // Limit loops
		} while( (spirx12.u32[1] == 0xffffFFFF) && 
			     (spirx12.u32[2] == 0xffffFFFF) && 
			     (loopctr < QQQCTR) );

		if (loopctr >= QQQCTR) morse_trap(58); // Loop reached limit
		if (loopctr >       1) 
		{
			bmsspiall.err1ct += 1;
//			morse_trap(59); // More than one pass
			if (loopctr > loopctr_save)
			{
				loopctr_save = loopctr;
			}
		}

		/* Check PEC15 matches. */
		// Compute PEC15 in received data
		CRC->CR = 0x9; // 16b poly, + reset CRC computation
		*(__IO uint32_t*)CRC_BASE = (uint32_t)__REV (spirx12.u32[1]);
		*(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 (spirx12.u16[4]); 

		// Compare received PEC to computed PEC
		if ((uint16_t)__REV16 (CRC->DR) != spirx12.u16[5]) 
			morse_trap(68); 
	
		/* Copy 6 byte spi rx to memory array. */
		*p++ = (spirx12.u16[2]);
		*p++ = (spirx12.u16[3]);
		*p++ = (spirx12.u16[4]);	
		/* Point to next read command. */
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
	/* Set pulldown bits OFF since GPIOs are used analog
	   These bits read out logic level, so they might be zero. */
		bmsspiall.configreg[0] |= 0x00F8;
		bmsspi_rw_cmd(WRCFGA, &bmsspiall.configreg[0],1);
		bmsspiall.configreg[3] |= 0x000F;			
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
 * void bmsspi_rw_cmd(uint16_t cmd, uint16_t* pdata, uint8_t rw);
 * @brief	: Send command  and load (write) data (little endian) if rw = 3
 * @param   : cmd = 2 byte command (little endian)
 * @param   : pdata = pointer to six bytes to be written (little endian),
 *          :    ignore pdata for read commands (rw = 2).
 * @param   : rw = type of command sequence
 *          : 0 = Send 2 byte command  + pec
 *          : 1 = Send command+pec, plus 6 bytes data+pec
 *          : 2 = Send command+pec, read 6 bytes data+pec into spirx12.uc[4]-[11]
 *          : 3 = Send 2 byte command  + pec. Wait for conversion completion
 * *************************************************************************/
void bmsspi_rw_cmd(uint16_t cmd, uint16_t* pdata, uint8_t rw)
{
	uint32_t noteval; // Wait notification bits

	// Number of bytes for DMA transfer
	uint32_t dmact = 12; // Default size: cmd/pec + 6bytes/pec

	/* Do this early since time delay involved. */
	CSB_GPIO_Port->BSRR = (CSB_Pin<<16); // Reset: CSB pin set low

	rwtype = rw; // Save for bmsspi_spidmarx_IRQHandler use.

	/* Build byte array for issuing single SPI/DMA operation. */
	// Command: 2 bytes plus two byte pec15
    CRC->CR = 0x9; // 16b poly, + reset CRC computation
	spitx12.u16[0] = (uint16_t)__REV16 (cmd);	// Command: big endian
    *(__IO uint16_t*)CRC_BASE = (uint16_t)__REV16 ( spitx12.u16[0]); 
	spitx12.u16[1] = (uint16_t)__REV16 (CRC->DR); // Big endian: Store PEC for command bytes

	/* Set dma count (dmact), and  load data register if write (endian and pec15) */
	switch (rw)
	{
	case 0: // Send command only
	case 3: // Send command and wait for conversion complete (SDO or timeout)
		dmact = 4;
		break;

	case 1: // Send command and write 6 data+2 pec
if (pdata == NULL) morse_trap(260);
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

	/* Disable SPI1 DMA request. */
	// Bit 1 TXDMAEN: Tx buffer DMA enable
	// Bit 0 RXDMAEN: Rx buffer DMA enable
	SPI1->CR2 &= ~(0x3); 

	/* Setup DMA read and write. */
	hdma_spi1_rx.Instance->CCR &= ~1; // Disable dma spi rx channel
	hdma_spi1_rx.Instance->CNDTR = dmact; // Number to DMA transfer
//	hdma_spi1_rx.Instance->CCR |= 1;  // Enable channel

	hdma_spi1_tx.Instance->CCR &= ~1; // Disable dma spi tx channel
	hdma_spi1_tx.Instance->CNDTR = dmact; // Number to DMA transfer
//	hdma_spi1_tx.Instance->CCR |= 1;  // Enable channel 	

	/* Allow 5 us CSB (chip select) delay. Before enabling SPI. */
	TIM15->CR1 = 0; // Disable timer JIC
	TIM15->CNT = 0; // Counter starts from zero
	TIM15->ARR = CSBDELAYFALL; // Length of count
	timstate   = TIMSTATE_1; 
	// Start counter (CEN), One pulse (OPM). Only overflow (URS)
	TIM15->CR1 = 0xD; // OPM URS CEN

	/* Wait for interrupt driven sequence to complete. */
	xTaskNotifyWait(0,0xffffffff, &noteval,5000);
	if (noteval != BMSTSKNOTEBIT00) morse_trap(751);

	return;
}	
/* #######################################################################
 * void bmsspi_tim15_IRQHandler(void);
 * @brief	: TIM15 interrupt shares with TIM1 break;
   ####################################################################### */
// TIMSTATE_TO; morse_trap(41) debugging helpers:
uint32_t dbgTO; // TIMSTATE_TO: TIM15 duration versus DTWTIME
uint32_t dbgTO_SR; // TIMSTATE_TO: Save SPI status reg 
uint32_t dbgTO_CNT;
uint32_t dbgTO_CCR1;

void bmsspi_tim15_IRQHandler(void)
{
//	struct BMSSPIALL* p = &bmsspiall; // Convenience pointer
	BaseType_t xHPT = pdFALSE;

	/* Clear interrupt flag. */
//	TIM15->SR = ~(1 << 1); // Bit 1 CC1IF: Capture/Compare 1 interrupt flag
	TIM15->SR = ~(1 << 0); // Bit 0 UIF: Update interrupt flag 

	switch (timstate)
	{
	case TIMSTATE_IDLE: // Idle. 
//		CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high
 	 	return;

 	case TIMSTATE_1: // CSB falling delay expired. Read to send
 	 	// Start sending write command + data
 		SPI1->CR2 |= 0x1; // Enable DMA: RXDMAEN
		hdma_spi1_rx.Instance->CCR |= 1;  // Enable DMA rx channel

		SPI1->CR2 |= 0x2; // Enable DMA: TXDMAEN
 		hdma_spi1_tx.Instance->CCR |= 1;  // Enable DMA tx channel 	

		SPI1->CR1 |= (1 << 6); // Enable SPE: SPI enable
		// SPI rx interrupt expected next.	
		timstate = TIMSTATE_TO; // JIC: AND it happened!
		/* Next interrupt will be SPI/DMA */
dbgTO = DTWTIME;		
 	 	return;

 	 /* Trap: SPI started, but wait for interrupt timed out. */
	case TIMSTATE_TO: // One register rollover time delay.
dbgTO_CNT = TIM15->CNT;
//dbgTO_CCR1 = TIM15->CCR1;
dbgTO = DTWTIME - dbgTO; 
dbgTO_SR = SPI1->SR;	
	morse_trap(41); // ARGH! No SPI/DMA rx interrupt.
		break; 	 	

 	case TIMSTATE_2: // CSB rising delay expired
		timstate = TIMSTATE_IDLE; // Set to idle, jic	

		// Notify bmsspi_rw_cmd() that sequence is complete
		xTaskNotifyFromISR(BMSTaskHandle, BMSTSKNOTEBIT00, eSetBits, &xHPT );
		portYIELD_FROM_ISR( xHPT );		
		return;

	case TIMSTATE_3OVR: // Overflow: Timer repetition counter was set
		wait_isr_ovr -= 1;
		if (wait_isr_ovr > 0)
		{
			TIM15->CNT = 0;
			TIM15->CR1 = 0xD;
			return;
		}

		wait_isr &= 0xFFFF; // Subtract number of overflows timed out.
		/* Time out remainder. */
		if (wait_isr < 6)
		{
			TIM15->ARR = 6;
		}
		else
		{
			TIM15->ARR = wait_isr;
		}
		timstate = TIMSTATE_3; // 
		TIM15->CNT = 0;
		TIM15->CR1 = 0xD;
		return;

	case TIMSTATE_3: // Wait for a conversion command timer expiration.
		CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

		/* Disable further EXTI interrupts. */
		EXTI->IMR1 &= ~(1<<4); // Mask interrupt
		EXTI->EMR1 &= ~(1<<4); // Mask interrupt
		EXTI->PR1   =  (1<<4); // Reset pending request: EXTI 4	 

		/* Assure CSB mininum high duration. */
		TIM15->ARR = CSBDELAYRISE;
		TIM15->CNT = 0;
		TIM15->CR1 = 0xD;

		/* TIM15 signals End of sequence. */	
		timstate = TIMSTATE_2; // 
		return;
	
	default: morse_trap(810); 
		break; // Debug jic
	}
	return;
}
/* #######################################################################
 * SPI rx dma transfer complete
   ####################################################################### */
uint32_t dbgexttim1;
uint32_t dbgexttim2;
uint32_t dbgpb4;
uint32_t dbgwait_isr;

void bmsspi_spidmarx_IRQHandler(DMA_HandleTypeDef* phdma_spi1_rx)
{
	/* Close down sequence for SPI/DMA communication. */
	hdma_spi1_tx.Instance->CCR &= ~1; // Disable dma spi tx channel
	hdma_spi1_rx.Instance->CCR &= ~1; // Disable dma spi rx channel
	SPI1->CR1 &= ~(1 << 6); // Disable SPI
	SPI1->CR2 &= ~0x3;      // Disable DMA: TXDMAEN | RXDMAEN
	TIM15->CR1 = 0x0; // JIC timer is running shut it down

	// Reset DMA SPI1 interrupt flags: DMA1: CH2,CH3
	// Note: DMA1: CH1 is ADC1 DMA channel, but interrupt not used
	hdma_spi1_rx.DmaBaseAddress->IFCR = 0xff0;

// Pin for 'scope triggering
//GPIOC->BSRR = (1<<(4+16)); // Set PC4 pin low

	/* ADC conversion cmmands require wait for end-of-conversion handling. */
	if (rwtype == 3)
	{ // Here, end of sending command that started a conversion. 

/*  Use rising SDO to signal end of conversions. */		
#ifdef USESDOTOSIGNALENDCONVERSION 
		/* Setup SPI MISO ('1818 SDO) to interrupt when it goes high. */
		// Enable interrupt & event
		EXTI->PR1   =  (1<<4); // Reset pending request: EXTI 4		
		EXTI->IMR1  |= (1<<4); // Interrupt mask register: not masked
		EXTI->EMR1  |= (1<<4); // Event mask register: not masked

dbgexttim1 = DTWTIME;		
#endif

	/* Timeout: JIC no EXTI4 interrupt, and debugging, and whatever... */

if (wait_isr == 0) morse_trap(801); // What!!
if (wait_isr > 65536)
	dbgwait_isr = (wait_isr >> 16);

		// Time counts greater than 65535 must count turnovers
		if ((wait_isr >> 16) > 0)
		{ // Overflow counting required.
			timstate = TIMSTATE_3OVR; // Long timeout: count TIM15 16b turnovers
			// Set number of overflows before timer "event" triggers
			wait_isr_ovr = (wait_isr >> 16); // Repitition counter
			wait_isr &= 0xFFFF; // Hi-ord is timer overflow, so clear'em.

			TIM15->CNT = 0; // Start count from zero
			TIM15->ARR = 65535; // Max time count
			TIM15->CR1 = 0xD; // Start counter; stop upon "event"
			return;
		}

		// Here, wait_isr is less than one timer turnover
		timstate = TIMSTATE_3; // 

		// Allow a minimum timeout JIC
		if (wait_isr > DELAYCONVERTMIN) 
		{
			TIM15->ARR = wait_isr;
		}
		else
		{ // Who would set such a low conversion timeout?
			morse_trap(802); // Assuredly bogus
		}

		TIM15->CNT = 0;
		TIM15->CR1 = 0xD;

		/* For detecting end-of-conversion using SDO 
		   Expect next interrupt is SPI MISO pin going high
		   CSB remains low and '1818 pulls SDO (MISO) low
		   until conversion is complete, then raises SDO
		   which will (should!) cause a rising edge EXTI interrupt.

		   Expect EXTI4 interrupt when conversion completes
		   TIM15 (TIMSTATE_3) timeout if there is a failure.
		 */
		return;
	}
	/* Here, no waiting for '1818 conversions needed. */
	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

	// Assure CSB minimum pulse width before next SPI operation.
	timstate = TIMSTATE_2;
//	TIM15->CCR1 = (uint16_t)(TIM15->CNT + CSBDELAYRISE); 

	TIM15->ARR = CSBDELAYRISE;
	TIM15->CNT = 0;
	TIM15->CR1 = 0xD;

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
uint32_t dbgexti4ctr;	
void EXTI4_IRQHandler1(void)
{

dbgexttim2 = DTWTIME - dbgexttim1;

#ifndef USESDOTOSIGNALENDCONVERSION  /*  Use rising SDO to signal end of conversions. */		
// Conversion time?	
dbgt2 = DTWTIME - dbgt1;
morse_trap(888);	
#else

dbgexti4ctr += 1;

	CSB_GPIO_Port->BSRR = (CSB_Pin); // Set: CSB pin set high

	/* Disable further EXTI interrupts. */
	EXTI->IMR1 &= ~(1<<4); // Mask interrupt
	EXTI->EMR1 &= ~(1<<4); // Mask interrupt
	EXTI->PR1   =  (1<<4); // Reset pending request: EXTI 4	 	

	// Assure CSB minimum pulse width before next SPI operation.
	timstate = TIMSTATE_3;
	TIM15->CR1 = 0; // Disable interrupts
	TIM15->SR = ~(1 << 0); // Clear interrupt flag if pending
	TIM15->ARR = CSBDELAYRISE;
	TIM15->CNT = 0;
	TIM15->CR1 = 0xD;	
#endif
	// Next TIM15 interrupt signals completion
	return;
}
