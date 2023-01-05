/******************************************************************************
* File Name          : CanCommTask.h
* Date First Issued  : 11/06/2021
* Description        : Can communications
*******************************************************************************/
#ifndef __CANCOMMTASK
#define __CANCOMMTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "cancomm_items.h"
#include "BQTask.h"
#include "ADCTask.h"

/* xTaskNotifyWait Notification bits */
#define CANCOMMBIT00 (1 <<  0) // Send cell voltage  command
#define CANCOMMBIT01 (1 <<  1) // Send misc reading command
#define CANCOMMBIT02 (1 <<  2) // Multi-purpose incoming command 1
#define CANCOMMBIT03 (1 <<  3) // BMSREQ_Q [0] complete: heartbeat
#define CANCOMMBIT04 (1 <<  4) // BMSREQ_Q [1] complete: Send cell voltage
#define CANCOMMBIT05 (1 <<  5) // BMSREQ_Q [2] complete: Send misc reading command
#define CANCOMMBIT06 (1 <<  6) // BMSREQ_Q [3] complete: Multi-purpose incoming command 1
#define CANCOMMBIT07 (1 <<  7) // Send cell voltage  command
#define CANCOMMBIT08 (1 <<  8) // Send misc reading command
#define CANCOMMBIT09 (1 <<  9) // Multi-purpose incoming command 2
#define CANCOMMBIT10 (1 << 10) // BMSREQ_Q [4] complete: Multi-purpose incoming command 2

/* *************************************************************************/
 void CanComm_init(struct BQFUNCTION* p );
/*	@brief	: Task startup
 * *************************************************************************/
 TaskHandle_t xCanCommCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: CanCommHandle
 * *************************************************************************/
 void CanComm_qreq(uint32_t reqcode, uint8_t idx, uint32_t notebit);
/*	@brief	: Queue request to BMSTask.c
 *  @param  : reqcode = see BMSTask.h 
 *  @param  : idx = this queue request slot
 *  @param  : notebit = notification bit when BMSTask completes request5
 * *************************************************************************/

extern TaskHandle_t CanCommHandle;
extern uint8_t rdyflag_cancomm;  // Initialization complete and ready = 1
extern float fbms[ADCBMSMAX]; // (16+3+1) = 20; Number of MAX14921 (cells+thermistors+tos)

#endif