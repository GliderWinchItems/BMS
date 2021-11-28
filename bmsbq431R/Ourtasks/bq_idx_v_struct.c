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

   p->hbct_t       = 200;   // Heartbeat ct: milliseconds between sending 
   p->hbct         = 64;    // Number of swctr ticks between heartbeats

   /* Arrays have been compile using NCELLMAX [18] */
   p->ncell = 16; // Number of series cells in this module
   if (p->ncell > NCELLMAX) morse_trap(702); // Needs recompiling

   p->dac1_hv_setting  = 2900; // 65.2 volt limit
   p->dac2_ix_setting  =  116; // Current sense level setting
   p->tim1_ccr1_on     =   50; // PWM ON count: Normal charge rate
   p->tim1_ccr1_on_vlc =    2; // PWM ON count: Very Low Charge rate required
   p->tim1_arr_init    =   79; // At 16 MHz: count of 80 = 5 us PWM frame

   p->cellv_max   = 4001;   // Max limit (mv) for charging any cell
   p->cellv_min   = 3350;   // Min limit (mv) for any discharging
   p->cellv_vlc   = 2900;   // Below this (mv) Very Low Charge (vlc)required
   p->modulev_max = (16*4150); // Battery module max limit (mv)
   p->modulev_min = (16*3450); // Battery module min limit (mv)

   p->balnummax   =  7;  // Max number of cells to discharge at one time
   p->cellbal_del =  2;  // Balance within lowest cellv + this delta (mv)
 
// CAN ids levelwind sends
   //                      CANID_HEX      CANID_NAME             CAN_MSG_FMT     DESCRIPTION
   // Others receive
//   p->cid_hb_levelwind  = 0xE4A00000;   // CANID_HB_LEVELWIND: U8_U32, Heartbeat Status, levelwind position accum');

// List of CAN ID's for setting up hw filter for incoming msgs
   	// We receive: EMC
   p->cid_cmd_bms_cellvq = CANID_CMD_BMS_CELLVQ; // B0200000 
   p->cid_cmd_bms_miscq  = CANID_CMD_BMS_MISCQ;  // B0400000 request reading for many options

	return;
}
