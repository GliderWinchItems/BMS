/******************************************************************************
* File Name          : LevelwindTask.h
* Date First Issued  : 09/15/2020
* Description        : Levelwind function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __LEVELWINDTASK
#define __LEVELWINDTASK

#include <stdint.h>
#include <math.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "CanTask.h"
#include "levelwind_switches.h"
#include "levelwind_idx_v_struct.h"
#include "levelwind_items.h"
#include "controlpanel_items.h"
#include "mastercontroller_states.h"

/* =========== switch between old and new CAN msgs ====== */
#define USECPSWSMSGS  // Use CP switches and CL



/* Stepper switch bit positions */
#define LEVELWINDSWSNOTEBITLIMINDB   (1<<LIMITDBINSIDE)  
#define LEVELWINDSWSNOTEBITLIMOUTDB  (1<<LIMITDBOUTSIDE) 
#define LEVELWINDSWSNOTEBITLIMINNC   (LIMITINSIDENC) 
#define LEVELWINDSWSNOTEBITLIMINNO   (LIMITINSIDENO)
#define LEVELWINDSWSNOTEBITLIMOUTNC  (LIMITOUTSIDENC)
#define LEVELWINDSWSNOTEBITLIMOUTNO  (LIMITOUTSIDENO)
#define LEVELWINDSWSNOTEBITLIMINOVR  (OVERRUNSWINSIDE)
#define LEVELWINDSWSNOTEBITLIMOUTOVR (OVERRUNSWOUTSIDE)
#define LEVELWINDSWSNOTEBITISR       (1<<16)    // Stepper ISR
#define LEVELWINDSWSNOTEBITCAN1      (1<<17)    // CAN msg: cid_drum_tst_stepcmd; CANID_TST_STEPCMD: U8_FF DRUM1: U8:
#define LEVELWINDSWSNOTEBITCAN2      (1<<19)    // CAN msg: cid_mc_state;         CANID_MC_STATE','26000000', 'MC', 'U8_U8'
#define LEVELWINDSWSNOTEBITCAN3      (1<<20)    // CAN msg: cid_hb_cpswsv1_1;     CANID_HB_CPSWSV1_1'  ,'31000000''CPMC', 1,1,'S8_U8_7'
#define LEVELWINDSWSNOTEBITCAN4      (1<<21)    // CAN msg: cid_hb_cpswsclv1_1;   CANID_HB_CPSWSCLV1_1','31800000','CPMC', 2,1,'S8_S16_FF_V'
#define LEVELWINDSWSNOTEBITCAN5      (1<<22)    // CAN msg: cid_cmd_levelwind_i1; CANID_CMD_LEVELWIND_I1','B1000014','GENCMD',1,23,'U8_U8_U8_X4'


/* Port and pin numbers for stepper controller. */
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

// LW state machine  definitions. Lower nibble reserved for sub-states
#define LW_OFF       (0 << 4)
#define LW_OVERRUN   (1 << 4)
#define LW_MANUAL    (2 << 4)
#define LW_CENTER    (3 << 4)
#define LW_INDEX     (4 << 4)
#define LW_TRACK     (5 << 4)
#define LW_LOS       (6 << 4)

/* LW Modes */
#define LW_MODE_OFF     0x03  // Level-wind off. Also cycled through to reset error
#define LW_MODE_TRACK   0x01  // track during all operational states
#define LW_MODE_CENTER  0x02  // center during retrieve

/* Level-Wind Status Definitions */
// these will be expanded to capture reasons for non-good status as
// development continues
// Green Status
#define  LW_STATUS_GOOD              0 
// Red Statii
#define  LW_STATUS_OFF_AFTER_ERROR  -1 
#define  LW_STATUS_OVERRUN          -2
#define  LW_STATUS_MANUAL           -3
#define  LW_STATUS_INDEX_FAILURE    -4
#define  LW_STATUS_SWEEP_FAILURE    -5
#define  LW_STATUS_LOS_FAILURE      -6
#define  LW_STATUS_STEPPER_POWER    -7
#define  LW_STATUS_NODE_POWER       -8
//Yellow Statii
#define  LW_STATUS_INDEXING          1
#define  LW_STATUS_SWEEPING          2 
#define  LW_STATUS_ARRESTING         3
#define  LW_STATUS_LOS_REINDEXING    4
#define  LW_STATUS_LOS_TRACKING      5


#define NUMCANMSGSLEVELWIND 2  // Number of CAN msgs levelwind sends

enum cididx // Index for CAN msgs we send array
{
   IDX_CID_CMD_LEVELWIND_R1,
   IDX_CID_HB_LEVELWIND_1
};

union PAYFLT
{
   float         f;
   uint8_t   u8[4];
   int8_t    s8[4];
   uint16_t u16[2];
   int16_t  s16[2];
   uint32_t    u32;
   int32_t     s32;
   
};

struct LEVELWINDFUNCTION
{
   struct   LEVELWINDLC lc; // external parameters for level-wind function
   
   // union variables
   union    PAYFLT   pf; // For extracting float from payload
   union    PAYFLT   posaccum;  // Stepper position accumulator
   union    PAYFLT   velaccum;  // Stepper velocity accumulator

   int16_t  pos_prev;   // Previous position accumulator integral portion
   
   // these are the working reversal points 
   int32_t  Lplus32;    // 32-bit extended working Lplus
   int32_t  Lminus32;   // 32-bit extended workin Lminus
   
   // 32-bit parameters mostly based on SI parameters in lc above
   int32_t  Lpos;       // accumulator postive reversal point     
   int32_t  Lneg;       // accumulator negative reversal point
   int32_t  Windxswp;   // maximum indexing sweep width  
   uint32_t Ws;        // measured limit switch span (not based on parameters)
   uint32_t LSSpan;    // expected LS span
   uint32_t LSTol;     // allowable tolerance for LS span
   

   // Sweep and reversal parameters   
   uint16_t Nr;         // Reversal steps to 0 velocity  
   uint16_t Ka;         // internal accelertion parameter
   uint32_t Ks;         // (Ks/65536)=levelwind pulses per encoder edge (<= 1)        
   int32_t  rvrsl;      // Reversal Distance in LSBs
   
   
   uint32_t ocinc;      // OC register increment for current level-wind state
   uint32_t ocswp;      // OC register increment for sweeping
   uint32_t ocman;      // OC register increment for manual motion
   uint8_t  stpprdiri;  // stepper direction indicator (0 normal, 1 inverted)
   uint32_t hbctr;      // Count ticks for sending heartbeat CAN msg
   uint32_t hbct_k;     // RTOS ticks between Heartbeat messages
   uint32_t enflag;     // BSRR pin set/reset bit position: enable
   uint8_t  drbit;      // Drum direction bit (0, forward|1, reverse)
   uint8_t  drbit_prev; // Previous Direction bit
   uint8_t  indexphase; // indexing phase indicator
   uint8_t  mydrum;     // internal instance number
   uint8_t  mydrumbit;  // mydrum number converted to bit position 0:6
   
   // states and flags
   uint8_t  state;         // level-wind state
   uint8_t  state_prev;    // level-wind previous state
   uint8_t  status;        // level-wind status
   uint8_t  status_prev;   // level-wind previous status
   uint8_t  isr_state;     // level-wind ISR state
   uint8_t  isr_state_prev;// level-wind ISR state, previous
   uint8_t  mode;          // level-wind mode (Off, Track, or Center)
   uint8_t  indexed;       // REVISIT: indexed status MAY NOT BE NEEDED
   uint8_t  mc_state;      // master controller state 
   uint8_t  mc_state_sub;  // master controller sub-state

   
   uint8_t  ocicbit;       
   uint8_t  ocicbit_prev;  

// REVIST: May not need 6 instances for the below. Only 2 are now processed as interrupts
   struct   STEPPERSWCONTACT ctk[6];   // Measured switch contact open/close posaccum
   struct   EXTISWITCHSTATUS sw[6];    // Limit & overrun switches
   uint16_t swbits;                    // Port E switch bits (10:15)

   // debug and characterization, potentially removable for operational cod
   uint32_t cltimectr;  // Counter for loss of CL msgs
   uint32_t speedcmdi;  // Commanded speed (integer)
   float    speedcmdf;  // Speed command (float)
   float    clpos;      // CL position extracted from CAN msg
   float    focdur;     // Temp for computer inverse of CL position
   uint32_t iobits;     // Bits from CL CAN msg positioned for PB0
   uint32_t dtwentry;   // DTW timer upon ISR entry
   int32_t  dtwdiff;    // DTW timer minus entry upon ISR exit
   int32_t  dtwmax;     // DTW difference max
   int32_t  dtwmin;     // DTW difference min
   uint32_t intcntr;    // interrupt counter
   uint32_t ocfauxinc;  // OC register increment for CL faux encoder  
   uint8_t  pay0;       // canmsg.cd.uc[0] saved
   uint32_t drflag;     // BSRR pin set/reset bit position: direction
   uint8_t  cpmode;     // mode extracted from CAN msg cid_hb_cpswsv1_1

/* Pointer into circular buffer for levelwind_items.c debugging. */
#if LEVELWINDDEBUG
   struct LEVELWINDDBGBUF*  pdbgbegin;
   struct LEVELWINDDBGBUF*  pdbgadd;
   struct LEVELWINDDBGBUF*  pdbgtake;
   struct LEVELWINDDBGBUF*  pdbgend;

   uint32_t ledctr1;    // Counter for throttling green LED
   uint32_t ledctr2;    // Counter for throttling orangeLED
   uint32_t ledbit1;    // Bit for toggling green led
   uint32_t ledbit2;    // Bit for toggling orange led

   uint32_t tim5cnt_offset;
#endif

   /* Pointers to incoming CAN msg mailboxes. */
   struct MAILBOXCAN* pmbx_cid_gps_sync;        // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
   struct MAILBOXCAN* pmbx_cid_drum_tst_stepcmd;// CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000
   struct MAILBOXCAN* pmbx_cid_hb_cpswsv1_1;    // CANID_HB_CPSWSV1_1'  ,'31000000','CPMC', 1,1,'S8_U8_7','HB_CPSWSV1 1: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');
   struct MAILBOXCAN* pmbx_cid_hb_cpswsclv1_1;  // CANID_HB_CPSWSCLV1_1','31800000','CPMC', 2,1,'S8_S16_FF_V','HB_CPSWSV1 1:S8:status, S16 CL: (+/-10000 )');
   struct MAILBOXCAN* pmbx_cid_cmd_levelwind_i1;// CANID_CMD_LEVELWIND_I1','B1000014','GENCMD',1,23,'U8_U8_U8_X4','1 incoming: U8:drum bits,U8:command code,X4:four byte value');
   struct MAILBOXCAN* pmbx_cid_mc_state; //'CANID_MC_STATE','26000000', 'MC', 'UNDEF','MC: Launch state msg');


   /* CAN msgs */
   struct CANTXQMSG canmsg[NUMCANMSGSLEVELWIND];
};

/* *************************************************************************/
 osThreadId xLevelwindTaskCreate(uint32_t taskpriority);
/* @brief   : Create task; task handle created is global for all to enjoy!
 * @param   : taskpriority = Task priority (just as it says!)
 * @return  : LevelwindTaskHandle
 * *************************************************************************/

 extern osThreadId LevelwindTaskHandle;
 extern struct LEVELWINDFUNCTION levelwindfunction;

#endif

/* *************************************************************************/
int levelwind_task_cp_state_update(struct CANRCVBUF* pcan);
/* @brief   : update of control panel state structure from CPSWSV1 CAN msg
 * @arg     : pcan = pointer to CAN msg
 * @return  : 0 = OK, -1 = sws not ready
 * *************************************************************************/
 void levelwind_task_cp_state_init();
/* @brief   :  function to initialize the state struct before first CAN
 *             message
 * *************************************************************************/

 extern struct CONTROLPANELSTATE cp_state;

 
