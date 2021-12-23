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

/* *************************************************************************
 * void levelwind_func_init_init(struct LEVELWINDFUNCTION* p);
 *	@brief	: Initialize working struct for Levelwind Task
 * @param	: p    = pointer to Levelwind Task
 * *************************************************************************/
void levelwind_func_init_init(struct LEVELWINDFUNCTION* p)
{
	int i;

   // Initialize hardcoded parameters (used in some computations below)
   levelwind_idx_v_struct_hardcode_params(&p->lc);

	p->Ks = p->lc.Nr * p->lc.Ka; // Sweep rate (Ks/65536) = levelwind pulses per encoder edge

		
// Skip CAN msg arrival notifications for lines that are commented out.
	/* Add CAN Mailboxes                               CAN     CAN ID             TaskHandle,Notify bit,Skip, Paytype */
//	p->pmbx_cid_gps_sync          =  MailboxTask_add(pctl0,p->lc.cid_gps_sync,          NULL,LEVELWINDBIT06,0,U8);
	p->pmbx_cid_drum_tst_stepcmd = MailboxTask_add(pctl0,p->lc.cid_drum_tst_stepcmd,NULL,LEVELWINDSWSNOTEBITCAN1,0,U8_FF);
   p->pmbx_cid_mc_state         = MailboxTask_add(pctl0,p->lc.cid_mc_state,        NULL,LEVELWINDSWSNOTEBITCAN2,0,U8_FF);   
	/* Pre-load fixed data in CAN msgs */
	for (i = 0; i < NUMCANMSGSLEVELWIND; i++)
	{
		p->canmsg[i].pctl = pctl0;   // Control block for CAN module (CAN 1)
		p->canmsg[i].maxretryct = 8; //
		p->canmsg[i].bits = 0;       //
		p->canmsg[i].can.dlc = 8;    // Default payload size (might be modified when loaded and sent)
	}

	  /* Pre-load CAN msg id and dlc. */
   // Stepper heartbeat
   p->canmsg[CID_LEVELWIND_HB].can.id  = p->lc.cid_hb_levelwind; // CAN id.
   p->canmsg[CID_LEVELWIND_HB].can.dlc = 5;    // U8_U32 payload

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

   p->rvrsldx = (p->lc.Nr * (p->lc.Nr - 1) * p->lc.Ka) / 2;
   
   /* this initialization has been moved to the OFF case in LevelwindTask
   // Position accumulator initial value. Reference paper for the value employed.
   // p->posaccum.s32 = (p->lc.Lminus << 16) - p->rvrsldx;
   p->posaccum.s32 = 0; // temporary to have it start at 0.
   p->pos_prev = p->posaccum.s32;
   // initialize 32-bit values for Lplus32 and Lminus32. Reference paper
   // p->Lminus32 = p->lc.Lminus << 16;
   p->Lminus32 = (p->lc.Lminus << 16) + p->rvrsldx;
   p->Lplus32  = p->Lminus32 
      + (((p->lc.Lplus - p->lc.Lminus) << 16) / p->Ks) * p->Ks;
   p->velaccum.s32 = 0;             // Velocity accumulator initial value  
   
   */

   p->drbit = p->drbit_prev = 0;    // Drum direction bit REVIST: Needed???   

   p->hbctr      = xTaskGetTickCount();

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
   

   /* Convert levelwind_idx_v_struct times to timer ticks. */
   p->keepalive_k = (p->lc.ka_levelwind_t); // keep-alive timeout (timeout delay ms)

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

   /* Enable limit and overrun switch interrupts EXTI15_10. */
   EXTI->IMR  |= 0xf000;  // Interrupt mask reg: enable 10:15
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
	return;
}
