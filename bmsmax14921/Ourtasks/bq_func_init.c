/******************************************************************************
* File Name          : bq_func_init.c
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Init function struc
*******************************************************************************/
#include "bq_func_init.h"
#include "CanCommTask.h"
#include "iir_f1.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* *************************************************************************
 * void bq_func_init(struct BQFUNCTION* p);
 * @brief	: Init struct with working parameters
 * @param   : p = pointer to struct will all parameters for BQ function
 * *************************************************************************/
void bq_func_init(struct BQFUNCTION* p)
{
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

	p->state      = 0;  // main state
	p->substateA  = 0;  // 
	p->substateB  = 0;  // spare substate 

	for (i = 0; i < 16; i ++)
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

	return;
}