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

	//    my drum should be associated with whole node and not just the
   //    level-wind function. need to figure out where the parameters for 
   //    the whole node should be placed       
   p->modulenumber = 1;     // drum this node is assigned to     
   p->hbct_t       = 500;   // Heartbeat ct: milliseconds between sending 
   p->hbct         = 64;    // Number of swctr ticks between heartbeats

   /* Arrays have been compile using NCELLMAX [18] */
   p->ncell = 16; // Number of series cells in this module
   if (p->ncell > NCELLMAX) morse_trap(8555); // Needs recompiling

   p->dac1_hv_setting  = 2900; // 65.2 volt limit
   p->dac2_ix_setting  =  116; // Current sense level setting
   p->tim1_ccr1_on     =   50; // PWM ON count: Normal charge rate
   p->tim1_ccr1_on_vlc =    2; // PWM ON count: Very Low Charge rate required
   p->tim1_arr_init    =   79; // At 16 MHz: count of 80 = 5 us PWM frame

   p->cellv_max   = 4000;   // Max limit (mv) for charging any cell
   p->cellv_min   = 3350;   // Min limit (mv) for any discharging
   p->cellv_vlc   = 2900;   // Below this (mv) Very Low Charge (vlc)required
   p->modulev_max = (16*3400); // Battery module max limit (mv)
   p->modulev_min = (16*4150); // Battery module min limit (mv)

   p->balnummax   =  6;  // Max number of cells to discharge at one time
   p->cellbal_del = 10;  // Balance within lowest cellv + this delta (mv)
 
// CAN ids levelwind sends
   //                      CANID_HEX      CANID_NAME             CAN_MSG_FMT     DESCRIPTION
   // Others receive
//   p->cid_hb_levelwind  = 0xE4A00000;   // CANID_HB_LEVELWIND: U8_U32, Heartbeat Status, levelwind position accum');

// List of CAN ID's for setting up hw filter for incoming msgs
   	// We receive: Logger/gps 
//	p->cid_gps_sync     = 0x00400000; // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
	// We receive stepper repo: update100K sends
//	p->cid_drum_tst_stepcmd	=  CANID_TST_STEPCMD; //0xE4600000; // CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position:
//
	return;
}
