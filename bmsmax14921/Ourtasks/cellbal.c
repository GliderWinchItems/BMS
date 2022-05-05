/******************************************************************************
* File Name          : cellbal.c
* Date First Issued  : 03/20/2022
* Description        : MAX14921: cell balancing
*******************************************************************************/

#include "cellbal.h"
#include "morse.h"
#include "BQTask.h"

/* 
 DUMP2 fet--External charger control
 PC6 0  OFF; 1 ON

 Dump fet control
PC8  PC10 DUMP
 0    0   OFF
 0    1   OFF
 1    0   ON
 1    1   OFF

  Heater fet control
 PC11 PC12 HEATER
 0    0    OFF
 0    1    OFF
 1    0    ON
 1    1    OFF
*/

#define CALLBALBIT00 (1<<0) // Wait notification 
struct ADCREADREQ adcreadreq;
float fcell[ADCBMSMAX]; // (16+3+1) = 20; Number of MAX14921 (cells+thermistors+tos)   	

/* *************************************************************************
 * void cellbal_init(void);
 * @brief	: Go thought a sequence of steps to determine balancing
 * @brief   : parq = pointer to INITIALIZED bms read request struct
 * *************************************************************************/
	void cellbal_init(void)
	{
	/* Pre-load BMS readout request queue block. */	
//	adcreadreq.taskhandle = xTaskGetCurrentTaskHandle();// Requesting task's handle
//	adcreadreq.tasknote   = CALLBALBIT00;// ADCTask completed BMS read 
	adcreadreq.taskdata   = &fcell[0];   // Requesting task's pointer to buffer to receive data
	adcreadreq.cellbits   = 0;           // Bits to set FETs
	adcreadreq.updn       = 0;           // BMS readout direction high->low cell numbers
	adcreadreq.reqcode    = REQ_SETFETS; // Set discharge fets.
	adcreadreq.encycle    = 1;     // Cycle EN: 0 = after read; 1 = before read w osDelay
	adcreadreq.readbmsfets= 0;           // Clear discharge fets before readbms.
	adcreadreq.doneflag   = 0; // 1 = ADCTask completed BMS read 
	return;	
}
/* *************************************************************************
 * void cellbal_do(struct ADCREADREQ* parq);
 * @brief	: Check cell voltages and set 
 *          : -  I/O pins for DUMP resistor FET ON/OFF
 *          : -  I/O pin for DUMP2 FET (External charger) ON/OFF
 *          : -  Discharge FETs
 *          : -  Trickle charger OFF, low rate, hight rate setting
 * @brief   : parq = pointer to INITIALIZED bms read request struct
 * *************************************************************************/
void cellbal_do(struct ADCREADREQ* parq)
{
	BaseType_t qret;
	int i;
	uint32_t noteval;
	uint32_t flag_dump; // PC8 & PC10 for DUMP ON/OFF
	uint32_t flag_extchgr; // External charger fet on/off
	uint32_t doneflagctr;
	uint16_t fetbits; // Discharge FET bits
	uint16_t flag_trickle; // PWM count for trickle charge level
	uint8_t  ctr1; // Count of cells at or above maximum (target)
	uint8_t  ctr2; // Count of cells at or below minimum

	/* Disable things that affect cell readings. */
	// PC6  = 0: DUMP2 (external charger control on/off fet)
	// PC8  = 1: DUMP (resistor load) 
	// PC10 = 0: DUMP (resistor load)
	// PC11 = 1: HEATER
	// PC12 = 0: HEATER
	GPIOC->BSRR =  ( (1<<( 6+16)) | 
		             (1<<( 8+ 0)) |
	                 (1<<(10+16)) |
	                 (1<<(11+ 0)) |
		             (1<<(12+16)) );
	// Trickle charger off
	TIM1->CCR1 = 0;	// FET is off

	/* Queue a read BMS request to ADCTask.c */
	qret = xQueueSendToBack(ADCTaskReadReqQHandle, parq, 3500);
if (qret != pdPASS) morse_trap(722); // JIC debug

#define DONEFLAGCT 200
		doneflagctr = 0;
		while ((adcreadreq.doneflag == 0) && (doneflagctr++ < DONEFLAGCT)) 
			osDelay(1);
		if (doneflagctr >= DONEFLAGCT) morse_trap(733);

	/* Categorize for setting discharging and charging.
	   and set discharge FET bits. */
	ctr1 = 0; ctr2 = 0; fetbits = 0;
	for ( i = 0; i < 16; i++)
	{
		if (*(parq->taskdata + i) < bqfunction.lc.cellv_min)
		{
			ctr2 += 1; // Count cells below minimum
		}		
		else if (*(parq->taskdata + i) > bqfunction.lc.cellv_max)
		{
			ctr1 += 1; // Count cells above target (max)
			fetbits |= (1<<i); // Set discharge fet for this cell
		}
	}

	/* Are all cells above max (target)? */
	if (ctr1 == 16)
	{ // All cells are above target.
	// PC6  = 0: DUMP2 (external charger control on/off fet)
	// PC8  = 1: DUMP (resistor load) 
	// PC10 = 0: DUMP (resistor load)
		flag_dump = ((1<<( 8+16))|(1<<(10+0))); // DUMP ON
		fetbits     = 0xffff; // All discharge FETs ON
		flag_trickle = 0; // Trickle charger rate zero
		flag_extchgr = (1<<( 6+16)); // External charger OFF
	}

	/* Are any cells above max (target)? */
	if (ctr1 > 0)
	{ // Here, yes.
		flag_dump = ((1<<( 8+0))|(1<<(10+16))); // DUMP OFF
		flag_trickle = bqfunction.lc.tim1_ccr1_on; // Trickle charger rate normal
		flag_extchgr = (1<<( 6+16)); // External charger OFF
	}

	/* Are any cells below minimum? */
	if (ctr2 > 0)
	{ // Here, yes.
		flag_dump = ((1<<( 8+0))|(1<<(10+16))); // DUMP OFF
		flag_trickle = bqfunction.lc.tim1_ccr1_on_vlc; // Trickle charger rate normal
		flag_extchgr = (1<<( 6+16)); // External charger OFF
	}

	/* Active this stuff. */
	// Set DUMP on/off
	GPIOC->BSRR = flag_dump;
	// Set externl charger on/off
	GPIOC->BSRR = flag_extchgr;
	// Set fets
	TIM1->CCR1 = flag_trickle;

	/* Queue fet set request to ADCTask.c */
	cellbal_init();
	qret = xQueueSendToBack(ADCTaskReadReqQHandle, &adcreadreq, 3500);
if (qret != pdPASS) morse_trap(724); // JIC debug

	doneflagctr = 0;
	while ((adcreadreq.doneflag == 0) && (doneflagctr++ < DONEFLAGCT)) 
		osDelay(1);
	if (doneflagctr >= DONEFLAGCT) morse_trap(732);

	return;
}