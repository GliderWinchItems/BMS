/******************************************************************************
* File Name          : bmsdriver.h
* Date First Issued  : 06/20/2022
* Board              : bmsbms1818: STM32L431
* Description        : Respond to queued read/write/etc requests
*******************************************************************************/
#ifndef __BMSDRIVER
#define __BMSDRIVER

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "adcparams.h"
#include "main.h"
#include "bmsspi.h"
#include "bq_idx_v_struct.h"
#include "semphr.h"

#define TSKNOTEBIT00	(1 << 0) // Task notification bit

/* SPI 12 bytes: CMD[2]+PEC[2]+DATA[6]+PEC[2] */
union SPI12
{
	uint8_t   u8[12];
	uint16_t u16[6];
	uint32_t u32[3];
};

/* Request Codes for services. */
#define REQ_BOGUS    0  // Trap mistakes
#define REQ_READBMS  1	// Read ADBMS1818 cells + GPIO1,2
#define REQ_CALIB    2  // Execute a self-calib (MAX14921 Aout offset) cycle
#define REQ_OPENCELL 3  // Do an open cell wire test
#define REQ_LOWPOWER 4  // Set low power mode.
#define REQ_SETFETS  5  // Set cell discharge FETs 
#define REQ_TEMPERATURE 6  // Read & calibrate GPIO temperature sensors

struct BMSSPIALL
{
	TaskHandle_t bmsTaskHandle;
	union SPI12 spitx12; // SPI command sent to '1818'
 	union SPI12 spirx12; // SPI monitor received from '1818'
	uint32_t setfets;      // Discharge FET settings
	uint16_t tim15ctr;     // Count TIM15 overflows
	uint16_t cellreg[6*3]; // Cell readings
	uint16_t auxreg [4*3]; // Aux readings
	uint16_t statreg[1*3]; // Status readings
	uint16_t configreg[2*3]; // Configuration register
	uint16_t sreg[1*3];    // S register
	uint8_t  timstate;     // State for TIM15
	uint8_t  reqcode;      // Request 
	uint8_t  bmssetfets;   // Discharge FET bits to be set 
	uint8_t  err;
};

/* *************************************************************************/
void bmsdriver(uint8_t reqcode);
/*	@brief	: Perform a bms function, e.g. read cells
 *  @param  : code = code(!) for function
 * *************************************************************************/

extern SemaphoreHandle_t semphrbms;

extern struct BMSSPIALL bmsspiall;
extern struct BMSUSER bmsuser;

#endif
