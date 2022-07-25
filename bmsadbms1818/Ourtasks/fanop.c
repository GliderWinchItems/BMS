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

#define TICKUPDATE 25 // 250 ms between updates

/* Sets fan speed. */
static uint8_t compute_fanspeed(void);

float fanrpm;

TIM_TypeDef  *pT2base; // Register base address 

 uint32_t T2C3ctr;
static uint64_t T2C3cum;

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
   pT2base->EGR  |= 0x0A; // Input capture active: CH3
   pT2base->DIER  = 0x08; // CH3 interrupt enable
   pT2base->CCER |= 0x0101; // CH3 IC & CH1 PWM 

   /* Start counter */
   pT2base->CR1 |= 1;
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
//bqfunction.fanspeed = 18; // 14 = minimum; 100+ = full speed
		bqfunction.fanspeed = compute_fanspeed();

		pT2base->CCR1 = (640 * bqfunction.fanspeed)/100;

		return fanrpm;
	}
	return 0;
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
 * static uint8_t compute_fanspeed(void);
 * @brief	: Compute a fanspeed setting based on thermistor temperature readings
 * *************************************************************************/
static uint8_t compute_fanspeed(void)
{
	uint8_t max = 0;
	uint8_t tmp = 0;
	float tmpf;
	int i;
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