/******************************************************************************
* File Name          : LedTask.c
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
#include "LedTask.h"

/* Queue */
#define QUEUESIZE 16	// Total size of qb's 

#define QUEUEWAIT 50     // 50 ms queue wait timeout
#define WINKONDUR    2
#define WINKOFFDUR  39
#define FASTWINKONDUR    1
#define FASTWINKOFFDUR  10
#define ALTERNATEONDUR   10
#define ALTERNATEOFFDUR  10  


TaskHandle_t LedTaskHandle  = NULL;
osMessageQId LedTaskQHandle = NULL;

enum ledstate
{
	IDLE,
	WINKON,
	WINKOFF,
};

struct LEDTIMING
{
	uint32_t dur_on;
	uint32_t dur_off;
	uint32_t next_ctr;
	uint16_t state;
};

static struct LEDTIMING grn;
static struct LEDTIMING red;

static uint32_t loopctr; // Queue receive wait loop timeout times LEDs

/* *************************************************************************
 * void StartLedTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartLedTask(void* argument)
{
//while(1==1) osDelay(100);
	struct LEDTASKQB  ltqb; // Copied item from queue
	BaseType_t ret;

	for (;;)
	{
		ret = xQueueReceive(LedTaskQHandle,&ltqb,QUEUEWAIT);
		if (ret == pdPASS)
		{ // Here, setup new blink scheme
			switch (ltqb.code)
			{
				case BOTH_OFF:
					HAL_GPIO_WritePin(GPIOB,LED_GRN_Pin,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOB,LED_RED_Pin,GPIO_PIN_SET);
					grn.next_ctr = 0; red.next_ctr = 0;
					grn.state = IDLE; red.state = IDLE;
					break;
				case GRN_OFF:
					HAL_GPIO_WritePin(GPIOB,LED_GRN_Pin,GPIO_PIN_SET);
					grn.next_ctr = 0; grn.state = IDLE;
					break;
				case RED_OFF:
					HAL_GPIO_WritePin(GPIOB,LED_RED_Pin,GPIO_PIN_SET);
					red.next_ctr = 0; red.state = IDLE;
					break;
				case GRN_WINK:
					grn.state = WINKON; grn.next_ctr = loopctr;
					break;
				case RED_WINK:
					red.state = WINKON; red.next_ctr = loopctr;
					break;
				case GRN_ON:
					HAL_GPIO_WritePin(GPIOB,LED_GRN_Pin,GPIO_PIN_RESET);
					grn.state = IDLE; grn.next_ctr = 0;
					break;
				case RED_ON:
					HAL_GPIO_WritePin(GPIOB,LED_RED_Pin,GPIO_PIN_RESET);
					red.state = IDLE; red.next_ctr = 0;
					break;
				case GRN_WINK_FAST:
					grn.state = WINKON; grn.next_ctr = loopctr;
					grn.dur_on = FASTWINKONDUR; grn.dur_off = FASTWINKOFFDUR;
					break;
				case RED_WINK_FAST:
					red.state = WINKON; red.next_ctr = loopctr;
					red.dur_on = FASTWINKONDUR; red.dur_off = FASTWINKOFFDUR;
					break;
				case BOTH_ALT:
					red.dur_on   = ALTERNATEONDUR;  grn.dur_on   = ALTERNATEONDUR;
					red.dur_off  = ALTERNATEOFFDUR; grn.dur_off  = ALTERNATEOFFDUR;
					red.next_ctr = loopctr;         grn.next_ctr = loopctr;
					red.state    = WINKON;	        grn.state    = WINKON;
					break;
				default:
					morse_trap(8221);
			}
		}
		else
		{ // Timeout: set LEDs ON or OFF
			loopctr += 1;
			/* GREEN control. */
			if ((int)(loopctr - grn.next_ctr) >= 0)
			switch(grn.state)
			{
				case IDLE: // Idle awaiting come command
					grn.next_ctr += 100000;
					break; 

				case WINKON:
					HAL_GPIO_WritePin(GPIOB,LED_GRN_Pin,GPIO_PIN_RESET);
					grn.next_ctr += grn.dur_on;
					grn.state = WINKOFF;
					
				case WINKOFF:
					HAL_GPIO_WritePin(GPIOB,LED_GRN_Pin,GPIO_PIN_SET);
					grn.next_ctr += grn.dur_off;
					grn.state = WINKON;
					break;
			}
			/* RED control */
			if ((int)(loopctr - red.next_ctr) >= 0)
			switch(red.state)
			{
				case IDLE: // Idle awaiting some command
					red.next_ctr += 100000;
					break;

				case WINKON:
					HAL_GPIO_WritePin(GPIOB,LED_RED_Pin,GPIO_PIN_RESET);
					red.next_ctr += red.dur_on;
					red.state = WINKOFF;
					
				case WINKOFF:
					HAL_GPIO_WritePin(GPIOB,LED_RED_Pin,GPIO_PIN_SET);
					red.next_ctr += red.dur_off;
					red.state = WINKON;
					break;
			}
		}
	}
}

/* *************************************************************************
 * TaskHandle_t xLedTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: LedTaskHandle
 * *************************************************************************/
TaskHandle_t xLedTaskCreate(uint32_t taskpriority)
{

/*
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
const char * const pcName,
unsigned short usStackDepth,
void *pvParameters,
UBaseType_t uxPriority,
TaskHandle_t *pxCreatedTask );
*/
	BaseType_t ret = xTaskCreate(StartLedTask, "LedTask",\
     (128), NULL, taskpriority,\
     &LedTaskHandle);
	if (ret != pdPASS) return NULL;

	LedTaskQHandle = xQueueCreate(QUEUESIZE, sizeof(struct SERIALSENDTASKBCB*) );
	if (LedTaskQHandle == NULL) return NULL;

	return LedTaskHandle;
}
