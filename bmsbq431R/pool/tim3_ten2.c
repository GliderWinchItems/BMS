/******************************************************************************
* File Name          : tim3_ten2.c
* Date First Issued  : 04/04/2015, 06/17/2016
* Board              : f103
* Description        : Timer for polling functions in tension.c w AD7799 zero calib
*******************************************************************************/
/*
This timer routine will trigger the same low level interrupt as the CAN receive
routine for not-top-priority CAN msgs ("void USB_LP_CAN_RX0_IRQHandler(void)", 
located in canwinch_pod_common_systick2048.c).

This trigger is necessary for the situation where there are no incoming CAN msgs

Two output capture channels are used.
CH1 - regular spaced interrupts for timing 
CH2 - variable spaced interrupts for sequencing AD7799 operations
*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "interrupt_priority.h"
#include "tim3_ten2.h"
#include "common_can.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 32000000

/* Calls upon interrupt of timer. */
void 	(*tim3_ten2_ptr)(void) = NULL;	// Address of function to call upon CH1 interrupt
void 	(*tim3_ten2_ll_ptr)(void) = NULL;	// Low level function call every 'n' CH1's
#include "ad7799_filter_ten2.h"	// Rather than use a pointer for CH2 interrupt

/* Reduce the rate for low level interrupt triggering. */
#define TIM3LLTHROTTLE	4 	// Trigger count
unsigned int throttlect = 0;

/* Running count of timer ticks. */					
unsigned int tim3_ten2_ticks;	// Running count of timer ticks

/* Ch2 interrupt calls a routine that returns the increment for the next interval. */
static uint16_t inc; 	// Timer tick increment for regular interrupts

/******************************************************************************
 * void tim3_ten2_init(uint16_t t);
 * @brief	: Initialize TIM3 that produces interrupts used for timing measurements
 * @param	: t = number of timer ticks between regular interrupts
*******************************************************************************/
void tim3_ten2_init(uint16_t t)
{
	/* ----------- TIM3 ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM3 */
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Save time tick increment. */
	inc = t;

	/* Auto reload. */
	TIM3_ARR =  0xffff;// Default count

	/* Prescale divider (freq/(PSC+1)) */
	TIM3_PSC = 0;	// Divide by one

	/* CH1 and CH2 output capture. */
	// Use defaults for TIM3_CCMR1

	/* Set and enable interrupt controller for TIM3 interrupt */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM1
	
	/* Set a reasonable first interrupt time from now. */
	TIM3_CCR1 = TIM3_CNT + inc;	// Set next CH1 compare interrupt time
	TIM3_CCR2 = TIM3_CNT + inc/2;	// Set next CH2 compare interrupt time

	/* Interrupt enable: CH1 | CH2 */
	TIM3_SR	&=  ~((0xf<<9) | (1<<6) | 0x1f);// Clear any flags that may have come on
	TIM3_DIER = (1<<1) | (1<<2);		// CC1 and CC2 interrupt flags enable

	/* Control register 1 */
	TIM3_CR1 |= TIM_CR1_CEN; 		// Counter enable: counter begins counting

	return;
}
/*#######################################################################################
 * ISR routine for TIM3
 *####################################################################################### */
void TIM3_IRQHandler(void)
{
	__attribute__((__unused__))int temp;
	uint16_t sr = TIM3_SR;
	uint16_t vinc;

	if ((sr & (1<<1)) != 0 )	// CH1 flag
	{ // Here CH1 flag is on.  Regular interval timing
		TIM3_CCR1 += inc;	// Compute next interrupt time
		TIM3_SR &= ~(1<<1);	// Reset CH1 flag

		/* Running count of timer ticks. */					
		tim3_ten2_ticks += 1;

		/* Call other routines if an address is set up */
		if (tim3_ten2_ptr != 0)	// Skip if ptr not set
			(*tim3_ten2_ptr)();	// Go do something

		/* Trigger a low priority interrupt to poll for CAN msgs. */
		throttlect += 1;
		if (throttlect >= TIM3LLTHROTTLE) // Time to trigger a low level interrupt?
		{ // Yes.
			throttlect = 0;	// Reset
			NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')
		}
	}
	if ((sr & (1<<2)) != 0)
	{ // Here, CH2 flag is on.  Variable interval timing
		vinc = ad7799_poll_rdy_ten2_both();
		TIM3_CCR2 += vinc; 	// Compute next next interrupt time
		TIM3_SR &= ~(1<<2);	// Reset CH2 flag
	}
	temp = TIM3_SR;	// Avoid any tail-chaining

	return;

}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
void I2C1_ER_IRQHandler_ten(void)
{
	/* Call other routines if an address is set up */
	if (tim3_ten2_ll_ptr != 0)	// Having no address for the following is bad.
		(*tim3_ten2_ll_ptr)();	// Go do something
	return;
}
