/******************************************************************************
* File Name          : gevcu_cmd_msg.h
* Date First Issued  : 07/03/2019
* Description        : cid_cmd_msg_i: Function command
*******************************************************************************/

#ifndef __CONTACTORCMDMSG
#define __CONTACTORCMDMSG

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "adc_idx_v_struct.h"
#include "adcparams.h"
#include "GevcuTask.h"
#include "CanTask.h"


/* *************************************************************************/
void gevcu_cmd_msg_i(struct GEVCUFUNCTION* pcf);
/*	@brief	: Given the Mailbox pointer (within CONTACTORFUNCTION) handle request
 * *************************************************************************/

#endif
