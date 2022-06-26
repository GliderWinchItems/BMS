/******************************************************************************
* File Name          : bq_items.c
* Date First Issued  : 10/02/2021
* Description        : routines associated with charging task
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

#define USESORTCODE
#ifdef  USESORTCODE
	#include <stdlib.h> // for qsort
	static int compare_v(const void *a, const void *b);
	void bq_items_qsortV(struct BQCELLV* p);
#endif

struct BQFUNCTION bqfunction;	

/* Sorted by cell voltage for display? */
#ifdef  USESORTCODE
struct BQCELLV dbsort[NCELLMAX];
#endif

#define TICKS_INC 250 // Duration between balance updates (ms)
static uint32_t ticks_next;

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
 * uint8_t bq_items(void);
 * @brief	: Cell balance 
 * @return  : 0 = did nothing; 1 = Did read and balance seq
 * *************************************************************************/
uint8_t bq_items(void)
{
	/* Every 10 ms do something. */
	if ((int32_t)(xTaskGetTickCount() - ticks_next) < 0) return 0;

	bmsdriver(REQ_READBMS); // Read cells + GPIO 1 & 2
	bq_items_selectfet();   // Update discharge FETs

	// Update other FETs

	ticks_next += TICKS_INC;
	return 1;
}

/* *************************************************************************
 * void bq_items_selectfet(void);
 * @brief	: Go thought a sequence of steps to determine balancing
 * *************************************************************************/
void bq_items_selectfet(void)
{
	struct BQFUNCTION* pbq = &bqfunction; // Convenience pointer
	struct BQCELLV* psort  = &pbq->cellv_bal[0]; // Working array for cell balancing
	float* p = &bmsuser.cellv[0]; // Calibrated cell voltage
	int16_t tmpv; // Cell voltages above 'tmp' are candidates for discharging
	int16_t i; // variable name selected in memory of FORTRAN

	/* This saves recomputing these in the loop. */
	uint8_t ncellm1 = pbq->lc.ncell - 1; // e.g. equals 15 for 16 cell module
	uint8_t ncellm2 = ncellm1 - 1; // E.g. equals 14 for 16 cell module

	pbq->battery_status = 0; // Reset status
	pbq->cellv_total    = 0; // Cell summation
	pbq->cellv_high     = 0; // Highest cell voltage
	pbq->cellv_low      = 0; // Lowest cell voltage

	/* Set discharge FET bits. */
	pbq->cellbal = 0; // Begin with no cell fets set

	/* Check each cell reading. */
	for (i = 0; i < pbq->lc.ncell; i++)
	{
		if ((bpq->cellspresent & (1<<i)) != 0)
		{ // Here, cell position is installed
			if  ((*p <= pbq->lc.cellopen_lo) ||(*p > pbq->lc.cellopen_hi))
			{ // Here, likely unexpected open wire
				pbq->battery_status |= BSTATUS_OPENWIRE;
			}
			else
			{ // Here, cell reading looks valid
				if (*p > pbq->cellv_high)
				{ // Find max cell reading
					pbq->cellv_high = *p; // Save voltage
					pbq->cellx_high = i;  // Save cell index
				} 
				if (*p < pbq->cellv_low)
				{ // Find lowest cell reading
					pbq->cellv_low = *p; // Save voltage
					pbq->cellx_low = i;  // Save cell index
				} 
				if (*p > pbq->targetv)
				{ // Note: targetv may change (hysteresis & storage mode)
					pbq->cellbal       |=  (1 << i); // Set bit for discharge FET
					pbq->hysterbits_hi |=  (1 << i); // Hysteresis high set
					pbq->hysterbits_lo &= ~(1 << i); // Hysteresis low clear
					pbq->active_ct += 1; // Count number of cells selected
				}
				if (*p < pbq->hysterv_lo)
				{ // 
					pbq->hysterbits_lo |=  (1 << i); // Hysteresis low set
					pbq->hysterbits_hi &= ~(1 << i); // Hysteresis high clear
				}

				pbq->cellv_total += *p; // Sum cell readings
				psort->v = *p; psort->idx = i; // Copy to struct for sorting
				psort += 1;
			}
		}
		p += 1; // Next cell
	}

	/* Unusual situation check. */
	if (((pbq->battery_status & BSTATUS_NOREADING) != 0) ||
	    ((pbq->battery_status & BSTATUS_OPENWIRE)  != 0) )
	{ // Here no charging, no discharging
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR);
		pbq->cellbal = 0; // All cell balancing FETS off
		return;
	}
	// Here it looks like we have a normal situation with good readings
	/* Check for out-of-limit voltages */

	/* Is any cell too low? */
	if (pbq->cellv_low < pbq->lc.cellv_min) // Too low?
	{ // Here, one or more cells are below min limit
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER); // Disable discharge
		// The following assumes DUMP2 FET controls an external charger
		pbq->cellbal = 0; // All cell balancing FET bits off
		pbq->battery_status |= BSTATUS_CELLTOOLO; // Update status 

		// One or more are too low. Are any still too high?
		if (pbq->cellv_high > pbq->lc.cellv_max)
		{ // EGADS YES! We cannot charge, but can selectively discharge
		  // until high cells low enough to turn on charging.	
			pbq->fet_status &= ~(FET_CHGR|FET_DUMP2); // Disable charging
		}
	}
	else
	{ // Here, no cell is too low.
		pbq->battery_status &= ~BSTATUS_CELLTOOLO; // Update status 
	}

	/* Is any cell too high? */
	if (pbq->cellv_high > pbq->lc.cellv_max) // Too high?
	{ // Here, yes. One or more cells are over max limit
		// No charging, but discharging is needed
		pbq->fet_status &= ~(FET_CHGR|FET_DUMP2);
		pbq->battery_status |= BSTATUS_CELLTOOHI;
	}
	else
	{ // Here, no cells are too high.
		pbq->battery_status &= ~BSTATUS_CELLTOOHI; // Update status
		pbq->fet_status |= FET_CHGR;
	}

	/* Healthy Hysteresis Handling. */
	// Cell balancing
/*	
	uint32_t targetv;     // Balance voltage target
	float hysterv_lo;     // Hysteresis bottom voltage.
	uint32_t trip_max;    // Bits for cells that reached cellv_max (target)
	uint8_t hyster_sw;    // Hysteresis switch: 1 = peak was reached	 */
	if (hyster_sw == 0)
	{ // Charging/balancing towards targetv
		if (pbq->hysterbits == bpq->cellspresent)
		{ // Here, all cells have gone of target voltages
			hyster_sw =	1; // Start "relaxation"
			pbq->targetv = pbq->hysterv_lo; // New targetv
		}
	}
	else
	{ // Relaxation hysteresis is in effect
		if (pbq->hysterbits_lo != 0)
		{ // One or more cells hit the low end of hysteresis
			pbq->targetv = p->lc.cellv_max; // targetv 
			hyster_sw =	0;
		}
	}


	/* Here, balancing is permitted. Make cell selections.
	Goal: Build a 32b word with bits set for setting cell 
	discharge FETs. BQ97952 uses 16b, ADBMS1818 uses 18b. */

	// A cell voltages above 'tmpv' are candidates for discharging.
	tmpv = pbq->cellv_low + pbq->lc.cellbal_del;

	// Limit the number of cells discharging at any given cycle
	pbq->active_ct = 0;    // Count number of cell bits set


#ifdef  USESORTCODE
	/* Sort on Cell voltages. (qsort does ascending) */
	psort  = &pbq->cellv_bal[0];
	qsort(psort, pbq->lc.ncell, sizeof(struct BQCELLV), compare_v);

// debugging: copy array for monitoring in 'main'
for (i= 0;i<pbq->lc.ncell;i++) dbsort[i] = pbq->cellv_bal[i];	
#endif

	/* Select cells for discharging. */
	pbq->cellbal = 0; // Begin with no cells set
	if (pbq->hyster_sw == 0)
	{
		for (i = 0; i < pbq->lc.ncell; i++)
		{
			if ((bpq->cellspresent & (1<<i)) != 0)
			{ // Here, cell position is installed
				if (cellv_bal[i] > pbq->targetv)
				{
					pbq->cellbal |= (1 << i); // Set bit for discharge FET
					pbq->active_ct += 1; // Count number of cells selected
				}
			}
		}
	}

	/* Set discharge FETs */
	bmsspi_setfets(pbq->cellbal);

	/* Activate the settings from the foregoing logic. */
	fetonoff_status_set(pbq->fet_status);

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