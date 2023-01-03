/******************************************************************************
* File Name          : bq_idx_v_struct.c
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include "bq_idx_v_struct.h"
#include "SerialTaskReceive.h"
#include "morse.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* *************************************************************************
 * void bq_idx_v_struct_hardcode_params(truct BQLC* p);
 * @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
void bq_idx_v_struct_hardcode_params(struct BQLC* p)
{
	p->size    = 32;
	p->crc     = 0;   // TBD
   p->version = 1;   // 

	/* Timings in milliseconds. Converted later to timer ticks. */


   /* Identification of this module node. */
   p->winchnum    = 1; // Winch number (1 - 4)
   if ((p->winchnum == 0) || (p->winchnum > 4)) morse_trap(700);
   p->stringnum    = 1; // Battery string number (1 - 4)
   if ((p->stringnum == 0) || (p->stringnum > 4)) morse_trap(701);
   p->modulenum    = 1; // Module number on string (1-16)
   if ((p->modulenum == 0) || (p->modulenum > 16)) morse_trap(702);

   p->hbct_t       = 3800;   // Heartbeat ct: milliseconds between sending 
   p->hbct         = 64;    // Number of swctr ticks between heartbeats

   /* Arrays have been compile using NCELLMAX [18] */
   p->ncell = 16; // Number of series cells in this module
   if (p->ncell > NCELLMAX) morse_trap(702); // Error trap as it needs fixing

   p->dac1_hv_setting  = 2900; // 65.2 volt limit
   p->dac2_ix_setting  =   101; // Current sense level setting
   p->tim1_ccr1_on     =   50; // PWM ON count: Normal charge rate
   p->tim1_ccr1_on_vlc =    2; // PWM ON count: Very Low Charge rate required
   p->tim1_arr_init    =   79; // At 16 MHz: count of 80 = 5 us PWM frame

   p->cellv_max   = 4050;   // Max limit (mv) for charging any cell
   p->cellv_min   = 2900;   // Min limit (mv) for any discharging
   p->cellv_vlc   = 2200;   // Below this (mv) Very Low Charge (vlc)required
   p->modulev_max = (16*4050); // Battery module max limit (mv)
   p->modulev_min = (16*2900); // Battery module min limit (mv)

   p->balnummax   =  7;  // Max number of cells to discharge at one time
   p->cellbal_del =  2;  // Balance within lowest cellv + this delta (mv)

   /* Convert ADC counts to cell voltage */
   p->cellcal[ 0] =  7.2233121E-07    ;
   p->cellcal[ 1] =  7.2234845E-07    ;
   p->cellcal[ 2] =  7.2234573E-07    ;
   p->cellcal[ 3] =  7.2233483E-07    ;
   p->cellcal[ 4] =  7.2235134E-07    ;
   p->cellcal[ 5] =  7.2237960E-07    ;
   p->cellcal[ 6] =  7.2238600E-07    ;
   p->cellcal[ 7] =  7.2238046E-07    ;
   p->cellcal[ 8] =  7.2240982E-07    ;
   p->cellcal[ 9] =  7.2242603E-07    ;
   p->cellcal[10] =  7.2256517E-07    ;
   p->cellcal[11] =  7.2248220E-07    ;
   p->cellcal[12] =  7.2252472E-07    ;
   p->cellcal[13] =  7.2261882E-07    ;
   p->cellcal[14] =  7.2265346E-07    ;
   p->cellcal[15] =  7.2272802E-07    ;
   p->cellcal[16] =  7.2247370E-04    ;
   p->cellcal[17] =  7.2247370E-04    ;

   /* Rescale */
   uint8_t i;
   for (i = 0; i < NCELLMAX; i++)
      p->cellcal[i] *= 10000.0f;


 
// List of CAN ID's for setting up hw filter for incoming msgs
   //                      CANID_HEX      CANID_NAME             CAN_MSG_FMT     DESCRIPTION
   	// We receive: EMC
   p->cid_cmd_bms_cellvq = CANID_CMD_BMS_CELLVQ; // B0200000 
   p->cid_cmd_bms_miscq  = CANID_CMD_BMS_MISCQ;  // B0400000 request reading for many options
   p->cid_unit_bms01     = CANID_UNIT_BMS01;     // B0600000 universal, multi-purpose BQ76952  #01
   p->cid_uni_bms_emc_i  = CANID_UNI_BMS_EMC_I;  //B0000000 multi-purpose universal BMS
   p->cid_uni_bms_pc_i   = CANID_UNI_BMS_PC_I;   //B0200000 multi-purpose universal BMS

// CAN ids BMS sends, others receive
   p->cid_msg_bms_cellvsmr = CANID_MSG_BMS_CELLV11R; //'B0201114', U16_U16_U16_U16'
	return;
}
