/******************************************************************************
* File Name          : DTW_counter.c
* Date First Issued  : 10/21/2013
* Board              : STM32
* Description        : Use of the DTW_CYCCNT counter
*******************************************************************************/

//#include "common.h"

/******************************************************************************
 * void DTW_counter_init(void);
 * @brief 	: Setup the DTW counter so that it can be read
*******************************************************************************/
void DTW_counter_init(void)
{
/* Use DTW_CYCCNT counter for timing */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)
	return;
}

