/******************************************************************************
* File Name          : bms_items.c
* Date First Issued  : 07/14/2022
* Description        : routines associated BMSTask and bmsspi
*******************************************************************************/

#include "DTW_counter.h"
#include "main.h"
#include "morse.h"
#include "bms_items.h"

/* *************************************************************************
 * void bms_items_set_overunder(void);
 * @brief	: Compute and set over and under voltage comparisons in configreg A 
 * *************************************************************************/
void bms_items_set_overunder(void)
{
	uint32_t vuv; // Under voltage
	uint32_t vov; // Over voltage 
	float fvuv;
	/* Convert max & min voltage into '1818 regs: CFGARA1,2,3 */
	/* vuv and vov are 12 bit numbers (0-4095) packed into CFGAR1,2,3 .*/
	/* Datasheet formula:
	   Undervoltage comparison voltage = (VUV+1)*16*100 (uV)
	   Overvoltage  comparison voltage = VOV*16*100 (uV) */
	fvuv = ((bqfunction.lc.cellv_min * 1E6) / 1600.0)-1;
	fvov =  (bqfunction.lc.cellv_max * 1E6) / 1600.0;
	vuv = fvuv;
	vov = fvov;

	// CFGAR1 (low ord 8 bits of vuv)
	bmsspiall.configreg[0] &= 0x00ff;
	bmsspiall.configreg[0] |= (vuv & 0xff);

	// CFGAR2 (hi 24bits of vov | hi ord 4 bits of vuv)
	bmsspiall.configreg[1] = (vov << 8) | (vuv >> 4);
	return;
}
/* *************************************************************************
 * void bms_items_set_dischargebits(uint32_t b);
 * @brief	: Update discharge bits in configreg (in memory)
 * @param   : b = bits for 18 cells (right justified): 0 = OFF, 1 = ON 
 * *************************************************************************/
void bms_items_set_dischargebits(uint32_t b)
{
	b = (b & 0x3FFFF); // JIC
	// Update low ord 12 bits in CFGAR 5,4
	bmsspiall.configreg[2] &= 0xF000; 
	bmsspiall.configreg[2] |= (b & 0x0FFF); 

	bmsspiall.configreg[3] &= 0xFC0F;
	bmsspiall.configreg[3] |= (b >> 12);

	return;
}