/******************************************************************************
* File Name          : ADCTask.h
* Date First Issued  : 08/22/2021
* Board              : BMScable: STM32L431
* Description        : ADC w DMA using FreeRTOS/ST HAL
*******************************************************************************/

#ifndef __ADCTASK
#define __ADCTASK

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "adcparams.h"
#include "adcspi.h"
#include "adc_idx_v_struct.h"

// Task notification bit for end of sequence (adctask.c)
#define TSKNOTEBIT00	(1 << 0)  

#define TIM15MHZ 16  // Timer rate MHz
#define DELAY8MS  (TIM15MHZ * 1000 * 8) // 8 millisecond TIM15 OC increment
#define DELAYUS    TIM15MHZ // 1 microsecond TIM15 OC increment

/* Number of TIM15 ticks for SPI to send 3 bytes. */
// (16 MHz timer / 2 MHz SPI clk) * 24 bits
#define SPIDELAY ((16/2)*24)
enum TIMSTATE
{
	TIMSTATE_IDLE,
	TIMSTATE_8MSTO,
	TIMSTATE_SMPL,
	TIMSTATE_2,
	TIMSTATE_3,
	TIMSTATE_N,
	TIMSTATE_OPENCELL,
	TIMSTATE_TRAP,

	SPISTATE_CALIB1,
	SPISTATE_CALIB2,
	SPISTATE_IDLE,
	SPISTATE_FETS,
	SPISTATE_OPENCELL,
	SPISTATE_LOWPOWER,
	SPISTATE_TRAP
};
enum ADCSTATE
{
	ADCSTATE_IDLE,
	ADCSTATE_1,
	ADCSTATE_2,
	ADCSTATE_3,
	ADCSTATE_TRAP
};

/* Conglomerates variables for this mess. */
struct ADCSPIALL
{
	union SPI24 spitx24; // SPI command sent to BMS
 	union SPI24 spirx24; // SPI monitor received from BMS
	uint32_t delayct;
	uint16_t raw[ADCBMSMAX]; // Raw readings from BMS sequence
	uint8_t cellnum;
//	uint8_t adcstate;  // State for ISR handling: ADC
	uint8_t timstate;  // State for ISR handling: TIM
//	uint8_t spistate;  // State for ISR handling: SPI
	uint8_t adcidx;
	uint8_t spiidx;
	uint8_t adcflag;    // 1 = adc busy; 0 = adc idle
	uint8_t adcrestart; // ADC conversion start initiated by: 0 = TIM2; 1= ADC
//	uint8_t readyflag;  //
	uint8_t updn;  // Readout "up" (cells 1->16) = 1; Down (cells 16->1) = 0
	uint8_t config;    // Current ADC configuration: 0 = DMA; 1 = BMS; 2 = not configured
	uint8_t noverlap;  // Overlapping SPI with ADC: 0 = overlapped; 1 = not overlapped
	int8_t  tim15ctr;   // TIM15CH1:OC turnover counter
};

/* Request Codes for ADCTask services. */
#define REQ_BOGUS    0  // Trap mistakes
#define REQ_READBMS  1	// Read MAX1921 cells, thermistor, Top-of-stack
#define REQ_CALIB    2  // Execute a self-calib (MAX14921 Aout offset) cycle
#define REQ_OPENCELL 3  // Do an open cell wire test
#define REQ_LOWPOWER 4  // Place MAX14921 into low power mode.
#define REQ_SETFETS  5  // Set cell discharge FETs 

/* Queue is a pointer to the following for requesting a "service". */
struct ADCREADREQ
{
	osThreadId	taskhandle; // Requesting task's handle
	BaseType_t  tasknote;   // Requesting task's notification bit
	float*      taskdata;   // Requesting task's pointer to buffer to receive data
	uint32_t    cellbits;   // Depends on command: FET to set; Open cell wires
	uint8_t     updn;       // see above 'struct ADCSPIALL'
	uint8_t     reqcode;    // Code for service requested
	uint8_t     noverlap;   // see above
};

/* *************************************************************************/
osThreadId xADCTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: ADCTaskHandle
 * *************************************************************************/

//extern osThreadId ADCTaskHandle;
extern TaskHandle_t ADCTaskHandle;

// Read request queue
extern osMessageQId ADCTaskReadReqQHandle;

extern struct ADCSPIALL adcspiall;

extern struct ADCREADREQ* pssb; // Pointer to struct for request details

#endif

