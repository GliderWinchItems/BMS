/******************************************************************************
* File Name          : bmsdriver.c
* Date First Issued  : 06/20/2022
* Board              : bmsbms1818: STM32L431
* Description        : Respond to queued read/write/etc requests
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "semphr.h"

#include "bmsdriver.h"
#include "bmsspi.h"
#include "morse.h"

#include "main.h"
#include "DTW_counter.h"
#include "iir_f1.h"

SemaphoreHandle_t semphrbms;
QueueHandle_t queuebms;
#define QUEUESIZE 12 // Number of items in queuue

extern osThreadId_t defaultTaskHandle;

//TaskHandle_t bmsTaskHandle;

struct BMSSPIALL bmsspiall;

uint32_t bmsdbctr;

/* *************************************************************************
 *  void bmsdriver_init(void);
 *	@brief	: Initialize
 * *************************************************************************/
void bmsdriver_init(void)
{
	/* Initialize SPI-DMA CRC ... */
	bmsspi_preinit();

	/* Queue of pointers to bms request. */
	queuebms = xQueueCreate( QUEUESIZE, sizeof(struct BMSREQ_Q*) );
	if (queuebms == NULL) morse_trap(742);

	/* Semaphore for waiting for BMS operation completion. */
	vSemaphoreCreateBinary( semphrbms);
	if (semphrbms == NULL) morse_trap(732);

	return;
}
/* *************************************************************************
 *  void bmsdriver(uint8_t reqcode);
 *	@brief	: Perform a bms function, e.g. read cells
 *  @param  : code = code(!) for function
 * *************************************************************************/
void bmsdriver(uint8_t reqcode)
{
	BaseType_t qret;
	struct BMSREQ_Q* pq;


if (semphrbms == NULL)	morse_trap(738);
if (queuebms  == NULL) morse_trap(743);

	//	qret = xSemaphoreTake(semphrbms, 5000);
	//	if (qret != pdPASS) morse_trap(736);

	do
	{
		qret = QueueReceive(queuebms, &pq, 0);
		if (pq == pdPASS)

		/* Save task handle used for ISR ending a sequence. */
		bmsspiall.bmsTaskHandle = xTaskGetCurrentTaskHandle();

	if (bmsspiall.bmsTaskHandle == NULL) morse_trap(739);

		/* Save for ISR handling. */
		bmsspiall.reqcode = reqcode; 

		/* Execute request. */
		switch (reqcode)
		{
		case REQ_BOGUS: // JIC debug
			morse_trap(811);
			break;
		case REQ_READBMS:   // Read cells + gpio 1 & 2
			bmsspi_readbms();
			break;
	/*	case REQ_CALIB:    // Execute a self-calib
			bmsspi_calib();
			break;
		case REQ_OPENCELL:// Do an open cell wire test
			bmsspi_opencell();
			break;
		case REQ_LOWPOWER:// Place into low power mode.
			bmsspi_lowpower();
			break; 				
	*/
		case REQ_SETFETS: // Set discharge FETs
			bmsspi_setfets();
		case REQ_TEMPERATURE: // Read & calibrate GPIO temperature sensors
			bmsspi_gpio();	
			break;
		default:
			morse_trap(806); // Debugging trap
			break;
		}
		bmsdbctr += 1;
		/* Release semaphore for bmsdriver. */
	if (semphrbms == NULL)	morse_trap(740);	
		qret = xSemaphoreGive(semphrbms);
		if (qret != pdPASS) morse_trap(737);	
	}
	return;
}
