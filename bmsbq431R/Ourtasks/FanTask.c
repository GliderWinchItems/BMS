/******************************************************************************
* File Name          : FanTask.c
* Date First Issued  : 10/02/2021
* Description        : Led blinking Task
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"
#include "FanTask.h"
#include "DTW_counter.h"


extern TIM_HandleTypeDef htim2;
/* Queue */
//#define QUEUESIZE 16


TaskHandle_t FanTaskHandle  = NULL;
osMessageQId FanTaskQHandle = NULL;

uint8_t fanspeed; // Fan speed: rpm pct 0 - 100

float fanrpm;


static TIM_TypeDef  *pT2base; // Register base address 

 uint32_t T2C3ctr;
static uint64_t T2C3cum;

	uint32_t itmpcum = 0;
	uint32_t itmpcum_prev = 0;
	uint32_t itmpctr = 0;
	uint32_t itmpctr_prev = 0;
	uint32_t itmp;
	uint32_t deltaN;
	float fdeltaT; 

/* *************************************************************************
 * void StartFanTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartFanTask(void* argument)
{
//while(1==1) osDelay(100);
	
	#define K (60 * 16E6 / 2) // Conversion to RPM of pulse_counts / timer_duration


	
	pT2base  = htim2.Instance;
	/* TIM2 Fan tach input capture & PWM output. */
   pT2base->ARR   = (640 - 1); // Count to give 25 KHz with 16MHz clock
   pT2base->EGR  |= 0x0A; // Input capture active: CH3
   pT2base->DIER  = 0x08; // CH3 interrupt enable
   pT2base->CCER |= 0x0101; // CH3 IC & CH1 PWM 

   /* Start counter */
   pT2base->CR1 |= 1;


	for (;;)
	{
		osDelay(250); // Speed computation update rate

		/* Get latest tachometer counts and time duration. */
		do 
		{ // Loop if interrupt updated either of the following 
			itmpcum = T2C3cum; // Get local copy of set by interrupt
			itmpctr = T2C3ctr; // Get local copy of set by interrupt
		} while ((T2C3cum != itmpcum) || (T2C3ctr != itmpctr));

		// Number of tachometer ticks since last loop pass
		deltaN = (unsigned int)(itmpctr - itmpctr_prev);
		if (deltaN == 0)	
		{ // No new tach ticks since last loop pass
			fdeltaT = (unsigned int)(DTWTIME - itmpcum_prev);
			deltaN = 1.0;
		}
		else
		{ // One or more tach ticks since last pass thru
			fdeltaT = (unsigned int)(itmpcum - itmpcum_prev);
			itmpcum_prev = itmpcum;
			itmpctr_prev = itmpctr;
		}
		/* RPM. */
		fanrpm = K * (float)deltaN / fdeltaT;

		/* Update FAN pwm. */
fanspeed = 0; // 14 = minimum; 100+ = full speed
;
		pT2base->CCR1 = (640 * fanspeed)/100;

	}
}
/* *************************************************************************
 * void FanTask_TIM2_IRQHandler(void);
 * @brief	: TIM2 IRQ (see stm32l4xx_it.c)
 * *************************************************************************/
void FanTask_TIM2_IRQHandler(void)
{
	if ((pT2base->SR & 0x08) != 0)
	{ // Here, input capture of tach pulse edge
		T2C3ctr += 1;
		T2C3cum  = DTWTIME;
	}
	pT2base->SR = 0x09; // Reset interrupt requests
	return;
}

/* *************************************************************************
 * TaskHandle_t xFanTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: FanTaskHandle
 * *************************************************************************/
TaskHandle_t xFanTaskCreate(uint32_t taskpriority)
{

/*
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
const char * const pcName,
unsigned short usStackDepth,
void *pvParameters,
UBaseType_t uxPriority,
TaskHandle_t *pxCreatedTask );
*/
	BaseType_t ret = xTaskCreate(StartFanTask, "FanTask",\
     (128), NULL, taskpriority,\
     &FanTaskHandle);
	if (ret != pdPASS) return NULL;

//	FanTaskQHandle = xQueueCreate(QUEUESIZE, sizeof(struct SERIALSENDTASKBCB*) );
//	if (FanTaskQHandle == NULL) return NULL;

	return FanTaskHandle;
}
