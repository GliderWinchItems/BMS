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

#define NCELLMAX 18  // Largest array size allowed

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

   uint16_t dac1_hv_setting;  // 65.2 volt limit
   uint16_t dac2_ix_setting;  //62;   // Current sense level setting
   uint16_t tim1_ccr1_on;     // PWM ON count: Normal charge rate
   uint16_t tim1_ccr1_on_vlc; // PWM ON count: Very Low Charge rate required
   uint16_t tim1_arr_init;    // Initial ARR (PWM frame) count - 1

   /* The following are measured at no-charging, no-load */
   int16_t cellv_max;   // Max limit for charging any cell
   int16_t cellv_min;   // Min limit for any discharging
   int16_t cellv_vlc;   // Below this Very Low Charge (_vlc)required
   uint32_t modulev_max; // Battery module max limit
   uint32_t modulev_min; // Battery module min limit

   uint8_t balnummax;    // Max number of cells to discharge at one time
   int16_t cellbal_del;  // Balance within lowest cellv + this delta (mv)

   uint8_t ncell;       // Number of series cells in module

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
