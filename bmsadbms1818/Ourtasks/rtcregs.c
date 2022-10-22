/******************************************************************************
* File Name          : rtcregs.c
* Date First Issued  : 07/26/2022
* Description        : RTC Backup registers access
*******************************************************************************/

#include "main.h"
#include "morse.h"
#include "DTW_counter.h"
#include "rtcregs.h"
#include "pec15_reg.h"
#include "BQTask.h"
#include "BMSTask.h"

static void rtcregs_load(void);

/* *************************************************************************
 * int8_t rtcregs_init(void);
 *	@brief	: Init access to regs
 *  @return : 0 = success; -1 = PEC failed
 * *************************************************************************/
int8_t rtcregs_init(void)
{
	// Bit 8 DBP: Disable backup domain write protection
	PWR->CR1 |= (1 << 8); // 1: Access to RTC and Backup registers enabled
	pec15_reg_init();
	int8_t ret =rtcregs_read();
	if (ret == 0)
	{ // RTC registers have correct CRC
		rtcregs_load(); // Load regs into SRAM
	}
	// Use default SRAM settings
	return ret;
}

/*
struct RTCREG
{
	uint32_t cellbal;       // Bits with FETs that were on
	uint32_t hysterbits_hi; // Bits for cells that reached cellv_max (target)
	uint32_t hysterbits_lo; // Bits for cells that fell below hysterv_lo
	uint16_t cellreg[18]; 
	uint8_t hyster_sw;      // Hysteresis switch: 1 = peak was reached
	uint8_t battery_status; // Cell status code bits 
	uint8_t fet_status;     // This controls on/off of FETs
	uint8_t err             // Err bits
	uint16_t pec15; // CRC15
};*/
/* *************************************************************************
 * void rtcregs_update(void);
 *	@brief	: Write selected SRAM into RTC registers
 * *************************************************************************/
void rtcregs_update(void)
{
	uint32_t* prtc = (uint32_t*)&RTC->BKP0R;
	uint16_t* p16 = &bmsspiall. cellreg[0];

	PWR->CR1 |= (1 << 8); // 1: Access to RTC and Backup registers enabled

	/* Unlock registers to allow writes. */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	*prtc++ = bqfunction.cellbal; // Bits to activate cell balance fets
	*prtc++ = bqfunction.hysterbits_lo; // Bits for cells that fell below hysterv_lo
	*prtc++ = *(p16 +  0) | (*(p16 +  1) << 16);
	*prtc++ = *(p16 +  2) | (*(p16 +  3) << 16);
	*prtc++ = *(p16 +  4) | (*(p16 +  5) << 16);
	*prtc++ = *(p16 +  6) | (*(p16 +  7) << 16);
	*prtc++ = *(p16 +  8) | (*(p16 +  9) << 16);
	*prtc++ = *(p16 + 10) | (*(p16 + 11) << 16);
	*prtc++ = *(p16 + 12) | (*(p16 + 13) << 16);
	*prtc++ = *(p16 + 14) | (*(p16 + 15) << 16);
	*prtc++ = *(p16 + 16) | (*(p16 + 17) << 16);
	*prtc++ = 	(
				(bqfunction.hyster_sw <<  0) |  /* Hysteresis switch: 1 = peak was reached */
				(bqfunction.battery_status << 8) | /* Cell status code bits */
				(bqfunction.fet_status << 16) |
				(bqfunction.err << 24)
				); 
	uint8_t* pr = (uint8_t*)&RTC->BKP0R;
	int len = (uint8_t*)prtc - (uint8_t*)&RTC->BKP0R;
	*prtc = pec15_reg (pr, len);

	/* Relock writing to registers. */
	RTC->WPR = 0x0;
//	PWR->CR1 &= ~(1 << 8); // Disable access (read or write)

	return;
}
/* *************************************************************************
 * int8_t rtcregs_read(void);
 *	@brief	: Check PEC and if OK load SRAM from RTC registers
 *  @return : 0 = success; -1 = PEC failed
 * *************************************************************************/
uint16_t* dbgrtc;
int8_t rtcregs_read(void)
{
	uint16_t pec1;
	uint16_t pec2;
	uint16_t* prtc = (uint16_t*)&RTC->BKP0R;
dbgrtc = prtc;	

	/* Enable read access. */
	PWR->CR1 |= (1 << 8); // 1: Access to RTC and Backup registers enabled

	int len = 4*13; // Number of bytes in data registers (see 'update' above)

	pec1 = pec15_reg (((uint8_t*)prtc), len); // Compute PEC15 
	pec2 = *(prtc + 13*2); // Retrieve PEC15 in registers
	if (pec1 != pec2)
		return -1; // Fail
	return 0; // Success
}
/*
struct RTCREG
{
	uint32_t cellbal;       // Bits with FETs that were on
	uint32_t hysterbits_hi; // Bits for cells that reached cellv_max (target)
	uint32_t hysterbits_lo; // Bits for cells that fell below hysterv_lo
	uint16_t cellreg[18]; 
	uint8_t hyster_sw;      // Hysteresis switch: 1 = peak was reached
	uint8_t battery_status; // Cell status code bits 
	uint8_t fet_status;     // This controls on/off of FETs
	uint8_t err             // Err bits
	uint16_t pec15; // CRC15
};*/
/* *************************************************************************
 * void rtcregs_load(void);
 *	@brief	: Load RTC registers into SRAM
 * *************************************************************************/
static void rtcregs_load(void)
{
	uint32_t* prtc = (uint32_t*)&RTC->BKP0R;
	uint32_t* p32  = (uint32_t*)&bmsspiall. cellreg[0];

	PWR->CR1 |= (1 << 8); // 1: Access to RTC and Backup registers enabled

	bqfunction.cellbal        = *prtc++; // Bits to activate cell balance fets
	bqfunction.hysterbits_lo  = *prtc++;; // Bits for cells that fell below hysterv_lo
	*p32++ = *prtc++;
	*p32++ = *prtc++;
	*p32++ = *prtc++;
	*p32++ = *prtc++;
	*p32++ = *prtc++;
	*p32++ = *prtc++;
	*p32++ = *prtc++;
	*p32++ = *prtc++;

	bqfunction.hyster_sw       = (*prtc >>  0) & 0xf; /* Hysteresis switch: 1 = peak was reached */
	bqfunction.battery_status  = (*prtc >>  8) & 0xf; /* Cell status code bits */
	bqfunction.fet_status      = (*prtc >> 16) & 0xf;
	bqfunction.err             = (*prtc >> 24) & 0xf;

	return;
}