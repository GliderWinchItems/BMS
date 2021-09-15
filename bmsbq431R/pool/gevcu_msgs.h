/******************************************************************************
* File Name          : gevcu_msgs.h
* Date First Issued  : 07/05/2019
* Description        : Setup and send non-command function CAN msgs
*******************************************************************************/

#ifndef __GEVCU_MSGS
#define __GEVCU_MSGS

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "adc_idx_v_struct.h"
#include "adcparams.h"
#include "GevcuTask.h"
#include "CanTask.h"

/* *************************************************************************/
void gevcu_msg1(struct GEVCUFUNCTION* pcf, uint8_t w);
/*	@brief	: Setup and send responses: battery string voltage & battery current
 * @param	: pcf = Pointer to working struct for Contactor function
 * @param	: w = switch for CID_HB1 (0) or CID_MSG1 CAN ids (1)
 * *************************************************************************/
void gevcu_msg2(struct GEVCUFUNCTION* pcf, uint8_t w);
/*	@brief	: Setup and send responses: voltages: DMOC+, DMOC-
 * @param	: pcf = Pointer to working struct for Contactor function
 * @param	: w = switch for CID_HB1 (0) or CID_MSG1 CAN ids (1)
 * *************************************************************************/
void gevcu_msg_ka(struct GEVCUFUNCTION* pcf);
/*	@brief	: Setup and send Keep-alive response
 * @param	: pcf = Pointer to working struct for Contactor function
 * *************************************************************************/

#endif


