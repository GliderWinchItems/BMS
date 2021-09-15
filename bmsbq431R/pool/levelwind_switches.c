/******************************************************************************
* File Name          : levelwind_switches.c
* Date First Issued  : 09/16/2020
* Description        : Levelwind function w STM32CubeMX w FreeRTOS
*******************************************************************************/

/*
Notes:

PE9 - Input:pullup. Test sw bridges across overrun switches.

Limit switches: resistor pullup to +5v. Contact closes to gnd
   Interrupt vector: EXTI15_10_IRQHandler (common to PE10-PE15)
PE10 - EXTI10 MotorSideNot  Limit switch: NO contacts (switch connects to gnd) 
PE11 - EXTI11 MotorSideNot  Limit switch: NC contacts (switch connects to gnd)
PE12 - EXTI12 MotorSide Limit switch: NO contacts (switch connects to gnd)
PE13 - EXTI13 MotorSide Limit switch: NC contacts (switch connects to gnd)

Non-interrupting GPIO inputs
PE14 - MotorSideNot Overrun switches: wire-Ored NO contacts (switch connects to gnd)
PE7  - MotorSideNot Manual direction switch (switch connects to gnd)
PE8  - MotorSide Manual direction switch (switch connects to gnd)
PE9  - Manual enable switch (switch connects to gnd)

Switch closed, the i/o pin shows--
MotorSideNot limit switch: 
  PE10 = 0
  PE11 = 1
MotorSide limit switch: 
  PE12 = 0
  PE13 = 1

Non-interrupting switches

Overrun switches (wired-ORed):
  PE14 = 1


The following is held in abeyance for input capture timing of switches
PC6 - PE10
PC7 - PE11
PC8 - PE12
PC9 - PE13

*/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "stm32f4xx_hal.h"
#include "morse.h"
#include "main.h"
#include "levelwind_items.h"
#include "DTW_counter.h"
#include "drum_items.h"
#include "levelwind_switches.h"
#include "LevelwindTask.h"

static TIM_TypeDef  *pT2base; // Register base address 
static TIM_TypeDef  *pT5base; // Register base address 

/* Circular buffer for processing switch transitions. */
#define SWITCHXITIONSIZE 16
static struct SWITCHXITION switchxtion[SWITCHXITIONSIZE];
static struct SWITCHXITION* pbegin;
static struct SWITCHXITION* padd;
static struct SWITCHXITION* ptake;
static struct SWITCHXITION* pend;

static uint32_t integrity;
static uint32_t alert;


/* *************************************************************************
 * void levelwind_switches_init(void);
 * @brief       : Initialization
 * *************************************************************************/
void levelwind_switches_init(void)
{
	struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
   pT2base  = htim2.Instance;
   pT5base  = htim5.Instance;

   /*	MX sets up switches for falling edge and enables interrupts
   	We were unable to keep it from immediately enableing interrupts
   	so disable all limit swithc interupts to prevent a possible
   	race condtion 
   EXTI->IMR &= ~(LimitSw_MSN_NO_Pin | LimitSw_MSN_NC_Pin
   	| LimitSw_MS_NO_Pin | LimitSw_MS_NC_Pin); 


   // Clear any pending (may not be needed)
   EXTI->PR   |=  (LimitSw_MSN_NO_Pin | LimitSw_MSN_NC_Pin
   	| LimitSw_MS_NO_Pin | LimitSw_MS_NC_Pin); 
   	*/ 

   /* Circular buffer pointers. */
   pbegin = &switchxtion[0];
   padd   = &switchxtion[0];
   ptake  = &switchxtion[0];
   pend   = &switchxtion[SWITCHXITIONSIZE];

	p->swbits = GPIOE->IDR & 0x7f80; // Save all current switch bits PE7:14

	/* Initialize the debounced limit switch state & flags. */
	if ((p->swbits & LimitSw_MSN_NO_Pin) == 0)
	{ // Here NO contact is now closed.
		p->sw[LIMITDBMSN].dbs = 1; // Set debounced R-S
		p->sw[LIMITDBMSN].flag1  = 1; // Flag for stepper ISR
		EXTI->IMR |= LimitSw_MSN_NC_Pin;	//	enable MSN_NC interrupts
		HAL_GPIO_WritePin(GPIOD,LED_ORANGE_Pin,GPIO_PIN_SET);	
	}
	else	EXTI->IMR |= LimitSw_MSN_NO_Pin;	//	enable MSN_NO interrupts

	if ((p->swbits & LimitSw_MS_NO_Pin) == 0)
	{ // Here NO contact is now closed.
		p->sw[LIMITDBMS].dbs = 1; // Set debounced R-S
		p->sw[LIMITDBMS].flag1  = 1; // Flag for stepper ISR
		EXTI->IMR |= LimitSw_MS_NC_Pin;	//	enable MS_NC interrupts 
		HAL_GPIO_WritePin(GPIOD,LED_RED_Pin,GPIO_PIN_SET);
	}
	else EXTI->IMR |= LimitSw_MS_NO_Pin;	//	enable MS_NO interrupts


	return;
}

/* *************************************************************************
 * struct SWITCHXITION* levelwind_switches_get(void);
 * @brief       : Get pointer to circular buffer if reading available
 * @return      : pointer to buffer entry; NULL = no reading
 * *************************************************************************/
struct SWITCHXITION* levelwind_switches_get(void)
{
	struct SWITCHXITION* ptmp;
	if (ptake == padd) return NULL;
	ptmp = ptake;
	ptake += 1;
	if (ptake >= pend) ptake = pbegin;
	return ptmp;
}

/*#######################################################################################
 * ISR routine for EXTI
 * CH1 = OC stepper reversal
 * CH2 = OC faux encoder interrupts
 *####################################################################################### */
/*
LimitSw_MSN_NO_Pin	GPIO_PIN_10
LimitSw_MSN_NC_Pin	GPIO_PIN_11
LimitSw_MS_NO_Pin   	GPIO_PIN_12
LimitSw_MS_NC_Pin   	GPIO_PIN_13
*/
uint32_t dbsws1[5] = {0}; // Debug

void Stepper_EXTI15_10_IRQHandler(void)
{
	struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
	struct SWITCHXITION* ptmp;
//HAL_GPIO_TogglePin(GPIOD,LED_ORANGE_Pin);
dbsws1[0]++;
/*	Name is from time when switches beyond 10:13 were interrupt driven	*/

	/* Here, one or more PE10-PE13 inputs changed. */
	p->swbits = GPIOE->IDR & 0x7f80; // Save latest switch bits 09:15

#if LEVELWINDDEBUG 
	padd->sws = p->swbits;		// Save all switch contact bits
	padd->tim = pT2base->CNT;  // 32b timer time
	padd->cnt = pT5base->CNT;  // Encoder counter
   ptmp = padd;  					// Save in case R-S change
	padd++;    						// Advance in circular buffer
	if (padd >= pend) padd = pbegin; // Wrap-around
#endif

	/* Do R-S flip-flop type switch debouncing for limit switches. */
	if ((EXTI->PR & (LimitSw_MSN_NO_Pin)) != 0)
	{ // Here NSN_NO switch closed while R-S flip-flop was reset
		EXTI->PR = LimitSw_MSN_NO_Pin; // Reset request
		dbsws1[1]++;

		p->sw[LIMITDBMSN].dbs = 1; // Set debounced R-S
		p->sw[LIMITDBMSN].posaccum_NO = p->posaccum.s32;
		p->sw[LIMITDBMSN].flag1  = 1; // Flag for stepper ISR
		p->sw[LIMITDBMSN].flag2 += 1; // Flag for task(?)
		EXTI->IMR &= ~LimitSw_MSN_NO_Pin;	//	disable MSN_NO interrupts
		EXTI->IMR |= LimitSw_MSN_NC_Pin;		//	enable MSN_NC interrupts

		ptmp->sws |= LIMITDBMSN;
		HAL_GPIO_WritePin(GPIOD,LED_ORANGE_Pin,GPIO_PIN_SET);				
		return;
	}
	if ((EXTI->PR & (LimitSw_MSN_NC_Pin)) != 0)
	{ // Here MSN_NO switch closed while R-S flip-flop was set
		EXTI->PR = LimitSw_MSN_NC_Pin; // Reset request
		dbsws1[2]++;

		p->sw[LIMITDBMSN].dbs = 0; // Reset debounced R-S
		p->sw[LIMITDBMSN].posaccum_NC = p->posaccum.s32;
		p->sw[LIMITDBMSN].flag1  = 1; // Flag for stepper ISR
		p->sw[LIMITDBMSN].flag2 += 1; // Flag for task(?)
		EXTI->IMR &= ~LimitSw_MSN_NC_Pin;	//	disable MSN_NC interrupts
		EXTI->IMR |= LimitSw_MSN_NO_Pin;		//	enable MSN_NO interrupts
		
		ptmp->sws |= LIMITDBMSN;
		HAL_GPIO_WritePin(GPIOD,LED_ORANGE_Pin,GPIO_PIN_RESET);				
		return;
	}

	if ((EXTI->PR & (LimitSw_MS_NO_Pin)) != 0)
	{ // Here Here MS_NO switch closed while R-S flip-flop was reset
		EXTI->PR = LimitSw_MS_NO_Pin; // Reset request
		dbsws1[3]++;

		p->sw[LIMITDBMS].dbs = 1; // Set debounced R-S
		p->sw[LIMITDBMS].posaccum_NO = p->posaccum.s32;
		p->sw[LIMITDBMS].flag1  = 0; // Flag for stepper ISR  
		p->sw[LIMITDBMS].flag2 += 1; // Flag for task(?)
		EXTI->IMR &= ~LimitSw_MS_NO_Pin;	// disable MS_NO interrupts
		EXTI->IMR |= LimitSw_MS_NC_Pin;	//	enable MS_NC interrupts	  	  
		
		ptmp->sws |= LIMITDBMS;
		HAL_GPIO_WritePin(GPIOD,LED_RED_Pin,GPIO_PIN_SET);			
		return;
	}
	if ((EXTI->PR & (LimitSw_MS_NC_Pin)) != 0)
	{ // Here MS_NC switch closed while R-S flip-flop was set
		EXTI->PR = LimitSw_MS_NC_Pin; // Reset request
		dbsws1[4]++;

		p->sw[LIMITDBMS].dbs = 0; // Reset debounced R-S
		p->sw[LIMITDBMS].posaccum_NC = p->posaccum.s32;
		p->sw[LIMITDBMS].flag1  = 1; // Flag for stepper ISR
		p->sw[LIMITDBMS].flag2 += 1; // Flag for task(?)
		EXTI->IMR &= ~LimitSw_MS_NC_Pin;	//	disable MS_NC interrupts
		EXTI->IMR |= LimitSw_MS_NO_Pin;	//	enable MS_NO interrupts
		
		ptmp->sws |= LIMITDBMS;
			HAL_GPIO_WritePin(GPIOD,LED_RED_Pin,GPIO_PIN_RESET);							
		return;
	}
	return;
}

/* *************************************************************************
 * void levelwind_switches_error_check(void);
 * @brief       : 
 * *************************************************************************/
void levelwind_switches_error_check(void)
{
	struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
	//	Revist: When and how is this function used?

/* === Illegal combinations */
	/* 1: Open cable, or missing ground. */
	if ((p->swbits & 0xfe00) == 0xfe00)
		integrity |= STEPPERSWSTS01; // Error
	else
		integrity &= ~STEPPERSWSTS01;

	/* 2: MotorSideNot-MotorSide limit sw NO both closed. */
	if ((p->swbits & (LIMITMSNNO | LIMITMSNO)) == 0)
		integrity |= STEPPERSWSTS02; // Error
	else
		integrity &= ~STEPPERSWSTS02;

	/* 3: MotorSideNot NC and NO are both closed. */
	if ((p->swbits & (LIMITMSNNO | LIMITMSNNC)) == 0)
		integrity |= STEPPERSWSTS03; // Error
	else
		integrity &= ~STEPPERSWSTS03;

	/* 4: MotorSideNot NC and NO are both closed. */
	if ((p->swbits & (LIMITMSNO | LIMITMSNC)) == 0)
		integrity |= STEPPERSWSTS04; // Error
	else
		integrity &= ~STEPPERSWSTS04;

	/*	Reconsider overrun switch integrity tests below	*/
	/* 5: Both overrun NC are open. Removed when switches were
			wire-Ored 	*/

	/* 6: MotorSide overrun closed, MotorSide limit sw open.
			Removed when overrun switches were wire-ORed. */
	

	/* 7: MotorSideNot overrun closed, MotorSide limit sw open 
			Removed when overrun switches were wire-ORed. */
	


/* ===== Alerts ===== */
	/* 0: Bridge switch. */
	if ((p->swbits & OVERRUNBRIDGE) == 0)
		alert &= ~STEPPERSWALRT00; // Sw in operational position
	else
		alert |= STEPPERSWALRT00; // Sw in MANUAL (bridged power) position	

	/* 1: MotorSideNot overun switch is closed. */
	if ((p->swbits & OVERRUNSWES) != 0)
		alert |= STEPPERSWALRT01; // 
	else
		alert &= ~STEPPERSWALRT01; // 	

	return;
}