/******************************************************************************
* File Name          : levelwind_cmd.h
* Date First Issued  : 11/11/2020
* Description        : Levelwind handle command msgs
*******************************************************************************/
/*
11/11/2020 drum:levelwind
*/

#ifndef __LEVELWIND_CMD
#define __LEVELWIND_CMD


#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "levelwind_items.h"
#include "common_can.h"
#include "CanTask.h"
#include "levelwind_switches.h"
#include "LevelwindTask.h"

/*
   0 =  Levelwind switches (uint32_t)
   1 = CAN bus voltage (float)
   2 = Stepper Controller voltage (float)
   3 = Position accumulator at motor-side limit sw closure
   4 = Position accumulator at not-motor-side limit sw closure
   5 = Position accumulator at center
*/
#define LWCMD_SWITCHES   0  // Levelwind switches (uint32_t)
#define LWCMD_BUS12V     1  // CAN bus voltage (float)
#define LWCMD_STEPVOLT   2  // Stepper Controller voltage (float)
#define LWCMD_POSACC_MS  3  // Position accumulator at motor-side limit sw closure
#define LWCMD_PSACC_MSN  4  // Position accumulator at not-motor-side limit sw closure
#define LWCMD_UNDEF    255  // Command not defined

/* *************************************************************************/
 void levelwind_CANrcv_cid_cmd_levelwind_i1(struct CANRCVBUF* pcan);
/* @brief   : Incoming (LW receives) CAN command
 * *************************************************************************/

#endif
