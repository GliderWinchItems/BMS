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

   /* Identification of this module node. */
   uint8_t  winchnum;  // Winch number (1-4)
   uint8_t  stringnum; // Battery string number (1 - 4)
   uint8_t  modulenum; // Module number on string (1-16)


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

   float cellcal[NCELLMAX]; // cell[n] voltage = ADC count * cellcal[n] (100 uv

 // CAN ids ...........................................................................
   //                                  CANID_NAME            CANID       PAYLOAD FORMAT
    // BMS sends; EMC receives
   uint32_t cid_msg_bms_cellvsmr; // CANID_MSG_BMS_CELLV11R','B0201114', 'U8_U8_U16_U16_U16'
   uint32_t cid_cmd_bms_misc11r;  // CANID_CMD_BMS_MISC11R' ,'B0401114', 'U8_U8_U8_X4'
 

 // List of CAN ID's for setting up hw filter for incoming msgs
   // EMC sends; BMS receives
   uint32_t cid_cmd_bms_cellvq; // CANID_CMD_BMS_CELLVQ B0200000 request cell voltages
   uint32_t cid_cmd_bms_miscq;  // CANID_CMD_BMS_MISCQ  B0400000 request reading for many options
   uint32_t cid_unit_bms01;     // CANID_UNIT_BMS01     B0600000 multi-purpose BQ76952  #01
// Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds;
   uint32_t cid_uni_bms_emc_i;  // CANID_UNI_BMS_EMC_I  B0000000 multi-purpose universal BMS
   uint32_t cid_uni_bms_pc_i;   // CANID_UNI_BMS_PC_I   B0200000 multi-purpose universal BMS
 };

/* *************************************************************************/
void bq_idx_v_struct_hardcode_params(struct BQLC* p);
/* @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
 
#endif

