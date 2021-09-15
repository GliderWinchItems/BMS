/******************************************************************************
* File Name          : levelwind_idx_v_struct.c
* Date First Issued  : 09/23/2020
* Board              :
* Description        : Load parameter struct
*******************************************************************************/

#include "levelwind_idx_v_struct.h"
#include "SerialTaskReceive.h"
#include "../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* *************************************************************************
 * void levelwind_idx_v_struct_hardcode_params(truct LEVELWINDLC* p);
 * @brief	: Init struct from hard-coded parameters (rather than database params in highflash)
 * @return	: 0
 * *************************************************************************/
void levelwind_idx_v_struct_hardcode_params(struct LEVELWINDLC* p)
{
	p->size    = 32;
	p->crc     = 0;   // TBD
   p->version = 1;   // 

	/* Timings in milliseconds. Converted later to timer ticks. */

/* GevcuTask counts 'sw1timer' ticks for various timeouts.
 We want the duration long, but good enough resolution(!)
 With systick at 512/sec, specifying 8 ms yields a 4 tick duration
 count = 4 -> 64/sec (if we want to approximate the logging rate)
 count = 64 -> 1/sec 
*/ 
	//    my drum should be associated with whole node and not just the
   //    level-wind function. need to figure out where the parameters for 
   //    the whole node should be placed 

   /*
      Need to have comments or compute parameters based on stepper setup
      and CNC parameters Example: number of steps per revoltuion (2000) 
      and the CNC ball screw lead (20 mm per revolution). All goes towards
      makein parameters SI units based on things like drum working width in
      millimeters.
   */      
   p->mydrum      = 1;     // drum this node is assigned (1-7)  

   p->hbct_t      = 500;   // Heartbeat ct: milliseconds between sending 

   p->Ka          = 8;     // Reversal rate
   p->Nr          = 3500;  // Sweep rate to reversal rate ratio
   p->Lplus       = 15000; // Calibrated start of positive reversal region
   p->Lminus      =    0;  // Calibrated start of negative reversal region

   /* DEH ocidx
   p->ocidx       = 21000; // Indexing increment for 250 us (4 kHz) This was from DEH)
   p->Nswp        = 8;     // Test sweep speed increment increase factor
   p->Nman        = 6;     // Manual sweep increment decrease factor
   */

   p->ocidx       = 15000; // Indexing increment (compare with DEH above)
   p->Nswp        = 4;     // Test sweep speed increment increase factor
   p->Nman        = 8;     // Manual sweep increment decrease factor



// For development; these will likely not be in operational code
   p->clfactor    = 168E3; // CL scaling: 100% = 50 us
   p->ka_levelwind_t = 2555;  // keep-alive timeout (timeout delay ms)
   p->cltimemax   = 512;   // Number of software timeout ticks max

/* 
      New Friendly/SI external parameters
         All distances in meters.
         All masses in kg. 
         All times in seconds. 
         All factors dimensionless
*/

   p->LimitSwitchSpan = 230e-3;     // between activation points
   p->LimitSwitchTol = 100e-3;
   p->OverrunSwitchSpan = 330e-3;   // between activation points    
   p->CenterOffset = 5.0e-3;
   p->DrumWidth = 165e-3;
   p->CableDiameter = 5e-3;
   p->ExcessRollerGap = 3e-3;
   p->LevelWindFactor = 2.5;
   p->ReversalFactor = 1.5;
   p->IndexingSweepSpeed = 25e-3;
   p->ManualSweepSpeed = 5e-3;
   p->TestSweepSpeed = 50e-3;
   p->NumberTestSweeps = 2;
   p->LevelWindHBPeriod = 500e-3;
   p->DrumInstance = 1;   // 1:7
   p->MicroStepsPerRevolution = 2000;
   p->StepperDirection = 1;
   p->BallScrewLead = 20e-3;
   p->EncoderPulsesPerRevolution = 360;
   p->EncoderDirection = 1;
   p->EncoderToDrumGearRatio = 35.0f/9.0f;
   p->StepperVoltageScale = 25e-3;
   p->StepperVoltageOffset = 0.0;
   
   // TBA   CAN Ids, see below



// CAN ids levelwind sends
   //                      CANID_HEX      CANID_NAME             CAN_MSG_FMT     DESCRIPTION
   // Others receive
// obsolete   p->cid_hb_levelwind  = 0xE4A00000;   // CANID_HB_LEVELWIND: U8_U32, Heartbeat Status, levelwind position accum');

// List of CAN ID's for setting up hw filter for incoming msgs
   	// We receive: Logger/gps 
//	p->cid_gps_sync     = 0x00400000; // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
	// We receive the following | stepper repo:levelwind (or later repos) sends
	p->cid_drum_tst_stepcmd	= CANID_TST_STEPCMD;     //'E4600000','GENCMD', 1,4, 'U8_FF','DRUM1: U8: Enable,Direction, FF: CL position');
   p->cid_hb_cpswsv1_1     = CANID_HB_CPSWSV1_1;    //'31000000','CPMC', 1,1,'S8_U8_7','HB_CPSWSV1 1: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');
   p->cid_hb_cpswsclv1_1   = CANID_HB_CPSWSCLV1_1;  //'31800000','CPMC', 2,1,'S8_S16_FF_V','HB_CPSWSV1 1:S8:status, S16 CL: (+/-10000 )');
   p->cid_cmd_levelwind_i1 = CANID_CMD_LEVELWIND_I1;//'B1000014','GENCMD',1,23,'U8_U8_U8_X4','1 incoming: U8:drum bits,U8:command code,X4:four byte value');
   p->cid_mc_state         = CANID_MC_STATE;        //'26000000','MC',1,5,'U8_U8','MC: Launch state msg');

   // We (levelwind) sends
   p->cid_cmd_levelwind_r1 = CANID_CMD_LEVELWIND_R1;//'B1000114','LEVELWIND',1,3,'U8_U8_U8_X4','1: U8:drum bits,U8:command code,X4:four byte value');
   p->cid_hb_levelwind_1   = CANID_HB_LEVELWIND_1;  //'80000000','LEVELWIND',1,2,'S8_U8','DRUM 1: S8:Status,U8:state');


	return;
}
