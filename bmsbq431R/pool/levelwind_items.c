/******************************************************************************
* File Name          : levelwind_items.c
* Date First Issued  : 09/23/2020
* Description        : Levelwind levelwind motor algorithm and items
*******************************************************************************/
/*
09/10/2020 realfaux branch

09/03/2020 testencoder branch 
Mods reminder list 
- CH2 fauxencoder -> CH1 testencoder (output compare)
- PA0-PA2 high is about 3.9v because PA0 pushbutton has 220K pull-down resistor
  PA1-PA3 high is 4.48v 
  PB3 - high is 5.05v

08/23/2020 druminversion branch started

08/10/2020 - pins added to Control Panel for levelwind testing

Control lines: Output pin drives FET gate, open drain to controller opto-isolator
PE5  - TIM9CH1 Stepper Pulse: PU (TIM1 Break shared interrupt vector)
   Interrupt vector: TIM1_BRK_TIM9_IRQHandler
PB0  - Direction: DR 
PB1  - Enable: EN  - High = enable (drive FET ON)

Drum encoder: TIM2CH1. Pullup resistors
PA0 - Encoder channel A
PA1 - Encoder channel B

TIM2 32b (84 MHz) capture mode (interrupt)
   CH3 PA2 input capture: encoder A (TIM5 PA0)
   CH4 PA3 input capture: encoder B (TIM5 PA1)
   CH2 PB3 input capture: encoder Z
   CH1 no-pin Indexing interrupts

TIM5 32b encoder counter (no interrupt)
   CH3 PA0 encoder config: encoder A (TIM2 PA2)
   CH4 PA1 encoder config: encoder B (TIM2 PA3)

TIM9 (168 MHz) Delayed levelwind pulse (no interrupt)
   CH1 PE5 PWM/OPM: Stepper pulse

TIM13 (84 MHz) Solenoid FET drive (no interrupt)
   CH1 PA6 PWM (4 KHz)

*/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "stm32f4xx_hal.h"
#include "morse.h"
#include "yprintf.h"
#include "main.h"
#include "levelwind_items.h"
#include "DTW_counter.h"
#include "drum_items.h"
#include "levelwind_switches.h"

#define DTW 1  // True to keep DTW timing Code


/* Union for various types of four byte CAN payloads. */
union X4
{
   uint8_t  u8[4];
   int32_t  s32;
   uint32_t u32;
   float    ff;
};

struct LEVELWINDDBGBUF levelwinddbgbuf[LEVELWINDDBGBUFSIZE];

TIM_TypeDef  *pT2base; // Register base address 
TIM_TypeDef  *pT5base; // Register base address 
TIM_TypeDef  *pT9base; // Register base address 

/* Struct with all you want to know. */
struct LEVELWINDFUNCTION levelwindfunction;

/* *************************************************************************
 * void levelwind_items_timeout(void);
 * @brief   : Check for loss of CL CAN msgs
 * *************************************************************************/
void levelwind_items_timeout(void)
{
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer

   p->cltimectr += 1;
   if (p->cltimectr >= p->lc.cltimemax)
   { // We timed out! Stop the levelwind
      p->cltimectr = p->lc.cltimemax;

      /* Set enable bit which turns FET on, which disables levelwind. */
      // Possible Error. This seems to reset the enable which would enable 
      // the stepper. This leaves stepper enabled
      //p->enflag = (Stepper_MF_Pin << 16); // Set bit with BSRR storing

      /* Bits positioned for updating PB BSRR register. */
      p->iobits = p->drflag | p->enflag;
   }
   return;
}
/* *************************************************************************
 * void levelwind_items_CANsend_hb_levelwind_1(void);
 * @brief   : CAN msg send: heartbeat for levelwind
 * *************************************************************************/
 void levelwind_items_CANsend_hb_levelwind_1(void)
 {
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
   /* Setup CAN msg */
   p->canmsg[IDX_CID_HB_LEVELWIND_1].can.cd.uc[0] = p->status;
   p->canmsg[IDX_CID_HB_LEVELWIND_1].can.cd.uc[1] = p->isr_state;
   
   /* Queue CAN msg to send. */
   xQueueSendToBack(CanTxQHandle,&p->canmsg[IDX_CID_HB_LEVELWIND_1],4);  
   return;
 }
/* *************************************************************************
 * void levelwind_items_rcv_cid_hb_cpswsclv1_1(struct CANRCVBUF* pcan);
 * @param   : pcan = pointer to CAN msg struct
 * @brief   : CAN msg rcv: cid_hb_cpswsclv1_1 (control lever position)
 * *************************************************************************/
 void levelwind_items_rcv_cid_hb_cpswsclv1_1(struct CANRCVBUF* pcan)
 {
#ifdef USECPSWSMSGS  // Use CP switches and CL
   struct CONTROLPANELSTATE* pcp = &cp_state;   // Convenience pointer
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
   int32_t tmp;

     /* Check Control Lever status. */
   if (pcan->cd.sc[0] < 0) return;

   /* Convert two byte int (+/-10,000) to float (+/-100.0) */
   tmp = (pcan->cd.uc[2] << 8) + pcan->cd.uc[1];
   p->clpos = (float)tmp * 0.01f;

   // JIC: someone uses the CL pos from CP struct
   pcp->clpos = p->clpos;

// The following code was lifted from levelwind_items_clupdate
     /* Convert CL position (0.0 - 100.0) to output comnpare duration increment. */
#define MAXDURF (84E5f) // 1/10sec per faux encoder interrupt max duration
   p->focdur = (p->lc.clfactor / p->clpos);
   if ( p->focdur > (MAXDURF))
   { 
      p->focdur = MAXDURF; // Hold at max
   }
   p->ocfauxinc = p->focdur;   // Convert to integer
#endif  
   
   return;
}
 /* *************************************************************************
 * void levelwind_items_rcv_cid_hb_cpswsv1_1(struct CANRCVBUF* pcan);
 * @param   : pcan = pointer to CAN msg struct
 * @brief   : CAN msg rcv: cid_hb_cpswsv1_1 (switches)
 * *************************************************************************/
 void levelwind_items_rcv_cid_hb_cpswsv1_1(struct CANRCVBUF* pcan)
 {
#ifdef USECPSWSMSGS  // Use CP switches and CL

   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
   struct CONTROLPANELSTATE* pcp = &cp_state;   // Convenience pointer

   /* Update struct with control panel switch settings. */
   if (levelwind_task_cp_state_update(pcan) < 0) return; // Return not ready

   /* Is this drum operational. */
   if ((pcan->cd.uc[3] & p->mydrumbit) == 0) return;

   /* Applicable to us? */
   if (!(((pcan->cd.uc[2] & 0x7) == 0) ||
         ((pcan->cd.uc[2] & 0x7) == p->mydrum))) return;

   /* Mode command from CAN msg. */
   p->mode = pcp->mode;


   // Here either it is intended for all drums, or just us.

/* ==== Lifted from levelwind_items_clupdate ========================= */
 /* Configure TIM2CH3 to be either input capture from encoder, or output compare (no pin). */
   // Each ZTBIT pushbutton press toggles beween IC and OC modes
   p->ocicbit = pcp->zero_tension; // Zero Tension Pushbutton switch bit
   if (p->ocicbit != p->ocicbit_prev)
   { // Here, the bit changed
      p->ocicbit_prev = p->ocicbit;
      if (p->ocicbit != 0)
      { // Here. Zero Tension bit went from Off to On
         pT2base->DIER &= ~0x80; // Disable TIM2CH3 interrupt
         if ((pT2base->CCMR2 & 0x1) != 0)
         { // Here, currently using encoder input capture
            // Setup for output c ompare
            pT2base->CCER  |=  (1 << 11);    // CC3NP: Configure as output
            pT2base->CCER  &= ~(1 << 8);     // CC3E = 0; Turn channel off
            pT2base->CCMR2 &= ~(0xff << 0);  // Change to Output compare, no pin
            pT2base->CCR3 = pT2base->CNT + p->ocfauxinc; // Schedule next faux encoder interrupt
         }
         else
         { // Here, currently using output compare
            // Setup for input capture
            pT2base->CCER  &= ~((1 << 8) || (1 << 11));  // CC3E, CC3NP = input
            pT2base->CCMR2 |= 0x01;       // Input capture mapped to TI3
            pT2base->SR = ~(1 << 3);      // Reset CH3 flag if on
            pT2base->CCER  |= (1 << 8);   // Capture enabled on pin.
         }
         pT2base->DIER |= 0x80; // Enable TIM2CH3 interrupt
      }
   }  
#endif   

   return;
 }

/* *************************************************************************
 * void levelwind_items_clupdate(struct CANRCVBUF* pcan);
 * @param   : pcan = pointer to CAN msg struct
 * @brief   : CAN msg rcv: cid_drum_tst_stepcmd
 * *************************************************************************/
void levelwind_items_clupdate(struct CANRCVBUF* pcan)
{
#ifndef USECPSWSMSGS  // Use CP switches and CL   
  return;   
#endif   
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer

   /* Reset loss of CL CAN msgs timeout counter. */
   p->cltimectr = 0; 

   /* Extract float from payload */
   p->pf.u8[0] = pcan->cd.uc[1];
   p->pf.u8[1] = pcan->cd.uc[2];
   p->pf.u8[2] = pcan->cd.uc[3];
   p->pf.u8[3] = pcan->cd.uc[4];

   p->clpos = p->pf.f; // Redundant???
   p->pay0 = pcan->cd.uc[0];

   /* Convert CL position (0.0 - 100.0) to output comnpare duration increment. */
#define MAXDURF (84E5f) // 1/10sec per faux encoder interrupt max duration
   p->focdur = (p->lc.clfactor / p->clpos);
   if ( p->focdur > (MAXDURF))
   { 
      p->focdur = MAXDURF; // Hold at max
   }
   p->ocfauxinc = p->focdur;   // Convert to integer

  /* Configure TIM2CH3 to be either input capture from encoder, or output compare (no pin). */
   // Each ZTBIT pushbutton press toggles beween IC and OC modes
   p->ocicbit = (pcan->cd.uc[0] & ZTBIT);
   if (p->ocicbit != p->ocicbit_prev)
   { // Here, the PREP??? bit changed
      p->ocicbit_prev = p->ocicbit;
      if (p->ocicbit != 0)
      { // Here. PREP??? bit went from off to on
         pT2base->DIER &= ~0x80; // Disable TIM2CH3 interrupt
         if ((pT2base->CCMR2 & 0x1) != 0)
         { // Here, currently using encoder input capture
            // Setup for output c ompare
            pT2base->CCER  |=  (1 << 11);    // CC3NP: Configure as output
            pT2base->CCER  &= ~(1 << 8);     // CC3E = 0; Turn channel off
            pT2base->CCMR2 &= ~(0xff << 0);  // Change to Output compare, no pin
            pT2base->CCR3 = pT2base->CNT + p->ocfauxinc; // Schedule next faux encoder interrupt
         }
         else
         { // Here, currently using output compare
            // Setup for input capture
            pT2base->CCER  &= ~((1 << 8) || (1 << 11));  // CC3E, CC3NP = input
            pT2base->CCMR2 |= 0x01;       // Input capture mapped to TI3
            pT2base->SR = ~(1 << 3);      // Reset CH3 flag if on
            pT2base->CCER  |= (1 << 8);   // Capture enabled on pin.
         }
         pT2base->DIER |= 0x80; // Enable TIM2CH3 interrupt
      }
   }
   /* Payload byte bits for direction and enable. */

   /* Direction bit of output compare (CL control) comes from
        CAN msg. 
      Direction bit for Input capture (encoder control) comes from
        TIM5 CR1 bit DIR which is setup in the ISR. */
   if ((pT2base->CCMR2 & 0x1) == 0) // Which mode?
   { // Here TIM2CH3 mode is output compare. Use CAN payload bit
         // Output capture (no pin) is TIM2CH3 mode
      if ((pcan->cd.uc[0] & DRBIT) == 0)  
         p->drflag = (Stepper_DR_Pin << 16); // Reset
      else  
      p->drflag = Stepper_DR_Pin;            // Set
   }

   // Motor Enable bit
   if ((pcan->cd.uc[0] & ENBIT) != 0)
   {  // enable stepper
      p->enflag = (Stepper_MF_Pin << 16); // Reset
   }
   else
   {  // disable stepper
      p->enflag = Stepper_MF_Pin; // Set
   }

   /* iobits does not seem to be actually used anywhere  */
   /* Bits positioned for updating PB BSRR register. */
   p->iobits = p->drflag | p->enflag;
   return;  
}

/*#######################################################################################
 * ISR routine for TIM2
 * CH1 - OC timed interrupts  indexing interrupts
 * CH2 - OC timed interrupts  or, FreeRTOS task forces this interrupt?
 * CH3 - IC encoder channel A or, OC generates faux encoder interrupts
 * CH4 - IC encoder channel B not used in this version
 *####################################################################################### */
void levelwind_items_TIM2_IRQHandler(void)
{
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
   /* This block for z channel (index) processing. It will be removed in operational
      code. */

   // TIM2CH2 = encodertimeZ
#if LEVELWINDDEBUG   
   if ((pT2base->SR & (1 << 2)) != 0) // CH2 Interrupt flag?
   { // Yes, encoder channel Z transition
      pT2base->SR = ~(1 << 2);  // Reset CH2 flag

      /* uncomment this overloaded use of orange LED to show index position is desired
      if ((GPIOB->IDR & (1 << 3)) == 0)
      {
         HAL_GPIO_WritePin(GPIOD,LED_ORANGE_Pin,GPIO_PIN_SET);
      }
      else
      {
         HAL_GPIO_WritePin(GPIOD,LED_ORANGE_Pin,GPIO_PIN_RESET);
      }
      */
      return;
   }
#endif

#if DTW
   // Capture DTW timer for cycle counting
   p->dtwentry = DTWTIME;
#endif 

   /* If we get here, we must have a TIM2 interrupt but it could be from channel 1
   or channel 3 or both. We service both in a single pass if needed. If the reversing 
   screw eumulation should be run, its switch block sets emulation_run to 1.*/   
   
   uint8_t  emulation_run = 0;   

   /* TIM2CH3 = encodertimeA PA2 TIM5CH1 PA0  */
   if ((pT2base->SR & (1 << 3)) != 0)  // CH3 Interrupt flag?
   { // Yes, either encoder channel A, or output compare emulating an encoder edge
      uint8_t  ddir; // REVIST: temporary drum direction while we are using faux interrupts 

      pT2base->SR = ~(1 << 3);   // Reset CH3 flag

      /* Was this interrupt due to encoder input capture or output compare?. */
      if ((pT2base->CCMR2 & 0x1) == 0)
      { // Here we are using TIM2CH3 as OC compare instead of input capture. */
         // Duration increment computed from CL CAN msg
         pT2base->CCR3 += p->ocfauxinc; // Schedule next faux encoder interrupt
         // direction for faux encoder
         ddir = (p->drflag == 1) ? 1 : 0; // set drum direction from CP
      }      
      else
      { // Here, encoder driven input capture. Save for odometer and speed
         ddir = (pT5base->CR1 & 0x10) ? 1 : 0;  // Encoder direction(0|1)
      }

      /* Here: either encoder channel A driven input capture interrupt, or 
         CL controlled timer output compare interrupt. */

      /* These encoder input capture or output compare interrupts do not drive the levelwind
         during regular indexing, sweeping, arrest, and off states,  */
      switch (p->isr_state & 0xF0)   // deal with interrupts based on lw isr_state msn
      {
         case (LW_ISR_TRACK):
         {
            /* code here to check if LOS has occured. if so, switch to LOS recovery
               state. */
            emulation_run = 1;
            p->drbit = ddir;
            break;
         }

         case (LW_ISR_LOS):
         {            
            // code here looking for limit switch clousure to re-index on
            // then switch back to tracking state 
            emulation_run = 1;
            p->drbit = ddir;
            break;
         }
      }
   }

   // Tim2 OC interrupt processing
   if ((pT2base->SR & (1 << 1)) != 0) // CH1 Interrupt flag
   { // Yes, OC drive 
      pT2base->SR = ~(1 << 1);  // Reset CH1 flag

      // Duration increment computed from CL CAN msg (during development)
      pT2base->CCR1 += p->ocinc; // Schedule next indexing interrupt
     
      switch (p->isr_state & 0xF0)   // deal with oc compare interrupts based on lw isr_state
      {
         case (LW_ISR_MANUAL):
         {            
            if (!(GPIOE->IDR & ManualSw_MSN_NO_Pin))   // REVISIT: test the left/right switch for left
            {  // switch is signaling left
               Stepper_DR_GPIO_Port->BSRR = L0R_LEFT;  // set direction left
               // Start TIM9 to generate a delayed pulse.
               pT9base->CR1 = 0x9; 
            }
            else if (!(GPIOE->IDR & ManualSw_MS_NO_Pin))   // REVIST: test the left/right switch for right
            {  // switch is signaling right
               Stepper_DR_GPIO_Port->BSRR = L0R_RIGHT;  // set direction right
               // Start TIM9 to generate a delayed pulse.
               pT9base->CR1 = 0x9; 
            }
            //  if neither is activated, do nothing
            break;
         }

         case (LW_ISR_INDEX):
         {
            switch (p->indexphase)
            {
               case (0):
               {  // moving towards motor (position accumulator is decreasing)               
                  if ((p->sw[LIMITDBMSN].dbs == 0) && (p->velaccum.s32 == -p->Ks))
                  {  // MSN switch has deactivated and at full speed towards stepper motor
                     /*
                        Reset position accumulator to initiate a reversal to start
                        moving back towards MSN limit switch. It will then index
                        when it the MSN reactivates and will have enough distance
                        to reach nominal speed.
                     */
                     p->posaccum.s32 = p->Lminus32;
                     p->pos_prev = p->posaccum.s16[1];
                     p->indexphase = 1;
                     break;
                  }

                  // REVISIT: test test and action here for posaccum <= Lminus32
                  if (0)
                  {

                  }
                  break; 
               }
               case (1): 
               {  // moving away from motor looking for MSN activation 
                  if (p->sw[LIMITDBMSN].dbs != 0)
                  {  // MSN switch has now activated
                     p->posaccum.s32 = p->Lpos;
                     p->pos_prev = p->posaccum.s16[1];

                     /* temporary until  indexing phase 2 is implemented
                     // switch to sweep state
                     p->Lminus32 = p->Lneg;
                     p->isr_state = LW_ISR_SWEEP; // move to sweep ISR state
                     */
#if LEVELWINDDEBUG //   for development only
                     p->tim5cnt_offset = -pT5base->CNT; // reset odometer to 0 for testing only
#endif            
                     p->indexphase = 2;
                     break;
                  }
                  
                  // REVISIT: test and action here  here for posaccum >= Lplus32
                  if (0)
                  {

                  }
                  break;
               }
               case (2):
               {  // moving towards motor looking for MS activation
                  if (p->sw[LIMITDBMS].dbs != 0)
                  {  // MS switch has activated
                     // center position accumulator on limit switches' activation points
                     p->Ws = p->Lpos - p->posaccum.s32;  // capture measureed LS span
                     // REVIST: need test that span measured is within span tolerance and
                     // mechanism to act on failures
                     if (0)
                     {
                        break;
                     }

                     p->posaccum.s32 = -(((p->Ws + p->Ks) >> 1) / p->Ks) * p->Ks;
                     p->pos_prev = p->posaccum.s16[1];
                     p->Lminus32 = p->Lneg;
                     p->isr_state = LW_ISR_SWEEP;
                     break;  
                  }
                  
                  // REVISIT: test and action here if posaccum32 <= Lminus32
                  if (0)
                  {

                  }
                  break;
               }
            }
            emulation_run = 1;
            break;
         }         

         case (LW_ISR_SWEEP):
         {  
            
            /* REVIST: code here, or in limit switch interrupt handler, for testing limit 
            switches in operational code and characterizing their behavior 
            during development. If needed, this mode can be used for
            calibrating limit switches for speed dependent corrections in LOS recovery. */

            // exit to arrest when velocity goes through 0
            
            if (p->velaccum.s32 == 0)  // when velocity == 0, speed up
            {
               p->ocinc = p->ocswp;  // speed up interrupt rate for test sweep
            }

            // temporary until termination criteria is established
            if (!(GPIOE->IDR & ManualSw_MS_NO_Pin)) 
               // transition to Arrest with next state Track 
               p->isr_state = LW_ISR_ARREST | (LW_ISR_TRACK >> 4);             
            
            emulation_run = 1;
            break;
         }

         case (LW_ISR_ARREST):
         {  // code here dealing with stopping next time velocity reaches 0  
            if (p->velaccum.s32 == 0)
            {
               /* Transition to Track or Center when done. The state to transtion 
               to is held in the lower nibble of isr_state */
               p->isr_state = p->isr_state << 4;
               p->ocinc = p->ocman; // reduce output compare interrupt rate (recues processor load)
            }
            else  emulation_run = 1;
            break;
         }
      }      
   }  

   /* reversing screw emulation code */
   if (emulation_run)
   {  // forward (levelwind) direction means position accumulator is increasing
      // negative direction means position accumulator is decreasing
      // drbit = 0 means positive drum direction

      // update velocity integrator         
      if (p->drbit != p->drbit_prev)   
      {  // Drum direction has changed
         p->drbit_prev = p->drbit;   // save new direction
         p->velaccum.s32 = -p->velaccum.s32; // invert velocity value
      }
      else if (p->posaccum.s32 >= p->Lplus32)   // in positive level-wind region ?
      {
         p->velaccum.s32 -= p->Ka;  // apply negative acceleration 
      }      
      else if (p->posaccum.s32 <= p->Lminus32)  // in negative level-wind region ?
      {
         p->velaccum.s32 += p->Ka;  // apply positive acceleration
      }
      
      // update position integrator
      p->posaccum.s32 += p->velaccum.s32;

#if LEVELWINDDEBUG    
   p->pdbgadd->tim5cnt = p->tim5cnt_offset + pT5base->CNT;
   p->intcntr++;         
   /* p->dbg1 = p->velaccum.s32;
      p->dbg2 = p->posaccum.s16[1];
      p->dbg3 = p->posaccum.u16[0]; */
#if 1
   // Store vars in buffer location. 
   p->pdbgadd->intcntr   = p->intcntr;
   p->pdbgadd->dbg1      = p->velaccum.s32;
   p->pdbgadd->dbg2      = p->posaccum.s16[1];
   p->pdbgadd->dbg3      = p->posaccum.u16[0];
   /*p->pdbgadd->dbg1      = p->Ka;
   p->pdbgadd->dbg2      = p->Nr;
   p->pdbgadd->dbg3      = p->Ks;*/
   p->pdbgadd += 1;    // Advance add pointer
   if (p->pdbgadd >= p->pdbgend) p->pdbgadd = p->pdbgbegin;
#else
   // temporary for deguging state machine
   p->pdbgadd->intcntr   = p->intcntr;
   p->pdbgadd->dbg1      = p->velaccum.s32;
   p->pdbgadd->dbg2      = p->state;
   p->pdbgadd->dbg3      = p->isr_state;
   p->pdbgadd += 1;    // Advance add pointer
   if (p->pdbgadd >= p->pdbgend) p->pdbgadd = p->pdbgbegin;
#endif

#endif
         
      /* When accumulator upper 16b changes generate a levelwind pulse. */
      if ((p->posaccum.s16[1]) != (p->pos_prev))
      { // Here carry/borrow from low 16b to high 16b
         p->pos_prev = p->posaccum.s16[1];

         // set direction based on sign of Velocity integrator
         // veclocity integrator upper 16 bits are all sign extensions
         // logical exclusive or of direction and stepper direction indicator
         Stepper_DR_GPIO_Port->BSRR = ((p->velaccum.s16[1] != 0) != p->stpprdiri)
            ? L0R_RIGHT : L0R_LEFT;

         // Start TIM9 to generate a delayed pulse.
         pT9base->CR1 = 0x9;         
      }
   }

#if LEVELWINDDEBUG
   p->ledctr1 += 1;
   if ((pT2base->CCMR2 & 0x1) != 0)
   {
      p->ledctr2 = 0;
   }   
   else
   {
      p->ledctr2 = 500;
   }

   if (p->ledctr1 > p->ledctr2)        
   {
      p->ledctr1 = 0;
      if ((GPIOA->IDR & (1 << 0)) == 0)
      {
         HAL_GPIO_WritePin(GPIOD,LED_GREEN_Pin,GPIO_PIN_SET); // GREEN LED       
      }
      else
      {
         HAL_GPIO_WritePin(GPIOD,LED_GREEN_Pin,GPIO_PIN_RESET); // GREEN LED    
      }  
   }
#endif

#if DTW
   p->dtwdiff = DTWTIME - p->dtwentry;
   if (p->dtwdiff > p->dtwmax) p->dtwmax = p->dtwdiff;
   else if (p->dtwdiff < p->dtwmin) p->dtwmin = p->dtwdiff;
#endif

   /* Notify LevelwindTask.c when the state, or status has changed. */
   if ((p->isr_state != p->isr_state_prev) || (p->status != p->status_prev))
   {
      p->isr_state_prev = p->isr_state;
      p->status_prev    = p->status;

      NVIC_SetPendingIRQ(ETH_IRQn); // Trigger intermediate interrupt
   }

   return;
}

/* *************************************************************************
 * struct LEVELWINDDBGBUF* levelwind_items_getdbg(void);
 * @brief   : Get pointer to debug buffer
 * @return  : NULL = no new data; otherwise ptr struct with data
 * *************************************************************************/
#if LEVELWINDDEBUG 
struct LEVELWINDDBGBUF* levelwind_items_getdbg(void)
{
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer
   struct LEVELWINDDBGBUF* ptmp;
   if (p->pdbgadd == p->pdbgtake) return NULL;
   ptmp = p->pdbgtake;
   p->pdbgtake += 1;
   if (p->pdbgtake >= p->pdbgend) p->pdbgtake = p->pdbgbegin;
   return ptmp;
}
#endif