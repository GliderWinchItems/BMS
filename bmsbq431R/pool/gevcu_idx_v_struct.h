/******************************************************************************
* File Name          : gevcu_idx_v_struct.h
* Date First Issued  : 10/08/2019
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"
#include "iir_filter_lx.h"
#include "GevcuTask.h"

#ifndef __GEVCU_IDX_V_STRUCT
#define __GEVCU_IDX_V_STRUCT

/* GevcuTask counts 'sw1timer' ticks for various timeouts.
 SWTIM1TICKDURATION
 We want the duration long, but good enough resolution(!)
 With systick at 512/sec, specifying 8 ms yields a 4 tick duration
 count = 4 -> 64/sec (if we want to approximate the logging rate)
 count = 64 -> 1/sec 
*/ 
#define SWTIM1TICKDURATION 8
#define SWTIM1TICKPERSEC (1000/SWTIM1TICKDURATION)

#define SWTIM1_64PERSEC (configTICK_RATE_HZ/64) // swtim1 ticks 

/* Parameters gevcu instance */
struct GEVCULC
 {

/* NOTE: all suffix _t parameters are times in milliseconds */

	uint32_t size;
	uint32_t crc;   // TBD
   uint32_t version;   // 



	uint32_t ka_t;        // Gevcu polling timer

	/* Timings in milliseconds. Converted later to 'swtim1' ticks. */
	uint32_t keepalive_t;
	uint32_t ka_cntct_t;  // Contactor sending keepalive/command (ms)
	uint32_t ka_gevcur_t; // GEVCUr keepalive/commmand from PC (ms) 
	uint32_t ka_dmoc_r_t; // DMOC sending keepalive/command (ms)
	uint32_t ka_dmoc_i_t; // DMOC failed to receive timeout (ms)
	uint32_t hbct_t;      // Heartbeat ct: ticks between sending 

	/* Test parameters. */
	float vmax;    // Motor speed limit in plus direction
	float vmin;    // Motor speed limit in minus direction
	float tqplus;  // Torque plus
	float tqminus; // Torque minus

 // CAN ids ...........................................................................
   //                                  CANID_NAME             CAN_MSG_FMT     DESCRIPTION
    // GEVCUr sends; Contactor receives
	uint32_t cid_cntctr_keepalive_i; // CANID_CMD_CNTCTRKAI:U8',    Contactor1: I KeepAlive and connect command
    // GEVCUr sends; PC or "anybody" receives
	uint32_t cid_gevcur_keepalive_r;// CANID_CMD_GEVCURKAR:  U8_U8 : GEVCUr: R KeepAlive response
	uint32_t cid_gevcur_ctllawv1;   // CANID_LOG_DMOCCMDSPD: FF    : GEVCUr: Desired speed');
    // GEVCUr sends; DMOC receives these commands
	uint32_t cid_dmoc_cmd_speed;    // CANID_DMOC_CMD_SPEED: I16_X6,         DMOC: cmd: speed, key state
	uint32_t cid_dmoc_cmd_torq;     // CANID_DMOC_CMD_TORQ:  I16_I16_I16_X6, DMOC: cmd: torq,copy,standby,status
	uint32_t cid_dmoc_cmd_regen;    // CANID_DMOC_CMD_REGEN: I16_I16_X_U8_U8,DMOC: cmd: watt,accel,degC,alive
	// GEVCUr sends: logger, or PC capture logging, receives.

 // List of CAN ID's for setting up hw filter for incoming msgs
     // Contactor sends; we receive
	uint32_t cid_cntctr_keepalive_r; // CANID_CMD_CNTCTRKAR: U8_U8_U8: Contactor1: R KeepAlive response to poll
     // PC sends;  we receive 
	uint32_t cid_gevcur_keepalive_i; // CANID_CMD_GEVCURKAI:U8 : GEVCUr: I KeepAlive and connect command
   // DMOC sends; we receive
	uint32_t cid_dmoc_actualtorq; // CANID_DMOC_ACTUALTORQ:I16,   DMOC: Actual Torque: payload-30000
	uint32_t cid_dmoc_speed;      // CANID_DMOC_SPEED:     I16_X6,DMOC: Actual Speed (rpm?)
	uint32_t cid_dmoc_dqvoltamp;  // CANID_DMOC_DQVOLTAMP: I16_I16_I16_I16','DMOC: D volt:amp, Q volt:amp
	uint32_t cid_dmoc_torque;     // CANID_DMOC_TORQUE:    I16_I16,'DMOC: Torque,-(Torque-30000)
	uint32_t cid_dmoc_critical_f; // CANID_DMOC_CRITICAL_F:    NONE',   'DMOC: Critical Fault: payload = DEADB0FF
	uint32_t cid_dmoc_hv_status;  // CANID_DMOC_HV_STATUS: I16_I16_X6,'DMOC: HV volts:amps, status
	uint32_t cid_dmoc_hv_temps;   // CANID_DMOC_HV_TEMPS:  U8_U8_U8,  'DMOC: Temperature:rotor,invert,stator
   // GPS/Logger sends; we receive
	uint32_t cid_gps_sync; // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
		// stepper test repo sends: drum receives
	uint32_t cid_drum_tst_stepcmd; // CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000

 };

/* *************************************************************************/
void gevcu_idx_v_struct_hardcode_params(struct GEVCULC* p);
/* @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
 
#endif

