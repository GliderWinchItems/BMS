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

/* Select which fanspeed algoritm to use. 
comment "out" for original version (simple proportional).
comment "in"  for latest. (state machine) */
//#define NEWFANSPEED

extern TIM_HandleTypeDef htim2;

/* Count 'ticks' for timing, synchronous to 'main'. */
#define TICKUPDATE 4 // 'main' calls 4 times/sec
#define FANFORCETIMEOUT (8*4) // 4 sec Delay before forced fan setting times out

/* Sets fan speed. */
static uint8_t compute_fanspeed(void);

float fanrpm;

TIM_TypeDef  *pT2base; // Register base address 

 uint32_t T2C3ctr;
static uint32_t T2C3cum;

static uint32_t tickctr_running; // Count entries from 'main'
static uint32_t tickctr_next1;   // Fan speed update
static uint32_t tickctr_next3;   // Time forced pwm setting

/* Commands set by CAN msgs in cancomm_items.c. */
uint32_t cmd_pwm_ctr;      // Running ctr: pwm forced flag
static uint32_t cmd_pwm_ctr_prev; // Previous ct
uint8_t  cmd_pwm_set;      // pwm setting being forced
static uint8_t  cmd_pwm_flag;     // 1 = forced pwm being time

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
#ifdef NEWFANSPEED
	/* Check that parameters were initialized. */
	struct BQLC* p = &bqfunction.lc; // Convenience pointer
	if ( (p->temp_fan_min  == 0) ||
         (p->temp_fan_max  == 0) ||
         (p->temp_fan_del  == 0) ||
         (p->temp_fan_cell == 0) || 
         (p->temp_fan_min_pwm == 0) )
         morse_trap(823);
#endif

	tickctr_next1 = tickctr_running + TICKUPDATE;      

   bqfunction.temp_fan_state = 0;

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

	tickctr_running += 1;
	if ((int)(tickctr_next1 - tickctr_running) > 0)
	{
		tickctr_next1 += TICKUPDATE;

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

		/* Did a CAN msg set a forced fan setting? (see cancomm_items.c) */
		if (cmd_pwm_ctr != cmd_pwm_ctr_prev)
		{ // Here, CAN msg requests a forced fan setting
			cmd_pwm_ctr_prev = cmd_pwm_ctr; // Update request check
			tickctr_next3 = tickctr_running + FANFORCETIMEOUT; // Update timeout
			cmd_pwm_flag = 1; // Show running with forced setting
		}
		if (cmd_pwm_flag != 0)
		{ // Here, forced setting being used.
			if ((int)(tickctr_next3 - tickctr_running) < 0)
			{ // Here, timed out, so quit forced mode.
				cmd_pwm_flag = 0;
			}
			// Set fan speed according to request
			if (cmd_pwm_set > 100) cmd_pwm_set = 100; // jic
			bqfunction.fanspeed = cmd_pwm_set;
		}
		else
		{ // Here use algorithm to set fan speed
			bqfunction.fanspeed = compute_fanspeed();
//bqfunction.fanspeed = 16;		
		}

// Debug
//bqfunction.fanspeed = 90; // 14 = minimum; 100+ = full speed		

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
#ifndef NEWFANSPEED

/* Simple version. */
static uint8_t compute_fanspeed(void)
{
	uint8_t tmp = 0;
	float tmpf;
	struct BQLC* p = &bqfunction.lc; // Convenience pointer
//	float tcell = p->thermcal[p->tcell_idx].temp;
	float tcell = p->thermcal[1].temp;

	if (tcell > p->temp_fan_min)
	{
		if (tcell > p->temp_fan_max)
		{
			tmp = 100;
		}
		else
		{ // Min speed pwm is 14, and max is 100
			tmpf = ((tcell - p->temp_fan_min) /
		           (p->temp_fan_max - p->temp_fan_min)     );
			tmp = (tmpf * 100.0f);
			if (tmp <= 14) tmp = 0;
		}	
	}
	return tmp;
}
#else

/* State machine verison. */
static uint32_t tickctr_next2;   // Time Tamb delay

static uint8_t compute_fanspeed(void)
{
	struct BQLC* p = &bqfunction.lc; // Convenience pointer

	/* Convenience vars. */
	float tcell = p->thermcal[p->tcell_idx].temp;
	float tamb  = p->thermcal[p->tamb_idx].temp;

	/* Running tick counter (4/sec) */
	tickctr_running += 1;
/*
   float  temp_fan_min_pwm;  // Fan pwm must be greater than this to run
   float  temp_fan_limit_hi; // Above this is no-launch range
   float  temp_fan_thres_hi; // Above this is caution zone
   float  temp_fan_thres_lo; // Below this is caution zone
   float  temp_fan_limit_lo; // Below this is no-launch range
   float  temp_fan_delay1;   // Time delay (secs) to assure reliable ambient reading
   float  temp_fan_delta;    // Null zone beween Tamb and Tcell*/	

	if (tcell > p->temp_fan_limit_hi)
	{ // Too hot
		return 100; // Fan at 100%
	}

	if (tcell > p->temp_fan_thres_lo)
	{ // Cooling 

		switch(bqfunction.temp_fan_state)
		{
		case 0: // Reset/initial
			if (tcell < p->temp_fan_thres_hi)
				return 0;
			tickctr_next2 = tickctr_running + p->temp_fan_delay1;
			bqfunction.temp_fan_state = 1;
			return 100;

		case 1: // Time delay in progress
			if ((int)(tickctr_next2 - tickctr_running) > 0)
				return 100;
			bqfunction.temp_fan_state = 2;
				return 100;

		case 2:
			if (tcell < p->temp_fan_thres_lo)
				bqfunction.temp_fan_state = 0;
				return 0; // STOP!. Almost too cool

			if ((tcell - tamb) > p->temp_fan_delta)
				return 100; 

			// Here, (tcell-tamb) difference inconsequential
			if (tcell > p->temp_fan_thres_hi)
				return 50; // Maybe ambient is bogus. Don't quit

			// Here, tcell-tamb difference inconsequential 
			// AND less than thres_hi
			// AND greater than thres_lo
			bqfunction.temp_fan_state = 0;
				return 0;
		}
	}
	else
	{ // Heating
		bqfunction.temp_fan_state = 0;

	}
	return 0;
}
#endif