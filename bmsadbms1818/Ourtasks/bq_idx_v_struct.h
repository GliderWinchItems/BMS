/******************************************************************************
* File Name          : bq_idx_v_struct.h
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Fixed parameter struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"
//#include "iir_filter_lx.h"
#include "iir_f1.h"

#ifndef __BQ_IDX_V_STRUCT
#define __BQ_IDX_V_STRUCT

#define NCELLMAX 18   // Largest array size allowed
#define NAUXMAX  12   // Auxiliary array size
#define CELLNONE 0xFF // Code for no cell in cell box position

/* 16b codes for impossible voltage readings. */
#define CELLVBAD   65535 // Yet to be defined
#define CELLVNONE  65534 // Cell position not installed
#define CELLVOPEN  65533 // Installed, but wire appears open

#define RAWTC 0.975f  // Filter time constant
#define RAWSKIPCT 2  // Ignore initial readings to filter


/* ADBMS1818 GPIO AUX Register ADC reading sequence/array indices */
#define BMSAUX_1_NC           0 // GPIO 1 No Connection
#define BMSAUX_2_THERM1       1 // GPIO 2 Thermistor 1
#define BMSAUX_3_THERM3       2 // GPIO 3 Thermistor 3
#define BMSAUX_4_THERM2       3 // GPIO 4 Thermistor 2
#define BMSAUX_5_US5          4 // GPIO 5 Spare
#define BMSAUX_REF            5 // 2nd Reference voltage
#define BMSAUX_6_CUR_SENSE    6 // GPIO 6 Current sense op amp
#define BMSAUX_7_HALL         7 // GPIO 7 Hall effect sensor signal
#define BMSAUX_8_US9          8 // GPIO 8 Spare: U$9
#define BMSAUX_9_US10         9 // GPIO 9 Spare: U$10
#define BMSAUX_RSVD          10 // RSVD1
#define BMSAUX_COV_CUV       11 // COV CUV RSVD1

struct BMSCAL
{
   struct FILTERIIRF1 iir_f1; // Filter: Time constant, integer scaling
   float coef[3]; // coefficients for: x^0 x^1 x^2
   float  f;
};

struct BMSCALTEMP // Thermistor->temperature calibration
{
   float tt[3]; // coefficients for: x^0 x^1 x^2
   float temp;  // Temperature (deg C)
   uint8_t installed; // Installed: 0 = no, 1 = yes
   uint8_t zread; // 1 = current reading was zero
};


/* Parameters levelwind instance (LC = Local Copy) */
struct BQLC
 {
/* NOTE: all suffix _t parameters are times in milliseconds */
   struct BMSCAL bmscal[NCELLMAX];
   struct BMSCAL bmsaux[NAUXMAX];

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

   uint32_t CanComm_hb; // CanCommTask 'wait' RTOS ticks per heartbeat sending

   float dcdc_v; // Isolated DC-DC converter output voltage (e.g. 15.0v)
   float dcdc_w; // Charger power max taken from DC-DC converter (e.g. 5.5W)
   float dcdc_calv; // Module voltage used in following settings (e.g. 57.6v)
   uint16_t dac1_hv_setting;  // DAC setting for 65.2 volt limit
   uint16_t dac2_ix_setting;  // DAC setting for 62 ma current sense level setting
   uint16_t tim1_ccr1_on;     // PWM ON count: Normal charge rate
   uint16_t tim1_ccr1_on_vlc; // PWM ON count: Very Low Charge rate required
   uint16_t tim1_arr_init;    // Initial ARR (PWM frame) count - 1

   uint32_t modulev_max; // Battery module max limit
   uint32_t modulev_min; // Battery module min limit
   uint16_t cellopen_lo; // Below this cell volatge wire is assumed open (mv)
   uint16_t cellopen_hi; // Above this cell volatge wire is assumed open (mv)
   uint16_t cellv_tgtdelta; // Target delta: cell max - this delta (mv)
   int16_t cellv_max;    // Max limit for charging any cell (mv)
   int16_t cellv_min;    // Min limit for any discharging (mv)
   int16_t cellv_vlc;    // Below this Very Low Charge (_vlc)required

   uint16_t maxchrgcurrent; // Maximum charge current (0.1a)


   int16_t cellv_hyster; // Relax-to-voltage = (cellv_max - cellv_hyster)
   uint32_t trip_max;   // Cells that have tripped max voltage when hyster_sw = 0;

   int16_t cellbal_del; // Balance within this amount of lowest
 

   uint8_t balnummax;    // Max number of cells to discharge at one time

   uint8_t ncell;       // Number of series cells in module
   uint8_t npositions;  // Number of cell positions in module "box"
   uint8_t cellpos[NCELLMAX]; // Cell position - 1 given index [cell number - 1]

   struct BMSCALTEMP thermcal[3]; // Thermisters: calibration & results
   // Below min fan is off
   float  temp_fan_min; // Between max & min fan proporational
   float  temp_fan_max; // Above max fan is 100%

 // CAN ids ...........................................................................
   //                                  CANID_NAME            CANID       PAYLOAD FORMAT
    // BMS sends; EMC, PC or other receives
   uint32_t cid_msg_bms_cellvsmr; // CAN id for cell voltage burst format 
   uint32_t cid_cmd_bms_miscr;    // CAN id for all other responses

 // List of CAN ID's for setting up hw filter for incoming msgs
   uint32_t cid_uni_bms_emc1_i;  // CANID_UNI_BMS_EMC1_I      B0000000 multi-purpose universal BMS
   uint32_t cid_uni_bms_emc2_i;  // CANID_UNI_BMS_EMC2_I      B0200000 multi-purpose universal BMS
   uint32_t cid_uni_bms_pc_i;    // CANID_UNI_BMS_PC_I        AEC00000 multi-purpose universal BMS

   uint8_t hbseq; // heartbeat CAN msg sequence number
 };

/* *************************************************************************/
void bq_idx_v_struct_hardcode_params(struct BQLC* p);
/* @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
 
#endif

