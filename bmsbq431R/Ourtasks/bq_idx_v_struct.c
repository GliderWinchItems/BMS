/******************************************************************************
* File Name          : bq_idx_v_struct.c
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include "bq_idx_v_struct.h"
#include "SerialTaskReceive.h"
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

/* GevcuTask counts 'sw1timer' ticks for various timeouts.
 We want the duration long, but good enough resolution(!)
 With systick at 512/sec, specifying 8 ms yields a 4 tick duration
 count = 4 -> 64/sec (if we want to approximate the logging rate)
 count = 64 -> 1/sec 
*/ 
	//    my drum should be associated with whole node and not just the
   //    level-wind function. need to figure out where the parameters for 
   //    the whole node should be placed       
   p->modulenumber = 1;     // drum this node is assigned to     
   p->hbct_t       = 500;   // Heartbeat ct: milliseconds between sending 
   p->hbct         = 64;    // Number of swctr ticks between heartbeats

   p->dac1_hv_setting = 2900; // 65.2 volt limit
   p->dac2_ix_setting = 116;  //62;   // Current sense level setting
   p->tim1_ccr1_ON    = 50;   // PWM ON count.
   p->tim1_arr_init   = 79;   // At 16 MHz: count of 80 = 5 us PWM frame
 

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
