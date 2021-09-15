/******************************************************************************
* File Name          : paycnvt.h
* Date First Issued  : 12/07/2019
* Description        : Conversions of various payload formats
*******************************************************************************/
#ifndef __PAYCNVT
#define __PAYCNVT

#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include "GevcuTask.h"
#include "gevcu_idx_v_struct.h"
#include "main.h"
#include "morse.h"
#include "common_can.h"

union UNION_PAY
{
	uint32_t ui; 
	int32_t  si;
	uint16_t us[2];
	int16_t  ss[2];
	uint8_t  uc[4];
	float ff;
};

/* ********************************************************************************
 * uintYXX payuYXX(struct CANRCVBUF* pcanx, int offset)
 * @brief	: Combine bytes to make an unsigned int: Y = U or I, XX = 32 or 16
 * @param	: pcanx = pointer to struct with payload
 * @param	: offset = number of bytes to offset for int
 * @return	: uint32_t, or uint16_t, or float
*/
uint32_t payU32(struct CANRCVBUF* pcanx, int offset); // 4 byte unsigned int
uint16_t payU16(struct CANRCVBUF* pcanx, int offset); // 2 byte unsigned int
uint32_t payI32(struct CANRCVBUF* pcanx, int offset); // 4 byte unsigned int Big Endian
uint16_t payI16(struct CANRCVBUF* pcanx, int offset); // 2 byte unsigned int Big Endian
float    payFF (struct CANRCVBUF* pcanx, int offset); // 4 byte float
/* ********************************************************************************/
union UNION_PAY convertpayload(struct CANRCVBUF* pcanx, uint8_t paycode, uint8_t k);
/* @brief	: Extract payload bytes
 * @param	: pcanx = pointer to CAN msg
 * @param	: paycode = type of payload (e.g. I16__I16' = 32)
 * @param	: k = payload item index: (0 - 7)
 * @return	: UNION_PAY = holds extracted payload item
 **********************************************************************************/

#endif

