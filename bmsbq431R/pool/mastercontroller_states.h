/******************************************************************************
* File Name          : mastercontroller_states.h
* Date First Issued  : 11/09/2020
* Description        : Defines states for Master Controller
*******************************************************************************/

#ifndef __MASTERCONTROLLERSTATES
#define __MASTERCONTROLLERSTATES

#include <stdint.h>

// Master Controller state machine  definitions 
// Major state names
#define MC_INIT     ( 0 << 3)
#define MC_SAFE     ( 1 << 3)
#define MC_PREP     ( 2 << 3)
#define MC_ARMED    ( 3 << 3)
#define MC_GRNDRTN  ( 4 << 3)
#define MC_RAMP     ( 5 << 3)
#define MC_CLIMB    ( 6 << 3)
#define MC_RECOVERY ( 7 << 3)
#define MC_RETRIEVE ( 8 << 3)
#define MC_ABORT    ( 9 << 3)
#define MC_STOP     (10 << 3)
#define MC_TEST     (11 < <3)

#endif