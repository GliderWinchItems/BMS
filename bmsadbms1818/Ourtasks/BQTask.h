/******************************************************************************
* File Name          : BQTask.h
* Date First Issued  : 09/08/2021
* Description        : BQ76952 BMS w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __BQTASK
#define __BQTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "bq_idx_v_struct.h"
#include "CanTask.h"
#include "adc_idx_v_struct.h"

//#define USESORTCODE

#define BQVSIZE 20 // Readout loop size (16 cells plus others)
/*
#define NUMCANMSGS 8 // Number of predefined CAN msgs 
#define MAXNUMCELLMSGS 6 // Max number of CAN msgs with cell readings
#define CID_MSG_CELLV01  0 // CAN msg with cell readings 1 2 3
#define CID_MSG_CELLV02  1 // CAN msg with cell readings 4 5 6
#define CID_MSG_CELLV03  2 // CAN msg with cell readings 7 8 9
#define CID_MSG_CELLV04  3 // CAN msg with cell readings 10 11 12
#define CID_MSG_CELLV05  4 // CAN msg with cell readings 13 14 15
#define CID_MSG_CELLV06  5 // CAN msg with cell readings 16 or 16 17 18
#define CID_CMD_MISC     6 // CAN msg with status and TBD stuff
#define CID_CMD_UNI      7 // CAN msg with universal command response
*/

/* Battery status bits: 'battery_status' */
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 4.3v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Low power charger ON | DUMP2 ON
#define BSTATUS_DUMPTOV   (1 << 6)  // Discharge to a voltage in progress
#define BSTATUS_CELLVRYLO (1 << 7)  // One or more cells very low

/* FET status bits" 'fet_status' */
#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

/* Mode status bits 'mode_status' */
#define MODE_SELFDCHG  (1 << 0) // 1 = Self discharge; 0 = charging
#define MODE_CELLTRIP  (1 << 1) // 1 = One or more cells tripped max
#define MODE_TRIPBTD   (1 << 2) // 1 = One or more cells tripped & below target-delta

/* Indices for CAN msg commands. */
#define REQ_HEATER 0
#define REQ_DUMP   1
#define REQ_DUMP2  2

/* CAN msgs are scaled to 100uv (see ADBMS1818 datasheet)
  BQ76952 can measure negative voltages, so these are coded into the upper values
  that are not possible. */
//#define CELLV_OPEN  65534  // cellv_latest array, uint16_t value for open wire
//#define CELLV_MINUS 65535  // cellv_latest array, uint16_t value for negative voltage

/* Cell current voltage for last measurement. */
struct VI
{
	uint32_t v;
	uint32_t i;
};
union ADCCELLCOUNTS
{
	struct VI vi[16];
// [x][0] = current, [x][1] = voltage, x = cell
	uint32_t blk[16][2]; // Cells 1-16
};

struct BQPN
{
	uint8_t* p;
	uint8_t  n;
};

struct BQCELLV
{
	int16_t v; // Cell voltage reading (mv)
	uint8_t idx; // Cell array index (0 - 15)
	uint8_t s; // Cell selection TBD (maybe not used)
};

struct BQREQ // CAN msg requests
{
	uint32_t tim;  // RTOS timer timeout 
	uint8_t  on;   // 1 = req ON; 0 = req OFF
	uint8_t  req;  // 1 = request in progress; 0 = no request
};

/* Working struct for BQ function. */
struct BQFUNCTION
{
   // Parameter loaded either by high-flash copy, or hard-coded subroutine
	struct BQLC lc; // Fixed parameters, (lc = Local Copy)

//	struct ADCFUNCTION* padc; // Pointer to ADC working struct

	/* Timings in milliseconds. Converted later to timer ticks. */
	uint32_t hbct_k;      // Heartbeat ct: ticks between sending

	uint8_t ident_string; // Packed: string
	uint8_t ident_onlyus; // Packed: string and module numbers
	/*  payload [0-1] U16 – Payload Identification
  [15:14] Winch (0 - 3)(winch #1 - #4)
  [13:12] Battery string (0 – 3) (string #1 - #4)
  [11:8] Module (0 – 15) (module #1 - #16)
  [7:3] Cell (0 - 31) (cell #1 - #32)
  [2:0] Group sequence number (0 - 7) */
	uint16_t cellvpayident; // Payload[0] identification for cell readings

//	uint8_t chargeflag;  // 0 = No charging; not zero = charging
//	uint8_t dumpflag;    // 0 = Dump FET OFF; not zero = dump fet ON
//	uint8_t extchgrflag; // 0 = Dump2 (external charger) OFF; not zero = ON
//	uint8_t cellv_ok;    // 0 = voltage readings not valid; not zero = OK

	uint16_t tim1_ccr1;  // Present CCR1 (PWM count)

	uint16_t cellv_latest[NCELLMAX]; // Cell voltage readings (0.1 millivolts)
	uint32_t cellvopenbits;// Bits for unexpected open cells (1 = open wire suspected) 
	int32_t  cellv_total;  // Sum of cell voltages (0.1 millivolts)
	int32_t  cellv_high;   // Highest cellv 0.1 millivolts
	int32_t  cellv_low;    // Lowest  cellv 0.1 millivolts
	int32_t  cellv_tmdelta;// Target minus target delta
	uint8_t  cellx_high;   // Highest cellv index (0-17)
	uint8_t  cellx_low;    // Lowest  cellv index (0-17)

	uint32_t cellv_max_bits; // Cells above cellv_max
	uint32_t cellv_min_bits; // Cells below cellv_min
	uint32_t cellv_vlc_bits; // Cells below cellv_vlc

	float cellv[NCELLMAX]; // Cell voltage (calibrated volts)
	float cellv_sort[NCELLMAX]; // cellv sorted
	float cellv_high_f;
	float cellv_low_f;
	float cellv_sum_f;
	float current;   // Op-amp sensing: amps current
	float temperature[3]; // Thermistors: Deg C temperature

	// Cell balancing & relaxation/self-discharge hysteresis
//	uint32_t targetv;       // Balance voltage target
	float    hysterv_lo;    // Hysteresis bottom voltage.
	uint32_t hysterbits_lo; // Bits for cells that fell below hysterv_lo
	uint32_t hysterbits_lo_save;
	uint8_t  hyster_sw;     // Hysteresis switch: 1 = peak was reached
	uint8_t  discharge_test_sw; // sw = 1, heater load on when hyster_sw on.
	uint8_t  hyster_sw_trip; // Set by CAN msg to trip hyster_sw

	// Adjustment to hold charger power taken from DC-DC converter constant
	uint8_t dcdc_oto;   // One-time-only calibration sw
	float dcdc_adjnom;  // Calibration adjustment factor
	float dcdc_adjfac;  // Cal adjustment factor * current factor 
	uint16_t tim1_arr_work; // Working ARR (PWM frame) count - 1

	/* Filter raw readings for calibration purposes. */
	struct FILTERIIRF1 filtiirf1_raw[ADCBMSMAX]; // Filter parameters
	float raw_filt[ADCBMSMAX]; // Filtered output
	float cal_filt[ADCBMSMAX]; // Filtered and calibrated

	uint32_t cellbal;       // Bits to activate cell balance fets!!!!!!!!!!!
	uint32_t celltrip;      // Bits for cell going over cellv_max
	uint32_t cell_tdt_bits; // Cells below Target-Delta & tripped
	uint32_t cellspresent;  // Bits for cell positions that are installed
	uint32_t cansetfet;     // Bits from CAN msg command sets FETs on|off
	uint32_t cansetfet_tim; // RTOS ticks for timeout of cansetfet
	uint8_t active_ct;      // Count of bits set in cellbal
	uint8_t battery_status; // Cell status code bits 
	uint8_t fet_status;     // This controls on/off of FETs
	uint8_t mode_status;       // Mode bits

	/* CAN msgs command to turn on HEATER, DUMP, DUMP2. 
	   request has timeout associated with it. */
	struct BQREQ bqreq[3];

	uint8_t fanspeed; // Fan speed (timer pwm setting): 14 = minimum for rotation; 100+ = full speed
	float   fanrpm;   // Fan speed (rpm)
	float   fanspeedscale; // Scaling factor: (100.0/(Tcell - Tamb))
	uint8_t  temp_fan_state; // Temperature fan state-machine

	uint32_t morse_err; // Error code retrieved from backup SRAM registers
	uint8_t err;
	uint16_t warning;   // Error code that is a warning

	uint8_t hbseq; // heartbeat CAN msg sequence number
	uint32_t HBstatus_ctr; // Count RTOS ticks for hearbeat timing: status msg
	uint32_t HBcellv_ctr; // Count RTOS ticks for hearbeat timing: cellv msg

	/* balnumwrk might be adjusted based on chip temperature. */
	uint8_t balnumwrk; // Max number of active cell bits (Working)

	/* Pointers to incoming CAN msg mailboxes. */
	struct MAILBOXCAN* pmbx_cid_cmd_bms_cellvq_emc;// CANID_CMD_BMS_CELLVQ: BMSV1 U8: EMC requests to BMS to send cellv, cmd code
	struct MAILBOXCAN* pmbx_cid_cmd_bms_miscq_emc; // CANID_CMD_BMS_MISCQ: BMSV1 U8: EMC requests to BMS to value for given cmd code
	struct MAILBOXCAN* pmbx_cid_uni_bms_emc1_i;     // CANID_UNI_BMS_I  B0000000 UNIversal BMS Incoming msg to BMS: X4=target CANID
	struct MAILBOXCAN* pmbx_cid_uni_bms_emc2_i;     // CANID_UNI_BMS_I  B0200000 UNIversal BMS Incoming msg to BMS: X4=target CANID

	struct MAILBOXCAN* pmbx_cid_cmd_bms_cellvq_pc;// CANID_CMD_BMS_CELLVQ: BMSV1 U8: EMC requests to BMS to send cellv, cmd code
	struct MAILBOXCAN* pmbx_cid_cmd_bms_miscq_pc; // CANID_CMD_BMS_MISCQ: BMSV1 U8: EMC requests to BMS to value for given cmd code
	struct MAILBOXCAN* pmbx_cid_uni_bms_pc_i;     // CANID_UNI_BMS_I  AEC00000 UNIversal BMS Incoming msg to BMS: X4=target CANID

	uint8_t state;      // main state
	uint8_t substateA;  // 
	uint8_t substateB;  // spare substate 

	/* CAN msgs */
	struct CANTXQMSG canmsg;

	uint32_t pad[4]; // Padding for debugging tests
};
/* *************************************************************************/
TaskHandle_t xBQTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BQTaskHandle
 * *************************************************************************/

 extern TaskHandle_t BQTaskHandle;
 extern struct BQFUNCTION bqfunction;

 extern union ADCCELLCOUNTS cellcts;

#endif

