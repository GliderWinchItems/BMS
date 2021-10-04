/******************************************************************************
* File Name          : bq_items.c
* Date First Issued  : 10/02/2021
* Description        : routines associated with charging task
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"
#include "LedTask.h"
#include "bq_items.h"
#include "BQTask.h"
#include <stdlib.h> // for qsort

/* *************************************************************************
 * void bq_items_seq(int16_t* p);
 * @brief	: Go thought a sequence of steps to determine balancing
 * @param   : p = pointer to 16 cell voltage array
 * *************************************************************************/
void bq_items_seq(int16_t* p)
{
	struct BQFUNCTION* pbq = &bqfunction; // Convenience pointer
	struct BQCELLV*   pbal = &bqfunction.cellv_bal[0];
	int i;
	int16_t* pv = &pbq->cellv_latest[0];

	pbq->battery_status = 0; // Reset status
	pbq->cellv_ok       = 0; // Flag: assume readings NG
	pbq->cellv_total    = 0;
	pbq->cellv_high     = 0;
	pbq->cellv_low      = 32767;


	/* Check each cell reading. */
	for (i = 0; i < 16; i++)
	{
		if (*pv == 0)
		{ // Here, no cell readings likely
			pbq->battery_status |= BSTATUS_NOREADING;
		}
		if ((*pv < 0) || (*pv > 5000))
		{ // Here, zero or negative likely open wires
			pbq->battery_status |= BSTATUS_OPENWIRE;
		}
		if (*pv > pbq->cellv_high)
		{ // Find max cell reading
			pbq->cellv_high = *pv; // Save voltage
			pbq->cellx_high = i;   // Save cell index
		} 
		if (*pv < pbq->cellv_low)
		{ // Find lowest cell reading
			pbq->cellv_low = *pv; // Save voltage
			pbq->cellx_low = i;   // Save cell index
		} 
		*pv = *p;  // Copy for possible(!) use
		pbq->cellv_total += *pv; // Sum cell readings

		// Copy and initialize for cell balancing selection
		pbal->v = *p; pbal->i = i; pbal->s = 0;

		p += 1; pv += 1; pbal += 1; // Up array pointers
	}

	/* Unusual situation check. */
	if (((pbq->battery_status & BSTATUS_NOREADING) != 0) ||
	    ((pbq->battery_status & BSTATUS_OPENWIRE)  != 0) )
	{ // Here no charging, no discharging
		pbq->fet_status &= ~(FET_DUMP|FET_HEATER|FET_DUMP2|FET_CHGR);
		pbq->cellbal = 0; // All cell balancing FETS off
	}
	else
	{ // Here it looks like we have a normal situation with good readings
		/* Check out of limit voltages */
		if (pbq->cellv_high > pbq->lc.cellv_max) // Too high?
		{ // Here, one or more cells are over max limit
			// No charging, and discharging is needed
			pbq->fet_status &= ~(FET_CHGR|FET_DUMP2);
		}
		if (pbq->cellv_low < pbq->lc.cellv_min) // Too low?
		{ // Here, one or more cells are below min limit
			pbq->fet_status &= ~(FET_DUMP|FET_HEATER); // No discharge
			// The following assumes DUMP2 FET controls an external charger
			pbq->fet_status |= (FET_CHGR|FET_DUMP2); // Enable charging

			pbq->cellbal = 0; // All cell balancing FET bits off
		}
		else
		{ // Here, balancing is permitted. Make cell selections

			/* Build a 16b word for the BQ to set discharge FETs. */

			// Cells above 'tmp' are candidates for discharging
			int16_t tmp = pbq->cellv_low + pbq->lc.cellbal_del;

			pbal = &bqfunction.cellv_bal[0]; // Cell readings pointer

			// Limit the number of cells discharging at any given cycle
			pbq->active_ct = 0;    // Count number of cell bits set

			// Prevent adjacent cells from being turned ON
			uint8_t previous = 0;  // Adjacent active cell restriction

			// Sending this word to BQ will set active cells. 
			pbq->cellbal = 0; // Begin with no cells set

			// Note: this always gives cell indices 1 to balnumax first dibs
			for (i = 0; i < 16; i++)
			{
				if (pbal->v > tmp) // Need discharging?
				{ // Here, yes. Cell is candidate for discharge
					if (previous == 0)
					{ // Here, previous (adjacent) cell was NOT SET
						if (pbq->active_ct < pbq->balnumwrk)
						{ // Here number active cells not at limit
							pbq->cellbal |= (1 << i); // Set BQ word bit
							pbq->active_ct += 1;
							previous = 1;
						}
					}
					else
					{ // Previous (adjacent) cell was SET ON
						previous = 0;
					}
				}
				else
				{ // Here, this cell does not need discharging
					previous = 0;
				}
				pbal += 1;
			}
		}
	}
	return;
}
/* ************************************************************************************************************ 
 * Comparison function for qsort
 * ************************************************************************************************************ */
//#define USESORTCODE
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
 * @param   : p = pointer to 16 cell struct array
 * *************************************************************************/
void bq_items_qsortV(struct BQCELLV* p)
{
	/* Sort (ascending) on Cell voltages. */
	qsort(p, 16, sizeof(struct BQCELLV), compare_v);
	return;
}
#endif