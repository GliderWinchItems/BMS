/******************************************************************************
* File Name          : levelwind_items.h
* Date First Issued  : 09/23/2020
* Description        : Levelwind levelwind motor algorithm and items
*******************************************************************************/
/*
09/10/2020 realfaux branch 
08/29/2020 fauxencoder
*/

#ifndef __LEVELWIND_ITEMS
#define __LEVELWIND_ITEMS

/* Debug */
#define LEVELWINDDEBUG  1  // True includes debugging code
#define LEVELWINDDBGBUFSIZE (360*4) // Circular buffer size

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "levelwind_items.h"
#include "common_can.h"
#include "CanTask.h"
#include "levelwind_switches.h"
#include "LevelwindTask.h"

#define TIM3CNTRATE 84000000   // TIM3 counter rate (Hz)
#define UPDATERATE 100000      // 100KHz interrupt/update rate
#define TIM3DUR  (TIM3CNTRATE/UPDATERATE) // 1680 counts per interrupt

#define TIM9CNTRATE 168000000 // TIM9 counter rate (Hz)
#define TIM9PWMCYCLE (168*10-30)   // 10us pwm cycle
#define TIM9PULSEDELAY (TIM9PWMCYCLE - (168*3))

/* Port and pin numbers for levelwind controller. */
#define PU_port  GPIOA      // Pulse
#define PU_pin   GPIO_PIN_5 // Pulse
#define DR_port  GPIOB      // Direction
#define DR_pin   GPIO_PIN_0 // Direction
#define EN_port  GPIOB      // Enable
#define EN_pin   GPIO_PIN_1 // Enable
#define LMIN_port  GPIOE       // Limit switch inner
#define LMIN_pin   GPIO_PIN_5  // Limit switch inner
#define LMOUT_port GPIOE       // Limit switch outside
#define LMOUT_pin  GPIO_PIN_10 // Limit switch outside

/* CAN msg: cid_drum_tst_stepcmd: payload[0] bit definitions. */
#define DRBIT 0x01 // (1) Bit mask Direction output pin: 0 = low; 1 = high
#define ENBIT 0x02 // (2) Bit mask Enable output pin: 0 = low; 1 = high
#define LMBIT 0x04 // (3) Bit mask Limit switch simulation
#define IXBIT 0x08 // (4) Bit mask Indexing command
#define ZTBIT 0x10 // (5) Bit mask PB State: Zero Tension
#define ZOBIT 0x20 // (6) Bit mask PB State: Zero Odometer
#define ARBIT 0x40 // (7) Bit mask PB State: ARM
#define PRBIT 0x80 // (8) Bit Mask PB State: PREP
/* Notes of above bit usage--
(1) CP PB processed: Zero Odometer TOGGLES direction minus sign on LCD
(2) CP SAFE/ACTIVE: Bit sets when in CP goes into ARM state
(3) CP PB: Zero Tension PB state simulates limit switch
(4) CP PB: ARM PB state simulates CP begin indexing command
(5) CP PB state: Zero Tension (CP toggles direction)
(6) CP PB state: Zero Odometer
(7) CP PB state: ARM
(8) CP PB state: Prep (CP toggles freeze of CL setting)
*/

// LW ISR mode definitions. Lower nibble reserved for sub-states
#define LW_ISR_OFF    (0 << 4)
#define LW_ISR_MANUAL (1 << 4)
#define LW_ISR_INDEX  (2 << 4)            // initial indexing state
#define LW_ISR_INDEX1 (LW_ISR_INDEX | 1)  // second indexing state
#define LW_ISR_SWEEP  (3 << 4)
#define LW_ISR_ARREST (4 << 4)
#define LW_ISR_TRACK  (5 << 4)
#define LW_ISR_LOS    (6 << 4)

// Left/0/Right switch direction definitions 
/* REVIST: Should these be tied to something else tthat deals with which
   side the drum is mounted on   */ 
#define L0R_LEFT  Stepper_DR_Pin
#define L0R_RIGHT (Stepper_DR_Pin << 16) 

struct LEVELWINDDBGBUF
{
   uint32_t intcntr;    // interrupt counter
   int32_t  dbg1;       // Debug 1
   int32_t  dbg2;       // Debug 2
   int32_t  dbg3;       // Debug 3
   uint32_t tim5cnt;    // Encoder count
};

/* *************************************************************************/
 void levelwind_items_clupdate(struct CANRCVBUF* pcan);
/* @param   : pcan = pointer to CAN msg struct
 * @brief   : Initialization of channel increment
 * *************************************************************************/
 void levelwind_items_timeout(void);
/* @brief   : Check for loss of CL CAN msgs
 * *************************************************************************/
 void levelwind_items_CANsend_hb_levelwind_1(void);
/* @brief   : Send CAN heartbeat for levelwind
 * *************************************************************************/
 void levelwind_items_rcv_cid_hb_cpswsv1_1(struct CANRCVBUF* pcan);
/* @param   : pcan = pointer to CAN msg struct
 * @brief   : CAN msg rcv: 
 * *************************************************************************/
void levelwind_items_rcv_cid_hb_cpswsclv1_1(struct CANRCVBUF* pcan);
/* @param   : pcan = pointer to CAN msg struct
 * @brief   : CAN msg rcv: cid_hb_cpswsclv1_1 (control lever position)
 * *************************************************************************/
  struct LEVELWINDDBGBUF* levelwind_items_getdbg(void);
/* @brief   : Get pointer to debug buffer
 * @return  : NULL = no new data; otherwise ptr struct with data
 * *************************************************************************/

 extern struct LEVELWINDSTUFF levelwindstuff;

#if LEVELWINDDEBUG 
 extern struct LEVELWINDDBGBUF levelwinddbgbuf[LEVELWINDDBGBUFSIZE];
#endif

#endif
