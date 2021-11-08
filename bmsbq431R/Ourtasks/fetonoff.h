/******************************************************************************
* File Name          : fetonoff.h
* Date First Issued  : 09/05/2021
* Description        : FET on/off control
*******************************************************************************/

#ifndef __FETONOFF
#define __FETONOFF

#define FET_SETOFF  0  // Turn FET off
#define FET_SETON   1  // Turn FET on

/* *************************************************************************/
uint8_t fetonoff(uint8_t fetnum, uint8_t fetcommand);
 /* @brief	: Set i/o bits to turn fet on or off
 * @param	: fetnum = designate FET
 * @param	: fetcommand: 1 = on; not 1 = off
 * @return  : status byte with bits set/reset for each FET 
 * *************************************************************************/
void fetonoff_status_set(uint8_t status);
/* @brief	: Set FETs according to status byte (see BQTask.h)
 * @param   : status = status bits
 * *************************************************************************/

#endif

