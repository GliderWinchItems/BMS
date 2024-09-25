/******************************************************************************
* File Name          : rtcregs.h
* Date First Issued  : 07/26/2022
* Description        : RTC Backup registers access
*******************************************************************************/
#ifndef __RTCREGS
#define __RTCREGS

#include <stdint.h>
#include "stm32l4xx_hal.h"

/* Backup register usage. 
Entries, except for pec15, correspond to names in bqfunction, and bmsspiall
structs.

Max (32b) registers: 32
*/
#define RTCREGUSED 15	// Number of RTC =>32b<= registers used (not including crc)
struct RTCREG 
{
	//                         Regs used
	uint32_t cellbal;       //   1.00 Bits with FETs that were on
	uint32_t hysterbits_lo; //   1.00 Bits for cells that fell below hysterv_lo
	uint16_t cellreg[18];   //   9.00 Last readings before power down
	uint8_t hyster_sw;      //   .125 Hysteresis switch: 1 = peak was reached
	uint8_t battery_status; //   .125 Cell status code bits 
	uint8_t fet_status;     //   .125 This controls on/off of FETs
	uint8_t err;            //   .125 Err bits
	uint32_t morse_err;     //   1.00 morse_trap err code
	uint32_t celltrip;      //   1.00 Cell trip bits
	uint16_t morse_err_ct;  //    .50 Count of same morse_trap code
	uint16_t warning;       //    .50 Warning, but not morse_trap'd
/* The foregoing ==> MUST <== fit whole 32b words. */	
	uint16_t pec15; // CRC15 (in lower 1/2 of 32b register)
};

/* *************************************************************************/
int8_t rtcregs_init(void);
/*	@brief	: Init access to regs
 *  @return : 0 = success; -1 = PEC failed
 * *************************************************************************/
void rtcregs_update(void);
/*	@brief	: Write selected SRAM into RTC registers
 * *************************************************************************/
int8_t rtcregs_read(void);
/*	@brief	: Check PEC and if OK load SRAM from RTC registers
 *  @return : 0 = success; -1 = PEC failed
 * *************************************************************************/

extern uint32_t morse_err;
extern uint16_t morse_err_ct;
extern uint16_t warning;

#endif