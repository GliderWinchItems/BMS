/******************************************************************************
* File Name          : BMSTask.c
* Date First Issued  : 06/19/2022
* Board              : bmsbms1818: STM32L431
* Description        : Respond to queued read/write/etc requests
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "BMSTask.h"
#include "bmsspi.h"
#include "morse.h"

#include "main.h"
#include "DTW_counter.h"
#include "iir_f1.h"
#include "bms_items.h"
#include "rtcregs.h"


struct BMSSPIALL bmsspiall;
struct EXTRACTCONFIGREG extractconfigreg;
struct EXTRACTSTATREG   extractstatreg;

/* Queue */
#define QUEUESIZE 16	// Total size of bcb's tasks can queue up
osMessageQId BMSTaskReadReqQHandle;

struct BMSREQ_Q* pssb; // Pointer to struct for request details

struct BMSREQ_Q req_ka; // Keepawake dummy request

TaskHandle_t BMSTaskHandle;

uint32_t bmsdbctr; // Debug counter

uint8_t rate_last; // ADC rate code, last set

/* '1818 Wakeup keep-alive */
#define WAKETICKS 1000 // 1 sec of FreeRTOS timer ticks
TickType_t tickref;
TickType_t tickwait;

/* *************************************************************************
 * void StartBMSTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
uint8_t dbgka;
void StartBMSTask(void *argument)
{
	BaseType_t ret;

	/* Execute a non-interrupt driven '1818 wake up sequence. */
	bmsspi_wakeseq(); // Delay will be ~8 ms

	/* Initialize configuration registers A & B. */
	bmsspi_readstuff(READCONFIG);
	bms_items_cfg_int(); // Initialize (memory) configreg bits
	bmsspi_writereg(WRITECONFIG); // Write configreg to '1818'

	/* Dummy request for keepawake. */
	req_ka.setfets  = 0; // Discharge fet bits off
    req_ka.noteyes  = 0; // We will NOT wait for notification
    req_ka.rate     = RATE26HZ; // RATE422HZ // RATE7KHZ; // ADC rate code

	tickref= xTaskGetTickCount();

   	/* Infinite loop */
  	for(;;)
  	{
  		/* Once per second with no slippage due to loop delay. */
  		tickref += pdMS_TO_TICKS(WAKETICKS);
  		tickwait = tickref - xTaskGetTickCount();
  		if (tickwait > pdMS_TO_TICKS(WAKETICKS))
  		{
  			tickref -= pdMS_TO_TICKS(WAKETICKS);
  			tickwait = tickref - xTaskGetTickCount();
  		}

  		/* Check queue of loaded items. */
		ret = xQueueReceive(BMSTaskReadReqQHandle,&pssb,tickwait);
		if (ret == pdPASS)
		{ // Request arrived
			if (pssb->rate > 7)
			{ // Here rate request not in selection table
				pssb->rate = 4; // Normal (7 KHz mode)
morse_trap(808);
			}

			/* Execute request. */
			switch (pssb->reqcode)
			{
			case REQ_BOGUS: // JIC debug
				morse_trap(812);
				break;
			case REQ_READBMS:   // Read ADBMS1818
				bmsspi_readbms();
				break;
//			case REQ_CALIB:    
//				bmsspi_calib();
//				break;
//			case REQ_OPENCELL:// Do an open cell wire test
//				bmsspi_opencell();
//				break;
//			case REQ_LOWPOWER:// Place into low power mode.
//				bmsspi_lowpower();
//				break;
			case REQ_SETFETS: // Set discharge FETs
				bmsspi_setfets();
				break;
			case REQ_TEMPERATURE:
				bms_gettemp();
				break;		
			case REQ_RTCREAD:   // Read & check PEC15 on RTC registers
				pssb->other = rtcregs_read();
				break;		
			case REQ_RTCUPDATE: // Update w PEC15 RTC registers
				rtcregs_update();
				break;		
			default:
				morse_trap(806); // Debugging trap
				break;
			}
			/* Requesting task may not wait, but polls 'done' to signal completion. */
			pssb->done = 0; // Show request completed

			/* Requesting task may be waiting and needs to be notified. */
			if (pssb->noteyes != 0)
			{
				ret = xTaskNotify(pssb->bmsTaskHandle, pssb->tasknote, eSetBits);
 				if (ret != pdPASS) morse_trap(247);
 			}
	  	}
	  	else
	  	{ // Timeout waiting for queued request. Execute a keep-awake command. */
// Debug: dbgka used by main.c to clocking display	  
//			pssb->done = 1; // Show request queued
	  		dbgka = bmsspi_keepawake();
//	  		pssb->done = 0; // Show request completed
// Running count, debugging
bmsdbctr += 1;	  	
	  	}
	}
}

/* *************************************************************************
 * osThreadId xBMSTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BMSTaskHandle
 * *************************************************************************/
osThreadId xBMSTaskCreate(uint32_t taskpriority)
{
	BaseType_t xRet;

	xRet = xTaskCreate(
		StartBMSTask,     /* Function that implements the task. */
		"BMSTask",        /* Text name for the task. */
		(256+128),              /* Stack size in words, not bytes. */
		NULL,             /* Parameter passed into the task. */
		taskpriority,     /* Priority at which the task is created. */
		&BMSTaskHandle ); /* Used to pass out the created task's handle. */ 

	if( xRet != pdPASS )return NULL;

	BMSTaskReadReqQHandle = xQueueCreate(QUEUESIZE, sizeof(struct BMSREQ_Q*) );
	if (BMSTaskReadReqQHandle == NULL) return NULL;

   	return BMSTaskHandle;	
}
