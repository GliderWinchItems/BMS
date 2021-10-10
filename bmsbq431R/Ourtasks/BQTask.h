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

#define BQVSIZE 20 // Readout loop size (16 cells plus others)

#define NUMCANMSGS 16 // 

#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 5v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells above max limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Charging in progress

#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate


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

/* Working struct for BQ function. */
struct BQFUNCTION
{
   // Parameter loaded either by high-flash copy, or hard-coded subroutine
	struct BQLC lc; // Fixed parameters, (lc = Local Copy)

//	struct ADCFUNCTION* padc; // Pointer to ADC working struct

	/* Timings in milliseconds. Converted later to timer ticks. */
	uint32_t ka_k;        // Gevcu polling timer
	uint32_t keepalive_k;

	uint32_t hbct_k;      // Heartbeat ct: ticks between sending

//	uint8_t chargeflag;  // 0 = No charging; not zero = charging
//	uint8_t dumpflag;    // 0 = Dump FET OFF; not zero = dump fet ON
//	uint8_t extchgrflag; // 0 = Dump2 (external charger) OFF; not zero = ON
//	uint8_t cellv_ok;    // 0 = voltage readings not valid; not zero = OK

	uint16_t tim1_ccr1;  // Present CCR1 (PWM count)

	struct BQCELLV cellv_bal[NCELLMAX]; // Working array for cell balancing
	int16_t cellv_latest[NCELLMAX]; // Cell voltage readings (millivolts) (signed)
	int32_t cellv_total;  // Some of cell voltages
	int16_t cellv_high;   // Highest cellv millivolts
	int16_t cellv_low;    // Lowest  cellv millivolts
	uint8_t cellx_high;   // Highest cellv index (0-15)
	uint8_t cellx_low;    // Lowest  cellv index (0-15)

	uint32_t cellbal;       // Bits to activate cell balance fets
	uint8_t active_ct;      // Count of bits set in cellbal
	uint8_t battery_status; // Cell status code bits 
	uint8_t fet_status;     // This controls on/off of FETs

	/* balnumwrk might be adjusted based on chip temperature. */
	uint8_t balnumwrk; // Max number of active cell bits (Working)


	/* Pointers to incoming CAN msg mailboxes. */
//	struct MAILBOXCAN* pmbx_cid_gps_sync;        // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
//	struct MAILBOXCAN* pmbx_cid_drum_tst_stepcmd;// CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000


	uint8_t state;      // main state
	uint8_t substateA;  // 
	uint8_t substateB;  // spare substate 

	/* CAN msgs */
//	struct CANTXQMSG canmsg[NUMCANMSGS];


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

