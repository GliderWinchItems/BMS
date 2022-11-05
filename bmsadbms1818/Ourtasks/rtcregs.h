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
#define RTCREGUSED 12	// Number of RTC registers used 
struct RTCREG 
{
	uint32_t cellbal;       // Bits with FETs that were on
	uint32_t hysterbits_lo; // Bits for cells that fell below hysterv_lo
	uint16_t cellreg[18];   // Last readings before power down
	uint8_t hyster_sw;      // Hysteresis switch: 1 = peak was reached
	uint8_t battery_status; // Cell status code bits 
	uint8_t fet_status;     // This controls on/off of FETs
	uint8_t err;            // Err bits
/* The foregoing MUST be an even number of 32b */	
	uint16_t pec15; // CRC15 (in lower 1/2 of register)
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

#endif