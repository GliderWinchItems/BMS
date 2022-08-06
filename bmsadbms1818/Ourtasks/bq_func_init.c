/******************************************************************************
* File Name          : bq_func_init.c
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Init function struc
*******************************************************************************/
/*
06/27/2022 updated for ADBMS1818
*/
#include "bq_func_init.h"
#include "CanCommTask.h"
#include "iir_f1.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* *************************************************************************
 * void bq_func_init(void);
 * @brief	: Init struct with working parameters
 * *************************************************************************/
void bq_func_init(void)
{
	struct BQFUNCTION* p = &bqfunction;
	int i;

	/* Get a copy of fixed parameters. */
	bq_idx_v_struct_hardcode_params(&p->lc);

	/* Hearbeat timing. */
	p->hbct_k =  pdMS_TO_TICKS(p->lc.hbct_t); // Convert ms to RTOS ticks

	// Packed: string and module numbers
	p->ident_onlyus = ((p->lc.stringnum-1) << 4) | ((p->lc.modulenum-1) << 0); 
	p->ident_string = ((p->lc.stringnum-1) << 4); 

	/*  payload [0-1] U16 – Payload Identification
	[15:14] Winch (0 - 3)(winch #1 - #4)
  	[13:12] Battery string (0 – 3) (string #1 - #4)
  	[11:8] Module (0 – 15) (module #1 - #16)
  	[7:3] Cell (0 - 31) (cell #1 - #32)
  	[2:0] Group sequence number (0 - 7) */
	p->cellvpayident = (((p->lc.winchnum -1) << 14) |
	                    ((p->lc.stringnum-1) << 12) |
					    ((p->lc.modulenum-1) <<  8) );

	p->tim1_ccr1    = 0;  // Charger FET ON time initially set for no charging.

	p->cellbal        = 0; // Bits to activate cell balance fets
	p->battery_status = 0; // Cell status Bits 
	p->fet_status     = 0; // FET bits
    p->hyster_sw      = 0; // 1 = means hysteresis (relaxation) currently in effect
    p->hysterbits_hi  = 0; // Reset bits: 1 = cell reached max while hyster_sw = 0;

	p->state      = 0;  // main state
	p->substateA  = 0;  // 
	p->substateB  = 0;  // spare substate 

	// Cell voltage hysteresis (relaxation)
    p->hysterbits_lo = 0; // Cell bit ON: voltage less than hysteresis low
    p->hysterbits_hi = 0; // Cell bit ON: voltage greater than max target
    p->hysterv_lo = (p->lc.cellv_max - p->lc.cellv_hyster); // hyster volt low

	for (i = 0; i < NCELLMAX; i ++)
	{
		p->cellv_latest[i] = 0;
	}

	p->balnumwrk = p->lc.balnummax; // Working number of active cell balancing bits

	/* Filter raw readings for calibration purposes. */
	for (i = 0; i < ADCBMSMAX; i ++)
	{
		p->filtiirf1_raw[i].coef    = RAWTC;
		p->filtiirf1_raw[i].onemcoef = 1.0 - p->filtiirf1_raw[i].coef;  // 1 - coef 
		p->filtiirf1_raw[i].skipctr = RAWSKIPCT;
	}

	/* Build a word with bits showing installed cell positions. */
	p->cellspresent = 0;
	for ( i = 0; i < NCELLMAX; i++)
	{ // Skip predetermined empty box positions
		if (p->lc.cellpos[i] != CELLNONE)
		{
			p->cellspresent |= (1<<i);
		}
	}

	/* Fan control. */
	p->fanspeed = 0; // Fan speed: rpm pct 0 - 100
	
	return;
}