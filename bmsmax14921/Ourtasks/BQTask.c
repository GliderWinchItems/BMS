/******************************************************************************
* File Name          : BQTask.c
* Date First Issued  : 09/08/2021
* Description        : BQ76952 BMS w STM32CubeMX w FreeRTOS
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"

#include "BQTask.h"
#include "BQ769x2Header.h"
#include "bq_idx_v_struct.h"
#include "bq_func_init.h"
#include "fetonoff.h"
#include "bq_items.h"
#include "cancomm_items.h"
#include "CanCommTask.h"
#include "chgr_items.h"


static void morse_sos(void);
static uint8_t getcellv(void);

struct BQFUNCTION bqfunction;


TaskHandle_t BQTaskHandle = NULL;

/* ADC counts for voltage and sync'd current */
union ADCCELLCOUNTS cellcts;


int16_t blk_0x0075_s16[16];

/* Cell balancing */
uint8_t bq_initflag = 0;
uint8_t flagmain;

/* *************************************************************************
 * static void bq_init(struct BQFUNCTION* p);
 *	@brief	: Some initialization before endless loop starts
 * *************************************************************************/
static void bq_init(void)
{
	bq_initflag = 0; // Reset flag	
	return;
}
/* *************************************************************************
 * void StartBQTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/

void StartBQTask(void* argument)
{
	struct BQFUNCTION* p = &bqfunction;
	int16_t i;

// Dummy cell reading data for CAN output
for (i = 0; i < NCELLMAX; i++)
	p->cellv_latest[i] = 35001+i;	

	bq_init();

	chgr_items_init(p);

	for (;;)
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET); // GRN LED ON
//	TIM1->CCR1 = p->tim1_ccr1; // Set charge rate
		osDelay(20);
	TIM1->CCR1 = 0; // Set charge rate
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET); // GRN LED OFF
		osDelay(2000-20);
	}
}
/* *************************************************************************
 * TaskHandle_t xBQTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BQTaskHandle
 * *************************************************************************/
TaskHandle_t xBQTaskCreate(uint32_t taskpriority)
{
	BaseType_t ret = xTaskCreate(StartBQTask, "BQTask",\
     (128), NULL, taskpriority,\
     &BQTaskHandle);
	if (ret != pdPASS) return NULL;

	return BQTaskHandle;

}
/* *************************************************************************
 * static uint8_t getcellv(void);
 * @brief	: Read 16 cell voltages
 * *************************************************************************/
static uint8_t getcellv(void)
{

	return 0;
}



/* *************************************************************************
 * static void morse_sos(void);
 * @brief	: 
 * *************************************************************************/
static void morse_sos(void)
{
	morse_string("SOS",GPIO_PIN_1);
	osDelay(1000);

	return;
}

/* *************************************************************************
 * static void charger_update(struct BQFUNCTION* p);
 * @brief	: Update charger/discharging FETs based on status byte
 * *************************************************************************/
static void charger_update(struct BQFUNCTION* p)
{
	/* Internal charger control. */
	if ((p->fet_status & FET_CHGR) != 0)
	{ // Here, set charging 
		TIM1->CCR1 = bqfunction.tim1_ccr1; // Set charge rate
	}
	else
	{ // If not normal rate, should it be Very Low Charge rate?
		if ((p->fet_status & FET_CHGR_VLC) != 0)
		{ // Here, yes.
			TIM1->CCR1 = bqfunction.lc.tim1_ccr1_on_vlc; // Set vlc rate
		}
		else
		{ // Here, stop charging
			
		}
	}
//		
//p->fet_status &= ~FET_CHGR;

	/* DUMP controls module discharging FET. */
	if ((p->fet_status & FET_DUMP) != 0)
	{ // Here, turn on FET for resistor discharge
		fetonoff(FET_DUMP,  FET_SETON);
	}
	else
	{ // Here, turn off FET for resistor discharge
		fetonoff(FET_DUMP,  FET_SETOFF);
	}

	/* DUMP2 controls external charger. */
	if ((p->fet_status & FET_DUMP2) != 0)
	{ // Here, turn on external module (or string?) charger
		fetonoff(FET_DUMP2,  FET_SETON);
	}
	else
	{ // Here, turn it off.
		fetonoff(FET_DUMP2,  FET_SETOFF);
	}
	return;
}
