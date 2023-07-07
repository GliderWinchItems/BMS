/******************************************************************************
* File Name          : FanTask.c
* Date First Issued  : 10/02/2021
* Description        : Led blinking Task
*******************************************************************************/
/* gsm email:  07/04/2023
Some thinking about temperature related battery module status and operation. This assumes pack heating does not use the dump or heating FETs under autonomous BMS control. (The pack could be commanded to turn on dump and heater by EMC.) All the numbers are working and placeholders. I would expect parameters in the per module struct for those. (I made some numbers specifically different to suggest separate parameters.) No urgency in implementing any of this but just to capture my thoughts on this and get yours.

Module Status related to temperature:
If pack temperature is greater than 50C, red status for too hot for operation.
If pack temperature is between 40C and 50C, yellow status for warm pack
if pack temperature is between 5C and 10C, yellow status for cold pack.
If pack temperature is below 5C, red status for too cold for operation
If exit air temperature is greater that 10C above ambient temperature, red status for possible fire.
If fan cannot be started when commanded, yellow status for fan inoperative.

Winch operations can continue and be initiated so long as pack temperature does not take status to red. (Expect status value (1 byte) to be 0 for normal, <0 for red, and > 0 for yellow. Probably conditions bit mapped beyond that.)

Fan Behavior: 

For pack temperatures between 8C and 20C, no fan.

When fan is to be started, I expect initial 100% PWM until fan motion is detected falling to values specified below once operation is detected. (Essentially what you seem to be doing when the BMS does boot up status checks.)

EMC may command fan to operate at a specified PWM for whatever purpose it desires. This disables the autonomous behaviors specified below until a command reverting the module to autonomous fan control. This command could be module specific or all modules in a string. Examples of this would be in anticipation of an imminent launch when the module pack is getting warm and ambient is below the pack temperature. Or the EMC has commanded heating when modules are in a doghouse enclosure and it wants to circulate the air for warming.

Autonomous Cooling: Pack above 20C

Fan starts at 10% PWM when pack temperature is 3C above ambient temperature. Fan PWM increases linearly to 100% as temperature differential increases to 10C and 100% for above.

Autonomous Heating: Pack is below 8C

Fan turns on at 12% when ambient temperature is 3C greater than pack temperature. Fan speed increases linearly to 100% as temperature differential increases to 8C and 100% for above.
*/
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

TIM_TypeDef  *pT2base; // Register base address 

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
fanspeed = 18; // 14 = minimum; 100+ = full speed
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
	pT2base->SR = 0x01; // Reset interrupt flag: UI
	if ((pT2base->SR & 0x08) != 0)
	{ // Here, input capture of tach pulse edge
		T2C3ctr += 1;
		T2C3cum  = DTWTIME;
		pT2base->SR = 0x08; // Reset interrupt flag: CC3IF
	}	
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
