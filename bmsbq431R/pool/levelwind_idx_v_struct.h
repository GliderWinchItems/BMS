/******************************************************************************
* File Name          : levelwind_idx_v_struct.h
* Date First Issued  : 09/23/2020
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"
#include "LevelwindTask.h"

#ifndef __LEVELWIND_IDX_V_STRUCT
#define __LEVELWIND_IDX_V_STRUCT

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

/* Parameters levelwind instance (LC = Local Copy) */
struct LEVELWINDLC
 {
/* NOTE: all suffix _t parameters are times in milliseconds */

	uint32_t size;
	uint32_t crc;   // TBD
   uint32_t version;   //

   // this belongs somewhere associated with the node, not the LW
   uint8_t  mydrum;     // the drum number for this node 

   uint32_t hbct_t;     // Heartbeat ct: milliseconds between sending 

	/* Timings in milliseconds. Converted later to 'swtim1' ticks. */
   uint32_t hbctmin_t;  // Minimum duration: between heartbeats (ms)

   // REVISIT: these should go away
   int32_t  Lplus;      // start of positive reversal region 
   int32_t  Lminus;     // start of negative reversal region
   

   int32_t  Ka;         // reversal rate
   int32_t  Nr;         // ratio of reversal rate to sweep rate


   uint32_t ocidx;      // OC register increment for indexing 
   uint8_t  Nswp;       // sweep rate speed-up factor
   int32_t  Nman;       // manual rate reduction factor

   // For development, these will likely not be needed in operational code
   float    clfactor;   // Constant to compute oc duration at CL = 100.0
   uint32_t cltimemax;  // Max timer count for shutdown 
   uint32_t ka_levelwind_t; // keepalive from PC (ms)

   /* 
      New Friendly/SI external parameters
         All distances in meters.
         All masses in kg. 
         All times in seconds. 
         All factors dimensionless
         See Level-wind Parameters document for details
   */

   float    LimitSwitchSpan;
   float    LimitSwitchTol;
   float    OverrunSwitchSpan;
   float    CenterOffset;
   float    DrumWidth;
   float    CableDiameter;
   float    ExcessRollerGap;
   float    LevelWindFactor;
   float    ReversalFactor;
   float    IndexingSweepSpeed;
   float    ManualSweepSpeed;
   float    TestSweepSpeed;
   int32_t  NumberTestSweeps;
   float    LOSTolerance;
   float    LevelWindHBPeriod;
   uint32_t DrumInstance;   // 1:7
   int32_t  MicroStepsPerRevolution;
   int32_t  StepperDirection;
   float    BallScrewLead;
   int16_t  EncoderPulsesPerRevolution;
   int32_t  EncoderDirection;
   float    EncoderToDrumGearRatio;
   float    StepperVoltageScale;
   float    StepperVoltageOffset;
   
   // TBA   CAN Ids




 // CAN ids ...........................................................................
   //                                  CANID_NAME             CAN_MSG_FMT     DESCRIPTION
    // Levelwind sends; PC receives
   uint32_t cid_hb_levelwind;        // CANID_HB_LEVELWIND: U8_U32','LEVELWIND: U8: Status, U32: stepper position accum

 // List of CAN ID's for setting up hw filter for incoming msgs
	// stepper test repo sends: levelwind receives
	uint32_t cid_drum_tst_stepcmd; // CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000
   uint32_t cid_hb_cpswsv1_1;     // CANID_HB_CPSWSV1_1'  ,'31000000','CPMC', 1,1,'S8_U8_7','HB_CPSWSV1 1: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');
   uint32_t cid_hb_cpswsclv1_1;   // CANID_HB_CPSWSCLV1_1','31800000','CPMC', 2,1,'S8_S16_FF_V','HB_CPSWSV1 1:S8:status, S16 CL: (+/-10000 )');
   uint32_t cid_cmd_levelwind_i1; // CANID_CMD_LEVELWIND_I1','B1000014','GENCMD',1,23,'U8_U8_U8_X4','1 incoming: U8:drum bits,U8:command code,X4:four byte value');
   uint32_t cid_mc_state;         // CANID_MC_STATE','26000000','MC',1,5,'U8_U8','MC: Launch state msg');

   // Levelwind sends : others receive
   uint32_t cid_cmd_levelwind_r1; // CANID_CMD_LEVELWIND_R1','B1000114','LEVELWIND',1,3,'U8_U8_U8_X4','1: U8:drum bits,U8:command code,X4:four byte value');
   uint32_t cid_hb_levelwind_1;   // CANID_HB_LEVELWIND_1','80000000','LEVELWIND',1,2,'S8_U8','DRUM 1: S8:Status,U8:state');
 };

/* *************************************************************************/
void levelwind_idx_v_struct_hardcode_params(struct LEVELWINDLC* p);
/* @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
 
#endif

