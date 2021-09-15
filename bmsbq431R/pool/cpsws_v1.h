/******************************************************************************
* File Name          : cpsws_v1.h
* Date First Issued  : 10/25/2020
* Description        : Payload: Control Panel Switches and Control Lever
*                    : Version 1
*******************************************************************************/

#ifndef __CPSWS_V1
#define __CPSWS_V1

#include <stdint.h>

/* Definition of payload values  */

// DLC 8
// Payload type: U8_U8_U8_U8_FF

// payload[0] U8
#define SWSAFEACTIVE 0x80 // Toggle: 0 = SAFE; 1 = ACTIVE
#define SWARMED      0x40 // 1 = PB: Arm
#define SWRTRVPREP   0x20 // 1 = PB: Retrieve
#define SWZEROTEN    0x10 // 1 = PB: Zero Tension
#define SWZEROODOM   0x80 // 1 = PB: Zero Odometer
#define SWBRAKE      0x40 // 1 = PB: Apply Brake
#define SWGUILLOTINE 0x20 // 1 = Hooded PB: Actuate guillotine
#define SWEMERGENCY  0x10 // 1 = Big red: Emergency 


// payload[1] U8
#define SWLWMODE         0xC0 // LW: 00=OFF, 01=CENTER, 10=TRACK
#define SWLWINDEX        0x20 // optional index
#define SWREVFWD         0x10 // optional direction
#define SWRMTLCL         0x08 // 1 = Remote control; 0 = local
#define SWACTIVEDRUM_NUM 0x07 // Selector sw: Active drum (1 - 7)

// payload[2] U8
/* Toggle switches for each drum position to signal availability. */
// Bit position signals: available = 1; not available = 0;
#define OPDRUMS_BIT      0x7F // Seven drums: drum#1: bit 0

// payload[3] U8    Reserve for switch expansion

// payload[4-7] FF
  // Control Lever position: float: (0 - 100.0)

#endif


