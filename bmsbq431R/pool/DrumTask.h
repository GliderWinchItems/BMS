/******************************************************************************
* File Name          : DrumTask.h
* Date First Issued  : 09/15/2020
* Description        : Drum function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __DRUMTASK
#define __DRUMTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "CanTask.h"
#include "drum_items.h"

/* REQMAX - number of different poll rates.
   Saves "last count & time" in an array indexed
   on reqnum, so that slow polling has more time and 
   counts between speed computations and faster ones
   might be better for acceleration computations, etc.
*/
#define REQMAX 4 //

/* Input Capture time and encoder count. */
struct DRUMTIMCNT
{
   uint32_t tim; // Input capture time
    int32_t cnt; // Encoder count at capture time	
};

struct DRUMENCODERCH /* Drum Encoder Channel */
{
   struct DRUMTIMCNT cur;
   struct DRUMTIMCNT prev;
   struct DRUMTIMCNT diff;
   struct DRUMTIMCNT tmp;

};

struct DRUMLC
{
	float Kgearratio; // Encoder shaft:drum axle gear ratio
	float Kdrumdia;   // Nominal drum diameter (meters)
	float Kencodercountperrev;
};


struct DRUMFUNCTION
{
   struct DRUMLC lc;          // Parameters
   struct DRUMENCODERCH decA[REQMAX]; // Time & Count Encoder Channel A: even

//   struct DRUMENCODERCH decB; // Time & Count Encoder Channel B
//   struct DRUMENCODERCH decZ; // Time & Count Encoder Channel Z
   uint32_t encoder; // Latest encoder (needed?)
   uint32_t CR1; // Latest direction bit (needed?)

   int32_t cablezeroref;     // Encoder count for zero line out.

	float Fspeed_rpm_encoder; // Factor to convert encoder counts to rpm
   float Fspeed_cable;	      // Factor to convert encoder rpm to meters/sec
	float Fcable_distance;    // Factor to convert encoder count to meters

	float Cspeed_rpm_encoder; // Computed encoder counts to rpm
   float Cspeed_cable;	      // Computed encoder rpm to meters/sec
	float Ccable_distance;    // Computed encoder count to meters

};

/* *************************************************************************/
 osThreadId xDrumTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: DrumTaskHandle
 * *************************************************************************/

 extern osThreadId DrumTaskHandle;
 extern struct DRUMFUNCTION drumstuff;

#endif

