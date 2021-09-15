/******************************************************************************
* File Name          : ControlPanelState.h
* Date First Issued  : 10/10/2020
* Description        : Defines a data structure and payload description
*                      for the control panel state message
*******************************************************************************/

#ifndef __CONTROLPANELITEMS
#define __CONTROLPANELITEMS

#include <stdint.h>

/* Definition of payload values  */

// CL position payload location and scaling factor

// payload byte 0
#define CLPOS_BYTE            0  // u16 control lever position
#define CLPOS_SCL          0.5f  // each lsb equal 0.5%


// switch and pb inputs payload locations

// number of bits in the mask is width of the field
// ...BIT is the lsb in the ...BYTE

// payload[1]
#define SAFEACTIVE_BYTE    1  // Safe/Active switch
#define SAFEACTIVE_BIT     7
#define SAFEACTIVE_MASK    0x01

#define ARMED_BYTE         1  // Arm PB
#define ARMED_BIT          6
#define ARMED_MASK         0x01

#define RTRVPREP_BYTE      1  // Prep/Retrieve PB
#define RTRVPREP_BIT       5
#define RTRVPREP_MASK      0x01

#define ZEROTEN_BYTE       1  // Zero Tension PB
#define ZEROTEN_BIT        4
#define ZEROTEN_MASK       0x01

#define ZEROODOM_BYTE      1  // Zero Odometer PB
#define ZEROODOM_BIT       3
#define ZEROODOM_MASK      0x01

#define BRAKE_BYTE         1  // Brake (guarded) switch
#define BRAKE_BIT          2
#define BRAKE_MASK         0x01

#define GUILLOTINE_BYTE    1  // Guillotine (guarded) switch
#define GUILLOTINE_BIT     1
#define GUILLOTINE_MASK    0x01

#define EMERGENCY_BYTE     1  // Emergency (latching) PB   aka BIG RED BUTTON
#define EMERGENCY_BIT      0
#define EMERGENCY_MASK     0x01

// payload[2]
#define LWMODE_BYTE        2  // Level-Wind Mode switch (3 position) 
#define LWMODE_BIT         6
#define LWMODE_MASK        0x03  // 2 bit field

#define LWINDEX_BYTE       2  // optional Level-wind Index 
#define LWINDEX_BIT        5
#define LWINDEX_MASK       0x01

#define REVFWD_BYTE        2  // optional drum direction PB
#define REVFWD_BIT         4
#define REVFWD_MASK        0x01

#define RMTLCL_BYTE        2  // Remote/Local switch
#define RMTLCL_BIT         3
#define RMTLCL_MASK        0x01

#define ACTIVEDRUM_BYTE    2  // active drum rotary switch  
#define ACTIVEDRUM_BIT     0   
#define ACTIVEDRUM_MASK    0x07  // 3 bit field



// payload[3]
#define SPARE3_BYTE       3
#define SPARE3_BIT        7
#define SPARE3_MASK       0x01

#define OPDRUMS_BYTE       3  // array of spst switches, one for each drum         
#define OPDRUMS_BIT        0    
#define OPDRUMS_MASK       0x7F  // 7 bit field

// bit 0 is spare


/* outputs  */
// fill from right

// payload byte 7

#define SAFELED_BYTE       7
#define SAFELED_BIT        0
#define SAFELED_MASK       0x01

#define PREPLED_BYTE       7
#define PREPLED_BIT        1
#define PREPLED_MASK       0x01

#define ARMEDLED_BYTE      7
#define ARMEDLED_BIT       2
#define ARMEDLED_MASK      0x01

#define GRNDRTNLED_BYTE    7
#define GRNDRTNLED_BIT     3
#define GRNDRTNLED_MASK    0x01

#define RAMPLED_BYTE       7
#define RAMPLED_BIT        4
#define RAMPLED_MASK       0x1

#define CLIMBLED_BYTE      7
#define CLIMBLED_BIT       5
#define CLIMBLED_MASK      0x01

#define RECOVERYLED_BYTE   7
#define RECOVERYLED_BIT    6
#define RECOVERYLED_MASK   0x01

#define RETRIEVELED_BYTE   7
#define RETRIEVELED_BIT    7
#define RETRIEVELED_MASK   0x01

// payload byte 6

#define ABORTLED_BYTE      6
#define ABORTLED_BIT       0
#define ABORTLED_MASK      0x01

#define STOPLED_BYTE       6
#define STOPLED_BIT        1
#define STOPLED_MASK       0x01

#define ARMPBLED_BYTE      6
#define ARMPBLED_BIT       2
#define ARMPBLED_MASK      0x01

#define PREPRCRYPBLED_BYTE 6
#define PREPRCRYPBLED_BIT  3
#define PREPRCRYPBLED_MASK 0x01

/* Beeper likely to get its own CAN message
#define BEEPER_BYTE        6
#define BEEPER_BIT         4 
#define BEEPER_MASK        0x01  */



struct CONTROLPANELSTATE
{
   uint8_t init; // struct is initialized:1, updated:0 from CAN message   

   /* control lever  */
   float    clpos;   // control lever position (0.0 to 100.0 percent)

   /* input signals  */
   // general assumption is all are debounced by control panel function 
   // (if needed)
   uint8_t  safe_active;   // safe:0, active:1
   uint8_t  arm;         
   uint8_t  rtrv_prep;     // retrieve:0, prep: 1
   uint8_t  zero_tension;
   uint8_t  zero_odometer;
   uint8_t  brake;
   uint8_t  guillotine;
   uint8_t  emergency;  // normal:0, emergency:1
   uint8_t  mode;       // off, track center
   uint8_t  index;      // index the level-wind   

   // provisions for future expansions
   uint8_t  rev_fwd;       // reverse:0, forward:1 control lever action
   uint8_t  rmt_lcl;       // remote:0, local:1 operator control
   uint8_t  active_drum;   // 1-7, 0 might signify all if needed
   uint8_t  op_drums;      // operational drums, bit mapped
   

   /* output signals   */
   // state leds
   uint8_t  safe_led;
   uint8_t  prep_led;
   uint8_t  armed_led;
   uint8_t  grndrtn_led;
   uint8_t  ramp_led;
   uint8_t  climb_led;
   uint8_t  recovery_led;
   uint8_t  retrieve_led;
   uint8_t  abort_led;
   uint8_t  stop_led;

   // lighted push button leds
   uint8_t  arm_pb_led;
   uint8_t  prep_rcvry_led;

   // uint8_t  beeper;  // likely getting its own message
};

#endif


