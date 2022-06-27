/******************************************************************************
* File Name          : cellbal.c
* Date First Issued  : 006/26/2022
* Description        : ADBMS1818: cell balancing
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
struct ADCREADREQ arrbal;
float fcell[ADCBMSMAX]; // (16+3+1) = 20; Number of MAX14921 (cells+thermistors+tos)   
float fcellsum;	


/* *************************************************************************
 * void cellbal_do(struct ADCREADREQ* parq);
 * @brief	: Check cell voltages and set 
 *          : -  I/O pins for DUMP resistor FET ON/OFF
 *          : -  I/O pin for DUMP2 FET (External charger) ON/OFF
 *          : -  Discharge FETs
 *          : -  Trickle charger OFF, low rate, hight rate setting
 * *************************************************************************/
void cellbal_do(void)
{
	struct BQFUNCTION* pbq = &bqfunction;
	struct BQLC* plc = &pbq->lc;
	BaseType_t qret;
	int i;
	int32_t idata;
	uint32_t flag_dump; // PC8 & PC10 for DUMP ON/OFF
	uint32_t flag_extchgr; // External charger fet on/off
	uint32_t doneflagctr;
	uint16_t flag_trickle; // PWM count for trickle charge level
	uint8_t  ctr1; // Count of cells at or above maximum (target)
	uint8_t  ctr2; // Count of cells at or below minimum


	/* Categorize for setting discharging and charging.
	   and set discharge FET bits. */
	ctr1               = 0; // Above maximum count
	ctr2               = 0; // Below minimum count
	fcellsum           = 0; // Installled cell voltage sum
	arrbal.cellbits    = 0; // Dischage FET bits
	parg->cellopenbits = 0; // Unexpected open wire bits

	/* Go through array for a maximum size box. */
	for ( i = 0; i < NCELLMAX; i++)
	{ // Skip predetermined empty box positions
		if (plc->cellpos[i] != CELLNONE)
		{
/* NOTE:  Should idata be a filter (hence float)? */
			idata = *(parg->taskdata + i);

			/* Sum installed cells for later check against top-of-stack. */
			fcellsum += idata;

			if (idata < plc->cellv_min)
			{ // Below usable voltage range
				if (idata < plc->cellopenlo)
				{ // Very low voltage is assumed to be an open wire
					parg->cellopenbits |= (1 << i);
				}
				else
				{ // Here, low, but not an unexpected open wire
					ctr2 += 1; // Count cells below minimum
				}
			}
			else if (idata < pbq->hysterv_lo)
			{ // Here, Cell is at low of hysteresis (relaxation)
				pbq->hyster_sw = 0; // Set hysteresis off
				pbq->trip_max  = 0; // All cells have to trip max again.
			}
			else if (idata > plc->cellv_max)
			{ // Here voltage is above max
				if (idata > plc->cellopenhi)
				{ // Very high voltage is assumed to be an open wire
					parg->cellopenbits |= (1 << i);
				}
				else
				{ // Here, high but not an unexpected open wire
					ctr1 += 1; // Count cells above max
				/* Note: cellibts is generated each pass. trip_max remains
				   until hyster_sw turns off, then back on. Otherwise they
				   are the same. */
					arrbal.cellbits |= (1<<i); // Set discharge fet for this cell
					pbq->trip_max       |= (1<<i); // Show it tripped the max
				}				
			}
		}
	}

	/* Have =>ALL<= installed cells tripped the max voltage? */
	if (pbq->cellspresent == pbq->trip_max)
	{ // Here, yes. Go into hysteresis (relaxation) mode
		pbq->hyster_sw = 1;
	}

	/* Save for others. */
	bqfunction.cellvopenbits = arrbal.cellbits;


	/* Default settings, if following doesn't override. */
	flag_dump = ((1<<( 8+0))|(1<<(10+16))); // DUMP OFF
	flag_trickle = 0; // Trickle charger rate zero
	flag_extchgr = (1<<( 6+16)); // External charger OFF

	/* Are =>ALL<= cells above max (target)? */
	if (ctr1 == plc->ncell)
	{ // All cells are above target.
			// PC6  = 0: DUMP2 (external charger control on/off fet)
			// PC8  = 1: DUMP (resistor load) 
			// PC10 = 0: DUMP (resistor load)
		flag_dump = ((1<<( 8+16))|(1<<(10+0))); // DUMP ON
	}
	else 
	/* Here, one or more cells are below max target. */
	{ // Are =>NO<= cells above max (target)?
		if (ctr1 == 0)
		{ // Here =>ALL<= cells below max target
			/* Is =>ANY<= cell below minimum? */
			if (ctr2 > 0)
			{ // Here, yes. Cell is in charging danger zone.
				flag_trickle = plc->tim1_ccr1_on_vlc; // Trickle rate very low
			}
			else
			{ // No cells: below minimum, or above maximum.
				if (pbq->hyster_sw == 0)
				{ // Hysteresis (relaxaxtion) is off, so charging is permitted.
					flag_trickle = plc->tim1_ccr1_on; // Trickle charger rate normal
					flag_extchgr = (1<<( 6+ 0)); // External charger On
				}
			}		
		}
	}
	/* Active the settings from the foregoing logic. */
	// Set DUMP on/off
	GPIOC->BSRR = flag_dump;
	// Set external charger on/off
	GPIOC->BSRR = flag_extchgr;
	// Set trickle charger rate
	TIM1->CCR1 = flag_trickle;



	return;
}