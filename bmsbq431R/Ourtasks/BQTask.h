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

	uint8_t chargeflag;  // 0 = No charging; not zero = charging
	uint8_t dumpflag;    // 0 = Dump FET OFF; not zero = dump fet ON
	uint8_t extchgrflag; // 0 = Dump2 (external charger) OFF; not zero = ON
	uint8_t cellv_ok;    // 0 = voltage readings not valid; not zero = OK

	uint16_t tim1_ccr1;  // Present CCR1 (PWM count)

	uint32_t cellv_latest[16];

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

