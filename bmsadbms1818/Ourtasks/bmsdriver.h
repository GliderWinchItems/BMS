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
	uint32_t setfets;      // Discharge FET settings
	uint16_t cellreg[6*3]; // Cell readings
	uint16_t auxreg [4*3]; // Aux readings
	uint16_t statreg[1*3]; // Status readings
	uint16_t configreg[2*3]; // Configuration register
	uint16_t sreg[1*3];  // S register
	
	uint8_t  reqcode;
	uint8_t  bmssetfets; // Discharge FET bits to be set 
};

struct BMSUSER
{
	float cellv[NCELLMAX]; // Cell voltage (calibrated volts) 
	float current;   // Op-amp sensing: amps current
	float temperature[3]; // Thermistors: Deg C temperature
	uint32_t dchgfetbits; // 1 = FET is on
	uint32_t cellopenbits;// 1 = cell is unexpectedly open 
	uint8_t err;
};

/* *************************************************************************/
osThreadId xBMSTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BMSTaskHandle
 * *************************************************************************/

extern TaskHandle_t BMSTaskHandle;// replace: extern osThreadId BMSTaskHandle;

extern osMessageQId BMSTaskReadReqQHandle;

extern SemaphoreHandle_t semphrbms;

extern struct BMSSPIALL bmsspiall;
extern struct BMSUSER bmsuser;
{
	
};

#endif
