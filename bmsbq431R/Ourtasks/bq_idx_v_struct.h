/******************************************************************************
* File Name          : bq_idx_v_struct.h
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"

#ifndef __BQ_IDX_V_STRUCT
#define __BQ_IDX_V_STRUCT


/* Parameters levelwind instance (LC = Local Copy) */
struct BQLC
 {
/* NOTE: all suffix _t parameters are times in milliseconds */

	uint32_t size;
	uint32_t crc;   // TBD
   uint32_t version;   //

   uint8_t  modulenumber;

	/* Timings in milliseconds. Converted later to 'swtim1' ticks. */
	uint32_t hbct_t;     // Heartbeat ct: ms between sending 
   uint32_t hbct;       // Number of ticks between hb msgs

   uint16_t dac1_hv_setting; // 65.2 volt limit
   uint16_t dac2_ix_setting; //62;   // Current sense level setting
   uint16_t tim1_ccr1_ON; // PWM ON count.
   uint16_t tim1_arr_init;   // Initial ARR (PWM frame) count - 1

 // CAN ids ...........................................................................
   //                                  CANID_NAME             CAN_MSG_FMT     DESCRIPTION
    // Levelwind sends; PC receives
//   uint32_t cid_hb_levelwind;        // CANID_HB_LEVELWIND: U8_U32','LEVELWIND: U8: Status, U32: stepper position accum


 // List of CAN ID's for setting up hw filter for incoming msgs
	// stepper test repo sends: drum receives
//	uint32_t cid_drum_tst_stepcmd; // CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000
   //uint32_t cid_mc_state; //'CANID_MC_STATE','26000000', 'MC', 'UNDEF','MC: Launch state msg');

 };

/* *************************************************************************/
void bq_idx_v_struct_hardcode_params(struct BQLC* p);
/* @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
 
#endif

