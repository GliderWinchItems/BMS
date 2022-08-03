/******************************************************************************
* File Name          : BMSTask.h
* Date First Issued  : 06/19/2022
* Board              : bmsbms1818: STM32L431
* Description        : Respond to queued read/write/etc requests
*******************************************************************************/
#ifndef __BMSTASK
#define __BMSTASK

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "adcparams.h"
#include "main.h"
#include "bq_items.h"

#define BMSTSKNOTEBIT00 (1<<0)

/* SPI 12 bytes: CMD[2]+PEC[2]+DATA[6]+PEC[2] */
// Note: uint16_t are sent/received big endian
union SPI12
{
	uint8_t   u8[12];
	uint16_t u16[6];
	uint32_t u32[3];
};

/* Request Codes for services. */
#define REQ_BOGUS    0  // This is a bogus request code trap
#define REQ_READBMS  1	// Read ADBMS1818 cells + GPIO1,2
#define REQ_CALIB    2  // Execute a self-calib (MAX14921 Aout offset) cycle
#define REQ_OPENCELL 3  // Do an open cell wire test
#define REQ_LOWPOWER 4  // Set low power mode.
#define REQ_SETFETS  5  // Set cell discharge FETs 
#define REQ_TEMPERATURE 6  // Read & calibrate GPIO temperature sensors
#define REQ_RTCREAD     7  // Read & check PEC15 on RTC registers
#define REQ_RTCUPDATE   8  // Update w PEC15 RTC registers

struct EXTRACTSTATREG
{
	float     sc;   // Sum of Cells (volts)
	float   itmp;   // Internal die temperature (deg C)
	float     va;   // Analog supply voltage (volts)
	float     vd;   // Digital supply (volts)
	uint32_t cov;   // Overvoltage bits 
	uint32_t cuv;   // Undervoltage bits
	uint8_t  rev;   // Revision code
	uint8_t  muxfail; 
	uint8_t  thsd;  // 1 = thermal shutdown 
};

struct EXTRACTCONFIGREG
{  // extract settings
	float    vov;   // Readback of overvoltage setting
	float    vuv;   // Readback of undervoltage setting
	uint32_t dcc;   // Discharge cell bits: 1 = ON
	uint16_t gpio;  // 9 GPIO's: 0 = pin at logic 0;
	uint16_t cfbr1; // bits: MUTE,FDRF,PS1,PS0,DTMEN,DCC0,0,0
};

/* Struct (mostly) holds register group readings from '1818 */
// Note: 'reg arrays are converted to little endian
struct BMSSPIALL
{
	uint16_t configreg[2*3]; // Configuration regs: A B
	uint16_t cellreg[6*3]; // Cell volt regs: A B C D E F
	uint16_t auxreg [4*3]; // Aux regs: A B C D
	uint16_t statreg[2*3]; // Status regs: A B
	uint16_t commreg[1*3]; // COMM register
	uint16_t sreg[1*3];    // S register
	uint16_t pwmreg[2*3];  // PWM regs: A & B
	uint8_t  err1ct;       // Count number of times readreg loop required extra pass
	uint8_t  err;          // TODO?
};
/* Queue holds pointers to the following struct for requesting a "service". */
struct BMSREQ_Q
{
	TaskHandle_t bmsTaskHandle; // Requesting task's handle
	BaseType_t  tasknote; // Requesting task's notification bit
	uint32_t setfets;     // Discharge FET settings
	uint8_t done;         // 0 = done; 1 = busy;
	uint8_t noteyes;      // 1 = Notify requesting task when complete
	uint8_t reqcode;      // Code for specific service requested
	int8_t other;        // BMSTask sends back something to requester.
};	

/* *************************************************************************/
osThreadId xBMSTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BMSTaskHandle
 * *************************************************************************/

extern TaskHandle_t BMSTaskHandle;// replace: extern osThreadId BMSTaskHandle;

extern osMessageQId BMSTaskReadReqQHandle;

extern struct BMSSPIALL bmsspiall;
extern struct EXTRACTSTATREG extractstatreg;
extern struct EXTRACTCONFIGREG extractconfigreg;

extern struct BMSREQ_Q* pssb; // Pointer to struct for request details


#endif
