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

#define USESORTCODE
#ifdef  USESORTCODE
	#include <stdlib.h> // for qsort
	static int compare_v(const void *a, const void *b);
	void bq_items_qsortV(struct BQCELLV* p);
#endif

struct BQFUNCTION bqfunction;	
struct BQCELLV dbsort[NCELLMAX];

static uint32_t dtw_next;

#define DTW10MS (16000*10) // 10 ms system ctr
/* *************************************************************************
 * void bq_items_init(void);
 * @brief	: Cell balance 
 * *************************************************************************/
void bq_items_init(void)
{
	dtw_next = DTWTIME + DTW10MS;
	return;
}
/* *************************************************************************
 * void bq_items(void);
 * @brief	: Cell balance 
 * *************************************************************************/
void bq_items(void)
{
	/* Every 10 ms do something. */
	if ((int32_t)(DTWTIME - dtw_next) < 0) return;

	bmsdriver(REQ_READBMS); // Read cells + GPIO 1 & 2
	bq_items_selectfet();

	return;
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
	int16_t i,j; // variable name selected in memory of FORTRAN

	/* This saves recomputing these in the loop. */
	uint8_t ncellm1 = pbq->lc.ncell - 1; // e.g. equals 15 for 16 cell module
	uint8_t ncellm2 = ncellm1 - 1; // E.g. equals 14 for 16 cell module

	pbq->battery_status = 0; // Reset status
	pbq->cellv_total    = 0; // Cell summation
	pbq->cellv_high     = 0; // Highest cell voltage
	pbq->cellv_low      = 32767; // Lowest cell voltage

	/* Check each cell reading. */
	for (i = 0; i < pbq->lc.ncell; i++)
	{
		if ((bpq->cellspresent & (1<<i)) != 0)
		{ // Here, cell position is installed
			if  ((*p <= 0) ||(*p > 4500.0f))
			{ // Here, zero or negative likely unexpected open wire
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
				pbq->cellv_total += *p; // Sum cell readings
				psort->v = *p; psort->idx = i; // Copy to struct for sorting
				psort += 1;
			}
		}
		p += 1; // Next cell
	}

// Debugging: visual check duration between calls	
morse_string("E",GPIO_PIN_1);

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

	/* Here, balancing is permitted. Make cell selections.
	Goal: Build a 32b word with bits set for setting cell 
	discharge FETs. BQ97952 uses 16b, ADBMS1818 uses 18b. */

	// A cell voltages above 'tmp' are candidates for discharging.
	tmp = pbq->cellv_low + pbq->lc.cellbal_del;

	// Limit the number of cells discharging at any given cycle
	pbq->active_ct = 0;    // Count number of cell bits set

	/* Sort on Cell voltages. (qsort does ascending) */
	psort  = &pbq->cellv_bal[0];
	qsort(psort, pbq->lc.ncell, sizeof(struct BQCELLV), compare_v);

// debugging: copy array for monitoring in 'main'
for (i= 0;i<pbq->lc.ncell;i++) dbsort[i] = pbq->cellv_bal[i];	

	/* Start with highest voltage cell and decrement index. */ 
	i = ncellm1; // Sorted array index for highest cell voltage

	/* Select cells for discharging. */
	pbq->cellbal = 0; // Begin with no cells set
	for (i = 0; i < pbq->lc.ncell; i++)
	{
		if ((bpq->cellspresent & (1<<i)) != 0)
		{ // Here, cell position is installed	
			pbq->cellbal |= (1 << i); // Set bit for discharge FET
			pbq->active_ct += 1; // Count number of cells selected
		}
	}
	return;
}
/* ************************************************************************************************************ 
 * Comparison function for qsort
 * ************************************************************************************************************ */
#define USESORTCODE
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