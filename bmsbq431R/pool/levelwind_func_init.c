/******************************************************************************
* File Name          : levelwind_func_init.c
* Date First Issued  : 09/23/2020
* Description        : LevelwindTask initialize function struct
*******************************************************************************/

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "timers.h"
#include "MailboxTask.h"
#include "CanTask.h"
#include "can_iface.h"
#include "CanTask.h"
#include "main.h"
#include "morse.h"
#include "canfilter_setup.h"
#include "levelwind_func_init.h"
#include "levelwind_idx_v_struct.h"
#include "levelwind_items.h"
#include "../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* From 'main.c' */
extern struct CAN_CTLBLOCK* pctl0;	// Pointer to CAN1 control block
extern CAN_HandleTypeDef hcan1; //

extern TIM_HandleTypeDef htim2; // Timer FreeRTOS handle
extern TIM_HandleTypeDef htim5; // Timer FreeRTOS handle
extern TIM_HandleTypeDef htim9; // Timer FreeRTOS handle

/* From levelwind_items.c. */
extern TIM_TypeDef  *pT2base; // Register base address 
extern TIM_TypeDef  *pT5base; // Register base address 
extern TIM_TypeDef  *pT9base; // Register base address 

// #defines for computation of derivited parameters
#define LSBS_PER_MICROSTEP (1 << 16)   // LSBs in the lower accumulator
#define MICROSTEP_RANGE (1 << 16)      // Upper accumulator LSBs 
#define INTERRUPTS_PER_ENCODER_PULSE 2

/* *************************************************************************
 * void levelwind_func_init_init(struct LEVELWINDFUNCTION* p);
 *	@brief	: Initialize working struct for Levelwind Task
 * @param	: p    = pointer to Levelwind Task
 * *************************************************************************/
void levelwind_func_init_init(struct LEVELWINDFUNCTION* p)
{
	int i;
   int error_code = 0;

   // Initialize hardcoded parameters (used in  computations below)
   levelwind_idx_v_struct_hardcode_params(&p->lc);

   // validate external parameters' ranges
   // note, error code is only for first out of range parameter identified
   if (p->lc.LimitSwitchSpan <= 0.0f || p->lc.LimitSwitchSpan > 1.0f)
      error_code = 100;
   else if (p->lc.LimitSwitchTol <= 0.0f || p->lc.LimitSwitchTol > 100e-3f)
      error_code = 101;
   else if (p->lc.CenterOffset <= 0.0f || p->lc.CenterOffset > 500e-3f)
      error_code = 102;
   else if (p->lc.DrumWidth <= 0.0f || p->lc.DrumWidth > 1.0f)
      error_code = 103;
   else if (p->lc.CableDiameter <= 0.0f || p->lc.CableDiameter > 100e-3f)
      error_code = 104;
   else if (p->lc.ExcessRollerGap <= 0.0f || p->lc.ExcessRollerGap > 150e-3f)
      error_code = 105;
   else if (p->lc.LevelWindFactor <= 0.0f || p->lc.LevelWindFactor > 10.0f)
      error_code = 106;
   else if (p->lc.ReversalFactor <= 0.0f || p->lc.ReversalFactor > 10.0f)
      error_code = 107;
   else if (p->lc.IndexingSweepSpeed <= 0.0f || p->lc.IndexingSweepSpeed > 1.0f)
      error_code = 108;
   else if (p->lc.ManualSweepSpeed <= 0.0f || p->lc.ManualSweepSpeed > 100.0e-3f)
      error_code = 109;
   else if (p->lc.TestSweepSpeed <= 0.0f || p->lc.TestSweepSpeed > 1.0f)
      error_code = 110;
   else if (p->lc.NumberTestSweeps <= -1 || p->lc.NumberTestSweeps > 100)
      error_code = 111;
   else if (p->lc.LevelWindHBPeriod <= 0.0f || p->lc.LevelWindHBPeriod > 100.0f)
      error_code = 112;
   else if (p->lc.DrumInstance < 1 || p->lc.DrumInstance > 7)
      error_code = 113;
   else if (p->lc.MicroStepsPerRevolution < 100 || p->lc.MicroStepsPerRevolution > 100000)
      error_code = 114;
   else if (!(p->lc.StepperDirection == 1 || p->lc.StepperDirection == -1))
      error_code = 115;
   else if (p->lc.BallScrewLead <= 0.0f || p->lc.BallScrewLead > 100e-3f)
      error_code = 116;
   else if (p->lc.EncoderPulsesPerRevolution <= 50 || p->lc.EncoderPulsesPerRevolution > 20000)
      error_code = 117;
   else if (!(p->lc.EncoderDirection == 1 || p->lc.EncoderDirection == -1))
      error_code = 118;
   else if (p->lc.EncoderToDrumGearRatio <= 0.0f || p->lc.EncoderToDrumGearRatio > 10.0f)
      error_code = 119;
   else if (p->lc.StepperVoltageScale <= 0.0f || p->lc.StepperVoltageScale > 5.0f)
      error_code = 120;
   else if (p->lc.StepperVoltageOffset <= -100.0f || p->lc.StepperVoltageOffset > 100.0f)
      error_code = 121;

   if (error_code != 0)
   {
      // generate some error condition and inform operator
      morse_trap(error_code);
   }


#if 1 // enable to use new parameters
   // Revist: Paramter range tests need to be added
   
   // level-wind delta x per position accumulator lsb
   float dxperlsb = p->lc.BallScrewLead 
      / (float) (p->lc.MicroStepsPerRevolution *  LSBS_PER_MICROSTEP);
   
   // lateral cable motion per drum revolution
   float cabledxperdrumrevolution = p->lc.LevelWindFactor 
            * p->lc.CableDiameter;              
   
   // encoder interrupts per drum revolution
   float interrupts_per_drum_revolution = (float) INTERRUPTS_PER_ENCODER_PULSE
            * (float) p->lc.EncoderPulsesPerRevolution 
            * p->lc.EncoderToDrumGearRatio;  
      
   // tentative Ks
   float fKs = cabledxperdrumrevolution
      / (dxperlsb * interrupts_per_drum_revolution);
   
      
   // compute tentative Nr
   float fNr = (2 * p->lc.CableDiameter * p->lc.ReversalFactor 
      / (dxperlsb * fKs)) + 1.0f;
      
   
   // round ratio of fKs/fNr to compute integer Ka 
   p->Ka = fKs / fNr + 0.5f;
   
   // compute integer Nr to get as close to  fKs as possible
   p->Nr = (fKs / (float) p->Ka) + 0.5f;

   // compute resulting integer Ks value
   p->Ks = p->Nr * p-> Ka;
   if (p->Ks > LSBS_PER_MICROSTEP)
      error_code = 200; // max speed is too large for lower accumulator size

   // compute width in meters of the linear sweep
   p->rvrsl = (p->Nr - 1) * p->Ks / 2;
   float linearsweepwidth = p->lc.DrumWidth - p->lc.CableDiameter
      - (float) (2 * p->rvrsl) * dxperlsb + p->lc.ExcessRollerGap;   

   // calculate the offset corrected values for the 32-bit values
   // all rounding done using truncation of positive numbers
   p->Lpos = (linearsweepwidth + p->lc.CenterOffset) / (2.0f * dxperlsb) + 0.5f;
   p->Lpos = ((p->Lpos + p->Ks/2) / p->Ks) * p->Ks;    // round to multiple of Ks
   p->Lneg = (linearsweepwidth - p->lc.CenterOffset) / (2.0f * dxperlsb) + 0.5f;
   p->Lneg = -(((p->Lneg + p->Ks/2) / p->Ks) * p->Ks); // round to multiple of Ks and negate   
   if (((((long) p->Lpos + p->rvrsl) / LSBS_PER_MICROSTEP) > (MICROSTEP_RANGE / 2 - 1))
      || ((-((long) p->Lneg - p->rvrsl) / LSBS_PER_MICROSTEP) > (MICROSTEP_RANGE / 2)))
         error_code = 201; // upper accumulator not long enough for drum width
   
   // Windxswp is about half the overrun switch span and a multiple of Ks
   p->Windxswp = (p->lc.OverrunSwitchSpan) / (2.0f * dxperlsb) + 0.5f;
   p->Windxswp = ((p->Windxswp + p->Ks/2) / p->Ks) * p->Ks; // round to multiple of Ks

   // limit switch span and tolerance used for error dectection only so not requiring rounding
   p->LSSpan = p->lc.LimitSwitchSpan / dxperlsb;   // limit switch span
   p->LSTol = p->lc.LimitSwitchTol / dxperlsb;     // limit switch tolerance
   /* REVIST: Could check if expected limit switch activation points are beyond the 
      reversal points using LSSpan 
   */

   // stepper direction indicator
   p->stpprdiri = (p->lc.StepperDirection == 1) ? 0 : 1;

   // some misc values
   p->mydrum = p->lc.DrumInstance;
   p->mydrumbit = (1 << (p->mydrum - 1)); // Convert drum number (1-7) to bit position (0-6)
   p->hbct_k = pdMS_TO_TICKS(p->lc.LevelWindHBPeriod * 1000);

   
#else // use old parameter set to compute Ks

	p->Ks = p->lc.Nr * p->lc.Ka; // Sweep rate (Ks/65536) = levelwind pulses per encoder edge

   p->hbct_k = pdMS_TO_TICKS(p->lc.hbct_t);  // Convert ms to RTOS ticks: Heartbeat duration

   p->mydrumbit = (1 << (p->lc.mydrum-1)); // Convert drum number (1-7) to bit position (0-6)
 #endif

   



	/* Add CAN Mailboxes                               CAN     CAN ID             TaskHandle,Notify bit,Skip, Paytype */
//	p->pmbx_cid_gps_sync         =  MailboxTask_add(pctl0,p->lc.cid_gps_sync,       NULL,LEVELWINDBIT06,0,U8);
	p->pmbx_cid_drum_tst_stepcmd = MailboxTask_add(pctl0,p->lc.cid_drum_tst_stepcmd,NULL,LEVELWINDSWSNOTEBITCAN1,0,U8_FF);
   p->pmbx_cid_mc_state         = MailboxTask_add(pctl0,p->lc.cid_mc_state,        NULL,LEVELWINDSWSNOTEBITCAN2,0,U8_U8); 
   p->pmbx_cid_hb_cpswsv1_1     = MailboxTask_add(pctl0,p->lc.cid_hb_cpswsv1_1,    NULL,LEVELWINDSWSNOTEBITCAN3,0,S8_U8_7);
   p->pmbx_cid_hb_cpswsclv1_1   = MailboxTask_add(pctl0,p->lc.cid_hb_cpswsclv1_1,  NULL,LEVELWINDSWSNOTEBITCAN4,0,S8_S16_FF_V);
   p->pmbx_cid_cmd_levelwind_i1 = MailboxTask_add(pctl0,p->lc.cid_cmd_levelwind_i1,NULL,LEVELWINDSWSNOTEBITCAN5,0,U8_U8_U8_X4);

	/* Pre-load fixed data in CAN msgs */
	for (i = 0; i < NUMCANMSGSLEVELWIND; i++)
	{
		p->canmsg[i].pctl = pctl0;   // Control block for CAN module (CAN 1)
		p->canmsg[i].maxretryct = 8; //
		p->canmsg[i].bits = 0;       //
		p->canmsg[i].can.dlc = 8;    // Default payload size (might be modified when loaded and sent)
	}

	/* Pre-load CAN msg id and dlc. */
   // Levelwind command response
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.id  = p->lc.cid_cmd_levelwind_r1; // CAN id.
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.dlc = 7; // U8_U8_U8_X4

   // Levelwind status & heartbeat
   p->canmsg[IDX_CID_HB_LEVELWIND_1].can.id  = p->lc.cid_hb_levelwind_1; // CAN id.
   p->canmsg[IDX_CID_HB_LEVELWIND_1].can.dlc = 2;  // S8_U8

#if LEVELWINDDEBUG 
	/* DEBUG buffer. */ 
   p->pdbgbegin = &levelwinddbgbuf[0];
   p->pdbgadd   = &levelwinddbgbuf[0];
   p->pdbgtake  = &levelwinddbgbuf[0];
   p->pdbgend   = &levelwinddbgbuf[LEVELWINDDBGBUFSIZE];;

   p->ledctr1   = 0;
   p->ledctr2   = 0;

   /* Bit positions for low overhead toggling. */
   p->ledbit1= (LED_GREEN_Pin);
   p->ledbit2= (LED_ORANGE_Pin);
#endif   

   

   p->drbit = p->drbit_prev = 0;    // Drum direction bit REVIST: Needed???   

   p->hbctr = xTaskGetTickCount();

   // initialize state machines and status
   p->state = p->state_prev = LW_OFF;
   p->status = p->status_prev = LW_STATUS_GOOD;
   p->isr_state = LW_ISR_OFF;
   p->mode = LW_MODE_OFF;
   p->indexed = 0;   //REVIST: May not be needed

   p->mc_state = MC_SAFE;

   // TIM2 output compare increments
   p->ocswp       = p->lc.ocidx/p->lc.Nswp;     // initalize sweep increment
   p->ocman       = p->lc.ocidx * p->lc.Nman;   // initalize manual increment 
 
   // for development;these will likely not be in operational code
   p->ocfauxinc   = 8400000;   // Default 1/10 sec duration
   p->cltimectr   = 0;
   
   /* Save base addresses of timers for faster use later. */
   pT2base  = htim2.Instance;
   pT5base  = htim5.Instance;
   pT9base  = htim9.Instance;

/* ### NOTE ### These might override STM32CubeMX settings. ### */
   /* Generate pulse for levelwind controller (PU line) */
   pT9base->DIER = 0;// No interrupt
   pT9base->CCR1 = TIM9PULSEDELAY; // Delay count
   pT9base->ARR  = (TIM9PWMCYCLE - 1); // (10 us)
   pT9base->CCER = 0x3; // OC active high; signal on pin

/* ### NOTE ### These might override STM32CubeMX settings. ### */

   /* TIM2 Shaft encoder input capture times & output caputre indexing interrupts. */
   pT2base->CCER |= 0x1110; // Input capture active: CH2,3,4
#if LEVELWINDDEBUG   
   pT2base->DIER  = 0xE;    // CH1,2,3 interrupt enable
#else   
   pT2base->DIER  = 0xA;    // CH1,3 interrupt enable
#endif   
   pT2base->CCR1  = pT2base->CNT + 1000; // 1 short delay
   pT2base->ARR   = 0xffffffff; // (Max count - 1)

   /* Start counters. */
   pT2base->CR1 |= 1;  // TIM2: CH1 oc, CH3 ic/oc
   pT5base->CR1 |= 1;  // TIM5: encoder CH1 CH2 (no interrupt)
   // if encoder direction is -1, reverse count
   if ( p->lc.EncoderDirection == -1  ) pT5base->CCER |= 0x2;

   return;
}
/* *************************************************************************
 * static void canfilt(uint16_t mm, struct MAILBOXCAN* p);
 * @brief	: Setup CAN hardware filter with CAN addresses to receive
 * @param	: p    = pointer to ContactorTask
 * @param   : mm = morse_trap numeric number
 * *************************************************************************/
static void canfilt(uint16_t mm, struct MAILBOXCAN* p)
{
/*	HAL_StatusTypeDef canfilter_setup_add32b_id(uint8_t cannum, CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint8_t  fifo );
 @brief	: Add a 32b id, advance bank number & odd/even
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: id    = 32b CAN id
 * @param	: fifo  = fifo: 0 or 1
 * @return	: HAL_ERROR or HAL_OK
*/
	HAL_StatusTypeDef ret;	
	ret = canfilter_setup_add32b_id(1,&hcan1,p->ncan.can.id,0);
	if (ret == HAL_ERROR) morse_trap(mm);	
	return;
}
/* *************************************************************************
 * void levelwind_func_init_canfilter(struct LEVELWINDFUNCTION* p);
 *	@brief	: Setup CAN hardware filter with CAN addresses to receive
 * @param	: p    = pointer to Gevcu function parameters
 * *************************************************************************/
void levelwind_func_init_canfilter(struct LEVELWINDFUNCTION* p)
{
//	canfilt(661,p->pmbx_cid_gps_sync);
	canfilt(660,p->pmbx_cid_drum_tst_stepcmd);
   canfilt(661,p->pmbx_cid_hb_cpswsv1_1);
   canfilt(662,p->pmbx_cid_hb_cpswsclv1_1);
   canfilt(663,p->pmbx_cid_cmd_levelwind_i1);
   canfilt(664,p->pmbx_cid_mc_state);

	return;
}
