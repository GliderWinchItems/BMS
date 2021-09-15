/******************************************************************************
* File Name          : gevcu_idx_v_struct.c
* Date First Issued  : 10/08/2019
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include "gevcu_idx_v_struct.h"
#include "SerialTaskReceive.h"

/* *************************************************************************
 * void gevcu_idx_v_struct_hardcode_params(struct struct GEVCULC* p);
 * @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
void gevcu_idx_v_struct_hardcode_params(struct GEVCULC* p)
{
	p->size       = 47;
	p->crc        = 0;   // TBD
   p->version    = 1;   // 

	/* Timings in milliseconds. Converted later to timer ticks. */

/* GevcuTask counts 'sw1timer' ticks for various timeouts.
 We want the duration long, but good enough resolution(!)
 With systick at 512/sec, specifying 8 ms yields a 4 tick duration
 count = 4 -> 64/sec (if we want to approximate the logging rate)
 count = 64 -> 1/sec 
*/ 
	p->ka_t       = 4; // Gevcu polling timer: 4 * (1/512) 

	p->keepalive_t= 2555; // keep-alive timeout (timeout delay ms)
	p->hbct_t    = 1000; // Heartbeat ct: ticks between sending 
	p->ka_dmoc_r_t = 2;  // DMOC keepalive/torque command (sw1tim ticks)

 // CAN ids we send
   //                      CANID_HEX      CANID_NAME             CAN_MSG_FMT     DESCRIPTION
//	p->cid_cntctr_keepalive_i  = 0xE3800000; // CANID_CMD_CNTCTRKAI:U8': Contactor1: I KeepAlive and connect command
    // GEVCUr sends to PC response to keepalive msg PC sends to GEVCUr
//   p->cid_gevcur_keepalive_r = 0xE4200000; // CANID_CMD_GEVCURKAR: U8_U8 : GEVCUr: R KeepAlive response
//   p->cid_gevcur_ctllawv1    = 0xE4000000; // CANID_LOG_DMOCCMDSPD: FF    : GEVCUr: Desired speed');
    // DMOC receives these commands
//	p->cid_dmoc_cmd_speed  = 0x46400000;   // CANID_DMOC_CMD_SPEED: I16_X6,         DMOC: cmd: speed, key state
//	p->cid_dmoc_cmd_torq   = 0x46600000;   // CANID_DMOC_CMD_TORQ:  I16_I16_I16_X6, DMOC: cmd: torq,copy,standby,status
//	p->cid_dmoc_cmd_regen  = 0x46800000;   // CANID_DMOC_CMD_REGEN: I16_I16_X_U8_U8,DMOC: cmd: watt,accel,degC,alive

	// 

 // List of CAN ID's for setting up hw filter for incoming msgs
     // Contactor sends
//	p->cid_cntctr_keepalive_r = 0xE3C00000; // CANID_CMD_CNTCTRKAR: U8_U8_U8: Contactor1: R KeepAlive response to poll
     // PC sends to GEVCUr (i.e. "us")
//	p->cid_gevcur_keepalive_i = 0xE3E00000; // CANID_CMD_GEVCURKAI:U8 : GEVCUr: I KeepAlive and connect command
     // DMOC sends                        11 bit
//	p->cid_dmoc_actualtorq = 0x47400000; // 0x23A CANID_DMOC_ACTUALTORQ:I16,   DMOC: Actual Torque: payload-30000
//	p->cid_dmoc_speed      = 0x47600000; // 0x23B CANID_DMOC_SPEED:     I16_X6,DMOC: Actual Speed (rpm?)
//	p->cid_dmoc_dqvoltamp  = 0x47C00000; // 0x23E CANID_DMOC_DQVOLTAMP: I16_I16_I16_I16','DMOC: D volt:amp, Q volt:amp
//	p->cid_dmoc_torque     = 0x05683004; // CANID_DMOC_TORQUE:    I16_I16,'DMOC: Torque,-(Torque-30000)
//	p->cid_dmoc_critical_f = 0x056837fc; // CANID_DMOC_CRITICAL_F:    NONE',   'DMOC: Critical Fault: payload = DEADB0FF
//	p->cid_dmoc_hv_status  = 0xCA000000; // 0x650 CANID_DMOC_HV_STATUS: I16_I16_X6,'DMOC: HV volts:amps, status
//	p->cid_dmoc_hv_temps   = 0xCA200000; // 0x651 CANID_DMOC_HV_TEMPS:  U8_U8_U8,  'DMOC: Temperature:rotor,invert,stator
   // Others send
	p->cid_gps_sync     = 0x00400000; // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
		 // stepper repo: update100K sends
	p->cid_drum_tst_stepcmd	=  0xE4600000; // CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position:

	return;
}
