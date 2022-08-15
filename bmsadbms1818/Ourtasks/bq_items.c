/******************************************************************************
* File Name          : bq_items.c
* Date First Issued  : 07/11/2022
* Description        : routines associated with charging & cell balance
*******************************************************************************/
/* 
06/22/2022 Update for ADBMS1818 
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

#define USESORTCODE
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

//	xTaskNotifyWait(0,0xffffffff, &noteval1, 5500);
//	if (noteval1 == BQITEMSNOTE00) morse_trap(203);
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
 * void bq_items_selectfet(void);
 * @brief	: Go thought a sequence of steps to determine balancing
 * *************************************************************************/
uint32_t dbgcell[18];
uint32_t dbgcellbal;
void bq_items_selectfet(void)
{
	struct BQFUNCTION* pbq = &bqfunction; // Convenience pointer

#ifdef  USESORTCODE	
	struct BQCELLV* psort  = &bqfunction.cellv_bal[0]; // 
#endif	

	float* p = &bqfunction.cellv[0]; // Calibrated cell voltage
	uint16_t idata;
	int16_t i; // variable name selected in memory of FORTRAN

	pbq->battery_status = 0; // Reset battery status
	pbq->cellv_total    = 0; // Sum of installed cell voltages
	pbq->cellv_high     = 0; // Highest cell voltage
	pbq->cellbal        = 0; // Discharge fet bits
	pbq->cellv_max_bits = 0; // Cells over cellv_max
	pbq->cellv_min_bits = 0; // Cells below cellv_min
	pbq->cellv_vlc_bits = 0; // Cells below very low
	pbq->cellv_low      = pbq->lc.cellopen_hi; // Lowest cell initial voltage

/*   p->cellv_max   = 3500; // Max limit (mv) for charging any cell
   p->cellv_min   = 2600; // Min limit (mv) for any discharging
   p->cellv_vlc   = 2550; // Below this (mv) Very Low Charge (vlc)required */
	/* Check each cell reading. */
	for (i = 0; i < NCELLMAX; i++)
	{
		idata = (*p * 0.1f); // Convert calibrated float (100uv) to uint16_t (1mv)
dbgcell[i] = idata*10;
		if ((pbq->cellspresent & (1<<i)) != 0)
		{ // Here, cell position is installed
			if  ((idata <= pbq->lc.cellopen_lo)||(idata > pbq->lc.cellopen_hi))
			{ // Here, likely unexpected open wire
				pbq->battery_status |= BSTATUS_OPENWIRE;
			}
			else
			{ // Here, cell voltage reading looks valid
				if (idata > pbq->cellv_high)
				{ // Find max cell reading
					pbq->cellv_high = idata; // Save voltage
					pbq->cellx_high = i;  // Save cell index
				} 
				if (idata < pbq->cellv_low)
				{ // Find lowest cell reading
					pbq->cellv_low = idata; // Save voltage
					pbq->cellx_low = i;  // Save cell index
				} 
				if (idata > pbq->lc.cellv_max) 
				{ // Cell higher than target voltage
					pbq->cellbal       |=  (1 << i); // Set bit for discharge FET
					pbq->hysterbits_hi |=  (1 << i); // Hysteresis high set
					pbq->hysterbits_lo &= ~(1 << i); // Hysteresis low clear
					pbq->active_ct += 1; // Count number of cell fets selected
				}
				if (idata < pbq->hysterv_lo)
				{ // Cell below (target-hysteresis) voltage
					pbq->hysterbits_lo |=  (1 << i); // Hysteresis low set
					pbq->hysterbits_hi &= ~(1 << i); // Hysteresis high clear
				}
				if (idata > pbq->lc.cellv_max)
				{ // Cell is above the fixed max limited
					pbq->cellv_max_bits |= (1 << i); // Cells above cellv_max
					pbq->battery_status |= BSTATUS_CELLTOOHI; // One or more cells above max limit
				}
				if (idata < pbq->lc.cellv_min)
				{
					pbq->cellv_min_bits |= (1 << i); // Cells below cellv_min
					pbq->battery_status |= BSTATUS_CELLTOOLO;  // One or more cells below min limit
				}
				if (idata < pbq->lc.cellv_vlc)
				{ // Here, seriously discharged!
						pbq->cellv_vlc_bits |= (1 << i); // Cells below cellv_vlc	
						pbq->battery_status |= BSTATUS_CELLVRYLO; // One or more cells very low
				}
				
				pbq->cellv_total += idata; // Sum cell readings

#ifdef  USESORTCODE					
				psort->v = *p; psort->idx = i; // Copy to struct for sorting
				psort += 1;
#endif				
			}
		}
		p += 1; // Next cellv array
	}
dbgcellbal = pbq->cellbal;
	/* Set FET status.  */

#if 1
	/* Unusual situation check. */
	if (((pbq->battery_status & BSTATUS_NOREADING) != 0) ||
	    ((pbq->battery_status & BSTATUS_OPENWIRE)  != 0) )
	{ // Here serious problem, so no charging, or discharging
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR|FET_CHGR_VLC);
		pbq->cellbal = 0; // All cell balancing FETS off
//morse_trap(1);
		return;
	}
#endif	
	// Here it looks like we have a normal situation with good readings
	/* Check for out-of-limit voltages */

	/* Is any cell too low? */
	if (pbq->cellv_min_bits != 0) // Too low?
	{ // Here, one or more cells are below min limit
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER); // Disable discharge
		// The following assumes DUMP2 FET controls an external charger
		pbq->cellbal = 0; // All cell balancing FET bits off
//morse_trap(2);		
		// Here, one or more are too low, but are any still too high?
		if (pbq->cellv_high > pbq->lc.cellv_max)
		{ // EGADS YES! We cannot charge, but can selectively discharge
		  // until high cells become low enough to turn on charging.	
			pbq->fet_status &= ~(FET_CHGR|FET_DUMP2); // Disable charging
		}
	}

	/* Is any cell too high? */
	if (pbq->cellv_high > pbq->lc.cellv_max) // Too high?
	{ // Here, yes. One or more cells are over max limit
		// No charging, but discharging is needed
		pbq->fet_status &= ~(FET_CHGR|FET_DUMP2);
	}
	else
	{ // Here, no cells are too high.
		pbq->fet_status |= (FET_CHGR|FET_DUMP2);
	}

	/* Healthy Hysteresis Handling. */
	if (pbq->hyster_sw == 0)
	{ // Charging/balancing towards targetv
		if (pbq->hysterbits_hi == pbq->cellspresent)
		{ // Here, all cells have gone over the target voltage
			pbq->hyster_sw = 1; // Start "relaxation"
			pbq->hysterbits_lo = 0; // Reset low cell bits
		}
	}
	else
	{ // Relaxation hysteresis is in effect
		// Everything off
		pbq->cellbal   = 0;
//morse_trap(3);
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR|FET_CHGR_VLC);

		// Stop relaxation when one or more cells hits hysteresis low end
		if (pbq->hysterbits_lo != 0)
		{ // One or more cells hit the low end of hysteresis
			pbq->hysterbits_hi = 0; // Reset high cell bits
			pbq->hyster_sw  = 0; // Set hysteresis switch off
		}
	}

#ifdef  USESORTCODE
	/* Sort on Cell voltages. (qsort does ascending) */
	psort  = &pbq->cellv_bal[0];
	qsort(psort, pbq->lc.ncell, sizeof(struct BQCELLV), compare_v);
#endif

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