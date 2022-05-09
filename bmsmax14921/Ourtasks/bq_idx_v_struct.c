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
   p->winchnum     = 1; // Winch number (1 - 4)
   if ((p->winchnum == 0) || (p->winchnum > 4)) morse_trap(700);
   p->stringnum    = 1; // Battery string number (1 - 4)
   if ((p->stringnum == 0) || (p->stringnum > 4)) morse_trap(701);
   p->modulenum    = 2; // Module number on string (1-16)
   if ((p->modulenum == 0) || (p->modulenum > 16)) morse_trap(702);

   p->hbct_t       = 64;//1001;   // Heartbeat ct: milliseconds between sending 
   p->hbct         = 64;    // Number of swctr ticks between heartbeats

//   p->adc_hb       = 64;     // Number of ticks for heartbeat ADC readout

   p->CanComm_hb = 24; // CanCommTask 'wait' counts per heartbeat sending
 
            /* Initial test settings. */
   p->dac1_hv_setting  = 3500; // HV volt limit
   p->dac2_ix_setting  = 100;  // Current sense level setting
   p->tim1_ccr1_on     =  50;  // PWM ON count: Normal charge rate
   p->tim1_ccr1_on_vlc =   2;  // PWM ON count: Very Low Charge rate required
   p->tim1_arr_init    =  79;  // At 16 MHz: count of 80 = 5 us PWM frame

   p->cellv_max   = 3500; // Max limit (mv) for charging any cell
   p->cellv_min   = 2600; // Min limit (mv) for any discharging
   p->cellv_vlc   = 2550; // Below this (mv) Very Low Charge (vlc)required
   p->cellopenhi  = 4300; // Above this voltage cell wire is assumed open (mv)
   p->cellopenlo  =  333; // Below this voltage cell wire is assumed open (mv)
   p->modulev_max = (16*3600); // Battery module max limit (mv)
   p->modulev_min = (16*2600); // Battery module min limit (mv)

   p->balnummax   =  7;  // Max number of cells to discharge at one time
   p->cellbal_del =  2;  // Balance within lowest cellv + this delta (mv)

  /* Arrays have been compile using NCELLMAX [18] */
   p->ncell = 16; // Number of series cells in this module

   if (p->ncell > NCELLMAX) morse_trap(702); // Error trap as it needs fixing
   p->npositions  = 18;  // Number of cell =>positions<= in module "box"

   /* Relate cell numbers to cell positions. (indices are ("number"-1) */
#ifdef EighteenPositionBox

   /* 16 cells installed in 18 position box. */
   p->cellpos[ 0]  =  0; // Cell #1 installed in cell position #1
   p->cellpos[ 1]  =  1; // ...
   p->cellpos[ 2]  =  2; // 
   p->cellpos[ 3]  =  3; // 
   p->cellpos[ 4]  =  4; // 
   p->cellpos[ 5]  =  5; // 
   p->cellpos[ 6]  =  6; // 
   p->cellpos[ 7]  =  7; // Cell #8 is installed in cell position #8
   p->cellpos[ 8]  =  CELLNONE; // No cell in cell position 8
   p->cellpos[ 9]  =  CELLNONE; // No cell in cell position 9
   p->cellpos[10]  =  8; // Cell #9 is installed in cell position 10
   p->cellpos[11]  =  9; // ...
   p->cellpos[12]  = 10; 
   p->cellpos[13]  = 11; 
   p->cellpos[14]  = 12; 
   p->cellpos[15]  = 13; 
   p->cellpos[16]  = 14; 
   p->cellpos[17]  = 15; // Cell #16 is installed in cell position #18

#else

     /* 16 cells installed in 16 position box, or 18 position box wired as such. */
   p->cellpos[ 0]  =  0; // Cell #1 installed in cell position #1
   p->cellpos[ 1]  =  1; // ...
   p->cellpos[ 2]  =  2; // 
   p->cellpos[ 3]  =  3; // 
   p->cellpos[ 4]  =  4; // 
   p->cellpos[ 5]  =  5; // 
   p->cellpos[ 6]  =  6; // 
   p->cellpos[ 7]  =  7; //
   p->cellpos[ 8]  =  8; // 
   p->cellpos[ 9]  =  9; // 
   p->cellpos[10]  = 10; // 
   p->cellpos[11]  = 11; // 
   p->cellpos[12]  = 12; //
   p->cellpos[13]  = 13; //
   p->cellpos[14]  = 14; //
   p->cellpos[15]  = 15; // Cell #16 installed in cell position #16
   p->cellpos[16]  = CELLNONE; // No cell
   p->cellpos[17]  = CELLNONE; // No cell

#endif

// List of CAN ID's for setting up hw filter for incoming msgs
   //                      CANID_HEX      CANID_NAME             CAN_MSG_FMT     DESCRIPTION
   	// We receive: EMC
   p->cid_cmd_bms_cellvq = CANID_CMD_BMS_CELLVQ; // B0200000 
   p->cid_cmd_bms_miscq  = CANID_CMD_BMS_MISCQ;  // B0400000 request reading for many options
   p->cid_unit_bms01     = CANID_UNIT_BMS01;     // B0600000 universal, multi-purpose BQ76952  #01
   p->cid_uni_bms_i      = CANID_UNI_BMS_I;      // B0000000 UNIversal BMS Incoming msg to BMS: X4=target CANID

// CAN ids BMS sends, others receive
   p->cid_msg_bms_cellvsmr = CANID_MSG_BMS_CELLV12R; //'B0201124', U16_U16_U16_U16'
	return;
}
