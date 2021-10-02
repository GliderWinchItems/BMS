/******************************************************************************
* File Name          : bq_func_init.c
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Init function struc
*******************************************************************************/
#include "bq_func_init.h"


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

	p->chargeflag   = 0;  // 0 = No charging; not zero = charging
	p->dumpflag     = 0;  // 0 = Dump FET OFF; not zero = dump fet ON
	p->extchgrflag  = 0;  // 0 = Dump2 (external charger) OFF; not zero = ON
	p->tim1_ccr1    = 0;  // Charger FET ON time initially set for no charging.
	p->cellv_ok     = 0;  // Cell voltage readings (cellv_latest[16]) initially not valid

	p->state      = 0;  // main state
	p->substateA  = 0;  // 
	p->substateB  = 0;  // spare substate 

	for (i = 0; i < 16; i ++) p->cellv_latest[i] = 0;


		/* Pointers to incoming CAN msg mailboxes. */
//	struct MAILBOXCAN* pmbx_cid_gps_sync;        // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
//	struct MAILBOXCAN* pmbx_cid_drum_tst_stepcmd;// CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000


	return;
}