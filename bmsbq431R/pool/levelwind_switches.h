/******************************************************************************
* File Name          : levelwind_switches.h
* Date First Issued  : 09/16/2020
* Description        : Levelwind function w STM32CubeMX w FreeRTOS
*******************************************************************************/


#ifndef __LEVELWINDSWITCHES
#define __LEVELWINDSWITCHES

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "CanTask.h"
#include "SerialTaskSend.h"
#include "main.h"



/* Positions of bits in 'switchbits' */
#define LIMITDBMSN  0   // Debounced MotorSideNot limit sw 1 = closed
#define LIMITDBMS 1   // Debounced MotorSide limit sw 1 = closed
#define LIMITMSNNO    (LimitSw_MSN_NO_Pin)  // MotorSideNot  NC contact 0 = closed
#define LIMITMSNNC    (LimitSw_MSN_NC_Pin)  // MotorSideNot  NO contact 0 = closed
#define LIMITMSNO   (LimitSw_MS_NO_Pin) // MotorSide NC contact 0 = closed
#define LIMITMSNC   (LimitSw_MS_NC_Pin) // MotorSide NO contact 0 = closed
#define OVERRUNSWES      (OverrunSwes_NO_Pin)     // Either  NO contact 0 = closed
#define OVERRUNBRIDGE (1<<9) // Bridge sw for testing overrun. 0 = test position

/* Integrity status bits. */
#define STEPPERSWSTS00 (1<<00) // Bridge switch in TEST position
#define STEPPERSWSTS01 (1<<01) // All switch contacts show ON (pullups)
#define STEPPERSWSTS02 (1<<02) // MotorSideNot-MotorSide limit sws both ON 
#define STEPPERSWSTS03 (1<<03) // MotorSideNot NC and NO are both closed
#define STEPPERSWSTS04 (1<<04) // MotorSide NC and NO are both closed

/*  Reconsider with wire-ORed overrun levelwind_switches*/
#define STEPPERSWSTS05 (1<<05) // Both overrun NC are open
#define STEPPERSWSTS06 (1<<06) // MotorSide overrun closed, MotorSide limit sw open
#define STEPPERSWSTS07 (1<<07) // MotorSideNot overrun closed, MotorSide limit sw open 


/* Alert status bits */
#define STEPPERSWALRT00 (1<<0) // Bridge switch in TEST position
#define STEPPERSWALRT01 (1<<1) // An overun switch is closed

struct SWITCHXITION
{
	uint32_t cnt;
	uint32_t tim;
	uint16_t sws;
};

/* Parameters for switch contact closures versus position accumulator. */
struct STEPPERSWCONTACT
{
   int32_t close;
   int32_t open;
   uint32_t delay;
};

struct EXTISWITCHSTATUS
{
   uint32_t tim2;   // TIM2 CNT
    int32_t posaccum_NO;  // Position accumulator
    int32_t posaccum_NC;  // Position accumulator
   uint32_t errbit; // Bit for each possible error
   uint8_t  cur;    // Current
   uint8_t  prev;   // Previous
   uint8_t  dbs;    // Debounced state
   uint8_t  flag1;   // 0 = handled; 1 = not handled
   uint8_t  flag2;   // 0 = handled; 1 = not handled
   uint8_t  status; // Integrity status
};

/* *************************************************************************/
int levelwind_switches_defaultTaskcall(struct SERIALSENDTASKBCB* pbuf1);
/* @brief       : Call from main.c defaultTAsk jic
 * *************************************************************************/
void levelwind_switches_init(void);
/* @brief       : Initialization
 * *************************************************************************/
struct SWITCHXITION* levelwind_switches_get(void);
/* @brief       : Get pointer to buffer if reading available
 * @return      : pointer to buffer entry; NULL = no reading
 * *************************************************************************/

#endif

