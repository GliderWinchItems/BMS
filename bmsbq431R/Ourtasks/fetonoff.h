/******************************************************************************
* File Name          : fetonoff.h
* Date First Issued  : 09/05/2021
* Description        : FET on/off control
*******************************************************************************/

#ifndef __FETONOFF
#define __FETONOFF

/* FET numbering. */
#define FETON_DUMP    1  // Battery module discharge "dump"
#define FETON_DUMP2   2  // Spare for relays, etc.
#define FETON_HEATER  3  // Battery module warmup heater

/* Bit ON = FET I/O Pins set for FET to be ON. */
#define FETON_DUMP_STATUS   (1<<0)
#define FETON_DUMP2_STATUS  (1<<1) 
#define FETON_HEATER_STATUS (1<<2)

#define FETON_SETOFF  0  // Turn FET off
#define FETON_SETON   1  // Turn FET on

/* *************************************************************************/
uint8_t fetonoff(uint8_t fetnum, uint8_t fetcommand);
 /* @brief	: Set i/o bits to turn fet on or off
 * @param	: fetnum = designate FET
 * @param	: fetcommand: 1 = on; not 1 = off
 * @return  : byte with bits set/reset for each FET 
 * *************************************************************************/
uint8_t fetonoff_status(void);
/* @brief	: Return status
 * @return  : byte with bits set/reset for each FET 
 * *************************************************************************/


#endif

