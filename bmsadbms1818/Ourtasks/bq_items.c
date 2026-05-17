/******************************************************************************
* File Name          : bq_items.c
* Date First Issued  : 07/11/2022
* Description        : routines associated with charging & cell balance
*******************************************************************************/
/* 
06/22/2022 Update for ADBMS1818 
02/28/2026 Major revisions
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "DTW_counter.h"
#include "main.h"
#include "morse.h"
#include "LedTask.h"
#include "bq_items.h"
#include "BQTask.h"
#include "fetonoff.h"
#include "bmsspi.h"

void bq_items_selectfet(void);

#ifdef  USESORTCODE
	#include <stdlib.h> // for qsort
	static int compare_v(const void *a, const void *b);
	void bq_items_qsortV(struct BQCELLV* p);
#endif

struct BQFUNCTION bqfunction;	

#define TICKS_INC 2000 // Duration between balance updates (ms)
static uint32_t ticks_next;

/* static */ struct BMSREQ_Q  bmstask_q_readbms;

/* *************************************************************************
 * void bq_items_init(void);
 * @brief	: Cell balance 
 * *************************************************************************/
void bq_items_init(void)
{
	ticks_next = TICKS_INC + xTaskGetTickCount();
	return;
}
/* *************************************************************************
 * static void bq_items_q(uint8_t reqcode);
 * @brief	: Queue request (for BMSTask handling)
 * *************************************************************************/
static void bq_items_q(uint8_t reqcode)
{
	BaseType_t ret;
	struct BMSREQ_Q* pq = &bmstask_q_readbms;

	bmstask_q_readbms.reqcode = reqcode;
	bmstask_q_readbms.noteyes = 0; // Do not notify calling task
	bmstask_q_readbms.done = 1; // Show request queued
	ret = xQueueSendToBack(BMSTaskReadReqQHandle, &pq, 0);
	if (ret != pdPASS) morse_trap(201);
}
/* *************************************************************************
 * uint8_t bq_items(void);
 * @brief	: Cell balance (most likely called from 'main.c')
 * @return  : 0 = nothing worth looking at
 *          : 1 = READBMS completed FETSET started
 *          : 2 = FETSET completed
 * *************************************************************************/
uint8_t dbgf = 17;
uint8_t dbgfct;

static uint8_t state = 0;
uint8_t bq_items(void)
{
	uint8_t retx = 0;  // Default return code = nothing done
	 /* Don't queue another request until the previous is finished. */
	// Note: case 2 & 3 could be combined by queueing two requests
	// but requires to request structs.
	if (bmstask_q_readbms.done != 0) return 0;

	switch(state)
	{
	case 0: // OTO Initialize
	 	bmstask_q_readbms.bmsTaskHandle = xTaskGetCurrentTaskHandle();
	 	bmstask_q_readbms.tasknote = BQITEMSNOTE00;
	 	ticks_next = TICKS_INC + xTaskGetTickCount();
	 	state = 1;
	 /* Fall through. */
	case 1: /* Every 'x' ms check balancing. */
		if ((int32_t)(xTaskGetTickCount() - ticks_next) < 0) 
			break;

		ticks_next += TICKS_INC; // Next balancing cycle tick count
		state = 2;
	 /* Fall through. */
	case 2: /* Get read & get current register settings. (REQ_READBMS) */
		bq_items_q(REQ_READBMS); // Queue request
		state = 3;
		break;

	case 3: /* Wait for REQ_READBMS completion. */
		if (bmstask_q_readbms.done != 0)		
			break;
		retx = 1;
	     /* Select discharge FETs, and update other FET selections. */
		bq_items_selectfet(); // Determine on/off for all FETs

#if 0 // Testing discharge FET bit operation
//dbgfct += 1; // Step through bits
//if (dbgfct > 3) // Stay on each or a while
{
	dbgfct = 0;
//	dbgf += 1; if (dbgf > 17) dbgf = 0;
dbgf = 1; // Set & compile for each
	bmstask_q_readbms.setfets = (1 << dbgf);
}
#endif
		/* Activate the settings from the foregoing logic. */
		// Update discharge FETs
		bmstask_q_readbms.setfets = bqfunction.cellbal;
		bq_items_q(REQ_SETFETS); // Queue BMS request
		// Other FETs
		fetonoff_status_set(bqfunction.fet_status);
		state = 4; 
		break;

	case 4: // Wait for REQ_SETFETS to complete
		if (bmstask_q_readbms.done != 0)		
			break;
		retx  = 2;  // Return shows REQ_SETFETS completed
		state = 1;  // Ready for a new cycle
		break;		
	}
	return retx;
}
/* *************************************************************************
 * static void bq_items_updatestatus(struct BQFUNCTION* pbq);
 * @brief	: Update buffered status bytes (used by cancomm_items in CANTask)
 * @param   : pointer to bqfunction struct
 * *************************************************************************/
/* Since the status bits are the result of loop through all cells and the status
is reported via a different task, the results are buffered.
*/
static void bq_items_updatestatus(struct BQFUNCTION* pbq)
{
	pbq->buf_battery_ext_status = pbq->battery_ext_status; // [3] Cell status code bits (extended)
	pbq->buf_battery_status     = pbq->battery_status;     // [4] Cell status code bits 
	pbq->buf_fet_status         = pbq->fet_status;         // [5] This controls on/off of FETs
	pbq->buf_mode_status        = pbq->mode_status;        // [6] Mode bits
	pbq->buf_temp_status        = pbq->temp_status;        // [7] Temperature status bits

	pbq->buf_cellv_max_bits  = pbq->cellv_max_bits;  // Cells above cellv_max
	pbq->buf_cellv_min_bits  = pbq->cellv_min_bits;  // Cells below cellv_min
	pbq->buf_cellv_vlc_bits  = pbq->cellv_vlc_bits;  // Cells below cellv_vlc
	pbq->buf_cellv_tdt_bits  = pbq->cellv_vlc_bits;  // Cells below (max-Delta) & tripped	
	pbq->buf_cellv_launch_ng = pbq->cellv_launch_ng; // Cells below launch no-go
	pbq->buf_hysterbits_lo      = pbq->hysterbits_lo; // Bits for cells that fell below hysterv_lo
	pbq->buf_hysterbits_lo_save = pbq->hysterbits_lo_save;// Prev hysterbits_lo
	pbq->buf_cellv_min_loaded_bits  = pbq->cellv_min_loaded_bits;	

	return;
}
/* *************************************************************************
 * void bq_items_selectfet(void);
 * @brief	: Go thru a sequence of steps to determine balancing
 * *************************************************************************/
uint32_t dbgtrc;
uint32_t dbgcellbal;
void bq_items_selectfet(void)
{
	struct BQFUNCTION* pbq = &bqfunction; // Convenience pointer
	float* p = &bqfunction.cellv[0]; // Calibrated cell voltage
	uint32_t idata; // Integer cell voltage in 1 mv units
	int16_t i; // variable name selected in memory of FORTRAN

dbgtrc = 0; // Debug: bits for checking logic

	pbq->battery_ext_status = 0; // Reset battery extended status	
	pbq->battery_status  = 0; // Reset battery status
	pbq->cellv_total     = 0; // Sum of installed cell voltages
	pbq->cellv_high      = 0; // Highest cell initial voltage
	pbq->cellv_max_bits  = 0; // Cells over cellv_max
	pbq->cellv_min_bits  = 0; // Cells below cellv_min
	pbq->cellv_vlc_bits  = 0; // Cells below very low
	pbq->cell_tdt_bits   = 0; // Cells below Target-Delta & tripped
	pbq->cell_amd_bits   = 0; // Cells above (max - delta)
	pbq->cellvopenbits   = 0; // Cell positions with open wire detected
	pbq->cellbal         = 0; // Discharge fet bits (will be sent to a bms readout queue)
	pbq->cellv_launch_ng = 0; // Cell bits for cells below launch no go
	pbq->cellv_min_loaded_bits = 0; // Cells far too low even under load
	pbq->cellv_sum       = 0;

	pbq->cellv_low       = pbq->lc.cellopen_hi; // Lowest cell initial voltage

	/* Check all the cell readings versus various volage thresolds. */
	for (i = 0; i < NCELLMAX; i++)
	{
		idata = (*p * 0.1f); // Convert calibrated float (100uv) to uint32_t (1mv)

		if ((pbq->cellspresent & (1<<i)) != 0) // Skip cells not installed
		{ // Here, cell position is installed
			if  ((idata <= pbq->lc.cellopen_lo)||(idata > pbq->lc.cellopen_hi))
			{ // Here, likely unexpected open wire
				pbq->cellvopenbits |= (1 << i);;   // Bits for unexpected open cells (1 = open wire suspected) 
			}
			else
			{ // Here, this cell voltage reading looks valid
			// Sum cell voltages
				pbq->cellv_sum += idata;

			// Find max cell reading in this scan
				if (*p > pbq->cellv_high_f)
				{ // Here, this cell is higher
					pbq->cellv_high_f = *p; // Save voltage float (0.1 mv)
					pbq->cellv_high   = idata; // Save uint (mv)
					pbq->cellx_high   = i;  // Save cell index
				} 

			// Find lowest cell reading
				if (idata < pbq->cellv_low)
				{ // Here, this cell is lower
					pbq->cellv_low_f = *p; // Save voltage float (0.1 mv)
					pbq->cellv_low   = idata; // Save uint (mv)
					pbq->cellx_low   = i;  // Save cell index
				} 

			// Cell is above max limit?
				if (idata > pbq->lc.cellv_max)
				{ // Cell is above (and could be in danger!)
					pbq->cellv_max_bits |= (1 << i); // Cells above cellv_max					
				}
					
			// Cell above (cellv_max - cellv_tgtdelta)?
				if (idata > pbq->cellv_tmdelta) // (cellv_max - cellv_tgtdelta)
				{ // Here, voltage is above (max - delta)
					pbq->cell_amd_bits |= (1 << i); // Cells above (max - delta)
				}

			//  Cell lower than launch no-go threshold?
				if (idata < pbq->lc.cellv_launch_ng)
				{ // Cell voltage is below launch no go threshold
						pbq->cellv_launch_ng = (1 << i);
				}

			// Cell below end of relaxation/self-discharge?
				if (idata < pbq->hysterv_lo)
				{ // Cell below (target-hysteresis) voltage
					pbq->hysterbits_lo |=  (1 << i); // Hysteresis low set
				}

			// Cell too low for any discharging?
				if (idata < pbq->lc.cellv_min)
				{ // Here too low for any discharging
					pbq->cellv_min_bits |= (1 << i); // Cells below cellv_min
				}

			// Cell so low that reduced charge current required?
				if (idata < pbq->lc.cellv_vlc)
				{ // Here, seriously discharged! 
						pbq->cellv_vlc_bits |= (1 << i); // Cells below cellv_vlc							
				}

			// Cell far too low even under load
				if (idata < pbq->lc.cellv_min_loaded) 
				{ // Cells too low even under load (mv))
					pbq->cellv_min_loaded_bits |= (1 << i); // Cells too low even under load (mv)
				}
				
				pbq->cellv_total += idata; // Sum cell readings
			}
		}
		p += 1; // Next cellv array
	}

	pbq->cellv_sum_f = (float)pbq->cellv_sum * 0.1; // Convert to mv


	/* Summary of scan of cell readings. */
	if (pbq->cellvopenbits != 0)
	{  // Bits for unexpected open cells (1 = open wire suspected) 
		pbq->battery_status |= BSTATUS_OPENWIRE;		
	}

	if (pbq->cellv_max_bits != 0)
	{ // One or more cells are over max (pbq->lc.cellv_max)
		pbq->battery_status |= BSTATUS_CELLTOOHI; // One or more cells above max limit
		pbq->celltrip |= pbq->cellv_max_bits;     // Cumulative cells tripped 
		if (pbq->celltrip == pbq->cellspresent)
		{ // Here, all cells have been tripped
			pbq->battery_ext_status |= BSTATUS_X_ALLTRIPPED;
		}
	}

	if (pbq->cell_amd_bits != 0)
	{ // One or more cells above (max - hysteresis)
		pbq->cell_tdt_bits &= pbq->celltrip;
		pbq->battery_ext_status |= BSTATUS_X_ABOVENTRIP;
	}
	
	if (pbq->cellv_launch_ng != 0)
	{ // One of more cells below launch no-go threshold
		pbq->battery_ext_status |= BSTATUS_X_LAUNCH_NG;
	}

	if (pbq->cellv_min_bits != 0)
	{ // One or more cells below min limit (lc.cellv_min, e.g. 2200)
		pbq->battery_status |= BSTATUS_CELLTOOLO;  
	}

	if (pbq->cellv_vlc_bits != 0)
	{ // Cells require very low charge current recovery, lc.cellv_vlc , e.g. 2100
		pbq->battery_status |= BSTATUS_CELLVRYLO; // One or more cells very low
	}

	if (pbq->cellv_min_loaded_bits != 0)
	{ // One or more below lc.cellv_min_loaded, e.g. 1800
		pbq->battery_ext_status |= BSTATUS_X_MINLOADED;
	}

dbgcellbal = pbq->cellbal;

	/* Set FET status.  */
/* NOTE: DUMP2 is assumed to control an external module charger. */
	/* CAN msgs set FETs ON|OFF: DUMP DUMP2 HEATER, set a request for this task.  */
   //	pbq->fet_status &= ~(FET_DUMP|FET_DUMP2|FET_HEATER); // set all off
	// Set FETs if command active and not timed out
	for(i = 0; i < BQREQ_SIZE; i++)
	{
		if (pbq->bqreq[i].req == 1)
		{ // Here, active command in progress
dbgtrc |= (1<<12); // Debug Trace	
			if ((int)(xTaskGetTickCount()-pbq->bqreq[i].tim) > 0)
			{ // Here, time has expired
				pbq->bqreq[i].req = 0; // Reset command status
				if (i == REQ_DUMP  ) pbq->fet_status &= ~FET_DUMP; else
				if (i == REQ_DUMP2 ) pbq->fet_status &= ~FET_DUMP2;else
				if (i == REQ_HEATER) pbq->fet_status &= ~FET_HEATER;else
				if (i == REQ_TRICKL) pbq->fet_status &= ~FET_CHGR;	
dbgtrc |= (1<<0); // Debug Trace				
			}
			else
			{ // Here, request is active, and time has not expired
				if (pbq->bqreq[i].on == 1)
				{ // Here command was ON
dbgtrc |= (1<<13); // Debug Trace	
 					if (i == REQ_DUMP  ) pbq->fet_status |= FET_DUMP; else
					if (i == REQ_DUMP2 ) pbq->fet_status |= FET_DUMP2;else
					if (i == REQ_HEATER) pbq->fet_status |= FET_HEATER;else
					if (i == REQ_TRICKL) pbq->fet_status |= FET_CHGR;	
				}
				else
				{ // Here, command was OFF.
					if (i == REQ_DUMP  ) pbq->fet_status &= ~FET_DUMP; else
					if (i == REQ_DUMP2 ) pbq->fet_status &= ~FET_DUMP2;else
					if (i == REQ_HEATER) pbq->fet_status &= ~FET_HEATER;else								
					if (i == REQ_TRICKL) pbq->fet_status &= ~FET_CHGR;	
				}
			}
		}
	}
/* ===> Let following cell voltage limits override the above FET status settings. <=== */

	/* Unusual situation check. */
	if (((pbq->battery_status & BSTATUS_NOREADING) != 0) ||
	    ((pbq->battery_status & BSTATUS_OPENWIRE)  != 0) )
	{ // Here serious problem, so no charging, or discharging
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR|FET_CHGR_VLC);
		pbq->cellbal = 0; // All cell balancing FETS off
dbgtrc |= (1<<1); // Debug Trace

		// Update buffered status bytes before returnning
		bq_items_updatestatus(pbq);
		return;
	}

	// Here it looks like we have a normal situation with good readings
	/* Check for out-of-limit voltages */

	/* Are one or more cells too low? */
	if (pbq->cellv_min_bits != 0) // Too low?
	{ // Here, one or more cells are below min limit (cellv_min)
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER); // Disable discharge
pbq->hyster_sw_trip = 0; // Stop a discharge test (redundant)
dbgtrc |= (1<<2);		
		// The following assumes DUMP2 FET controls an external charger
		// Here, one or more are too low, but are any still too high?
		if (pbq->cellv_max_bits != 0)
		{ // EGADS YES! We cannot charge, but can selectively discharge
		  // until high cells become low enough to turn on charging.	
			pbq->fet_status &= ~(FET_CHGR|FET_DUMP2); // Disable charging
			pbq->cellbal |= pbq->cellv_max_bits; // Set bits for discharge FETs
		}
		else
		{ // Here, no cells over max, but one or more are below cellv_min 
			pbq->cellbal = 0; // (JIC) All cell balancing FET bits off
dbgtrc |= (1<<3);						
		}
	}

	/* Are one or more cells too high? */
	if (pbq->cellv_max_bits != 0)
	{ // Here, yes. One or more cells are over max limit
		// No charging, but discharging is needed. Turn off both chargers
		pbq->fet_status &= ~(FET_CHGR|FET_DUMP2); // (DUMP2 external charger control)
dbgtrc |= (1<<4);
	}
	else
	{ // Here, no cells are too high. Set chargers on.
		pbq->fet_status |= FET_CHGR; // On-board charger ON
	}

	/* DUMP2 external charger only charges when no cells have tripped. */
	if (pbq->celltrip == 0)
	{ // Here, no cells have charged over max
		if (!((pbq->bqreq[REQ_DUMP2].req == 1) && (pbq->bqreq[REQ_DUMP2].on == 0)))
		{ // Here CAN command request is active and request is OFF
/* DUMP2 would have been set OFF above if command was active and OFF (0), so
   skip the following if that is the case. */			
			pbq->fet_status |= FET_DUMP2; // (DUMP2 external charger control ON)
		}
dbgtrc |= (1<<5);		
	}

	/* DUMP2 external charger turns OFF when first cell trips max. */
	if (pbq->celltrip != 0)
	{ // Here, one or more cells have charged over max.
		pbq->fet_status &= ~FET_DUMP2; // (DUMP2 external charger control OFF)
dbgtrc |= (1<<6);		
	}

	/* Relaxation/self-discharge versus charging mode. */
	if (pbq->hyster_sw == 0)
	{ // ======> Charging/balancing is in effect <=======		
		if ((pbq->celltrip == pbq->cellspresent) || (pbq->hyster_sw_trip == 1))
		{ // Here, ALL installed cells are over the (target voltage - delta)
		 //  Or, the switch was set (by a CAN msg...)
			pbq->hysterbits_lo = 0; // Reset low cell bits
			pbq->hyster_sw     = 1; // ===> Set "relaxation/self-discharge" mode <===
			pbq->cellbal       = 0; // Discharge FETs off.
			// Everybody off.
			pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR|FET_CHGR_VLC);
			if (pbq->hyster_sw_trip == 1)
			{
				pbq->hyster_sw_trip = 9; // Set bogus value 	
//?				pbq->fet_status |= FET_DUMP;  // Discharge test 
			}			
dbgtrc |= (1<<7);
		}
		else
		{ // Here, not all cells have tripped (AND hyster switch is 0)
			if (pbq->cell_tdt_bits != 0)
			{ // One or more cells above (max - hysteresis/delta) & tripped
				// Set discharge fet ON. Ideal is discharge matches charge current
				pbq->cellbal |= pbq->cell_tdt_bits;
			}
			else
			{ // No cells above (max - hysteresis/delta) AND tripped
				// Set cell discharge fet OFF. Tripped cell fell below|equal (max - delta)
				pbq->cellbal = 0;
			}
			pbq->fet_status |= FET_CHGR; // On-board charger ON
		}
	}
	else
	{ // ======> Relaxation/hysteresis is in effect <=======
		  // When all cells are above max, bring them down
		  //if (pbq->cellv_max_bits == pbq->cellspresent)
		// When any cell is above max bring it down
		if (pbq->cellv_max_bits != 0)
		{ // Here, all installed cells above max
/* This for bringing the pack down to a (new lower) target voltage. */			
			pbq->cellbal = pbq->cellv_max_bits; // Discharge FETs on all
dbgtrc |= (1<<8);			
		}
		else
		{ // Here, no cells above max so relax/hyster
				/* CAN msgs can set FETs but not override high & low voltage limits.
	   The timeout is set when the CAN command is received (cancomm_items.c) */
			if ((int)(xTaskGetTickCount() - pbq->cansetfet_tim) < 0)
			{ 
				pbq->cellbal = pbq->cansetfet;
dbgtrc |= (1<<9);				
			}
			else
			{
				pbq->cansetfet = 0; // Avoid time rollover turning fets on.
				pbq->cellbal   = 0; // Discharge fet bits
dbgtrc |= (1<<14);					
			}
		}

		/* Discharge test set switch from CAN msg. */
		if (pbq->discharge_test_sw != 0)
		{ // Here, discharge test ON: 
			// Turn DUMP2 (external chgr), FET charger off
			pbq->fet_status &= ~(FET_DUMP2|FET_CHGR|FET_CHGR_VLC);
			// Turn on heavy load for discharge testing
			pbq->fet_status |= FET_DUMP;
dbgtrc |= (1<<10);			
		}
		else
		{ // Here, normal self-discharge mode.
			// Turn all loads and chargers off
//			pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR|FET_CHGR_VLC);
			pbq->fet_status &= ~(FET_CHGR|FET_CHGR_VLC);
		}

		// Stop relaxation when one or more cells hits relaxation/self-discharge low end
		if ((pbq->hysterbits_lo != 0) || (pbq->hyster_sw_trip == 0))
		{ // One or more cells hit the low end of hysteresis/relaxation/self-discharge voltage
			pbq->hysterbits_lo_save = pbq->hysterbits_lo; // Save who it was for review later
			pbq->hyster_sw_trip    = 9; // Set bogus value 
			pbq->hyster_sw         = 0; // Set hysteresis switch off
			pbq->celltrip          = 0; // Reset cells that went over max
			pbq->cell_tdt_bits     = 0; // Reset cells below (max - delta) AND tripped
			pbq->discharge_test_sw = 0; // Reset discharge test, if it was on.
dbgtrc |= (1<<11);			
		}
	}
	/* Update status byte for 'mode' */
	if (pbq->hyster_sw == 0)
	{
		pbq->mode_status &= ~MODE_SELFDCHG;
	}
	else
	{
		pbq->mode_status |= MODE_SELFDCHG;		
	}

	if (pbq->celltrip == 0)	
	{
		pbq->mode_status &= ~MODE_CELLTRIP;
	}
	else
	{
		pbq->mode_status |= MODE_CELLTRIP;		
	}

	// Update buffered status bytes before returnning
	bq_items_updatestatus(pbq);
	return;
}
/* ************************************************************************* 
 * Comparison function for qsort
 * *************************************************************************/
#ifdef  USESORTCODE
static int compare_v(const void *a, const void *b)
{
    const struct BQCELLV *da = (const struct BQCELLV *) a;
    const struct BQCELLV *db = (const struct BQCELLV *) b;
     
    return (da->v > db->v) - (da->v < db->v);
}
/* *************************************************************************
 * void bq_items_qsortV(struct BQCELLV* p);
 * @brief	: Sort array by voltage
 * @param   : p = pointer to NCELL cell struct array
 * *************************************************************************/
void bq_items_qsortV(struct BQCELLV* p)
{
	/* Sort (ascending) on Cell voltages. */
	qsort(p, bqfunction.lc.ncell, sizeof(struct BQCELLV), compare_v);
	return;
}
#endif