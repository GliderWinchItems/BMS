/******************************************************************************
* File Name          : fanop.c
* Date First Issued  : 06/22/2022
* Description        : 12v Fan operation (call from 'main')
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"
#include "fanop.h"
#include "DTW_counter.h"
#include "bq_items.h"

extern TIM_HandleTypeDef htim2;

#define TICKUPDATE 5 // 250 ms between updates

/* Sets fan speed. */
static uint8_t compute_fanspeed(void);

float fanrpm;

TIM_TypeDef  *pT2base; // Register base address 

 uint32_t T2C3ctr;
static uint32_t T2C3cum;

static uint32_t tickctr;

uint32_t itmpcum = 0;
uint32_t itmpcum_prev = 0;
uint32_t itmpctr = 0;
uint32_t itmpctr_prev = 0;
uint32_t itmp;
uint32_t deltaN;
float fdeltaT; 

/* *************************************************************************
 * void fanop_init(void);
 *	@brief	: Init fan operation
 * *************************************************************************/
void fanop_init(void)
{
	pT2base  = htim2.Instance;
	/* TIM2 Fan tach input capture & PWM output. */
   pT2base->ARR   = (640 - 1); // Count to give 25 KHz with 16MHz clock
   pT2base->CCER |= 0x0001; // CH1 PWM 
   pT2base->CR1  |= 1; // Start counter

   	/* Enable limit and overrun switch interrupts EXTI15_10. */
   	EXTI->IMR1  |= (1 << 10); // Interrupt mask reg: enable line 10
//   	EXTI->EMR1  |= (1 << 10);
	EXTI->RTSR1 |= (1 << 10); // Rising edge

 	/* Px4 EXTI selection clear. */
//	SYSCFG->EXTICR[1] &= ~(0x7 << 0);	

	/* EXTI4 ('1818 SDO conversion complete) interrupt initialize*/
  	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ  (EXTI15_10_IRQn);  	   
   return;
}
/* *************************************************************************
 * float fanop(void);
 *	@brief	: Update 
 *  @return : 0 = nothing done; not 0 = rpm
 * *************************************************************************/
float fanop(void)
{
	#define K (60 * 16E6 / 2) // Conversion to RPM of pulse_counts / timer_duration

	tickctr += 1;
	if (tickctr >= TICKUPDATE)
	{
		tickctr = 0;

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
		bqfunction.fanspeed = compute_fanspeed();
// Debug: bqfunction.fanspeed = 14; // 14 = minimum; 100+ = full speed		

		/* Set TIM2CH1 PWM to control fan speed. */
		pT2base->CCR1 = (640 * bqfunction.fanspeed)/100;

		return fanrpm;
	}
	return 0;
}
/* *************************************************************************
 * void EXTI15_10_IRQHandler(void);
 * @brief	: Fan Tach--PB10 EXTI interrupt
 * *************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	EXTI->PR1 |=  (1<<10); // Reset request: PB10
	T2C3ctr += 1;
	T2C3cum  = DTWTIME;
	return;
}
/* *************************************************************************
 * static uint8_t compute_fanspeed(void);
 * @brief	: Compute a fanspeed pwm setting based on thermistor temperature readings
 * *************************************************************************/
static uint8_t compute_fanspeed(void)
{
	uint8_t max = 0;
	uint8_t tmp = 0;
	float tmpf;
	int i;
	/* Check each thermistor position. */
	for (i = 0; i < 3; i++)
	{ 
		if (bqfunction.lc.thermcal[0].installed == 1)
		{ // Here, thermistor position is installed
			if (bqfunction.lc.thermcal[i].temp > bqfunction.lc.temp_fan_min)
			{
				if (bqfunction.lc.thermcal[i].temp > bqfunction.lc.temp_fan_max)
				{
					tmp = 100;
				}
				else
				{ // Min speed pwm is 14, and max is 100 (hence 86.0 below)
					tmpf = ((bqfunction.lc.thermcal[i].temp - bqfunction.lc.temp_fan_min) /
				           (bqfunction.lc.temp_fan_max - bqfunction.lc.temp_fan_min)     );
					tmp = (tmpf * 86.0f) + 14;
				}
			}
			else
			{
				tmp = 0;
			}
		}
		if (tmp > max) max = tmp;
	}
	return tmp;
}