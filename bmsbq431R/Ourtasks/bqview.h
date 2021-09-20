/******************************************************************************
* File Name          : bqview.h
* Date First Issued  : 09/18/2021
* Description        : BQ76952: yprintf BQ data
*******************************************************************************/

#ifndef __BQVIEW
#define __BQVIEW

#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "SerialTaskSend.h"

/* *************************************************************************/
void bqview_blk_0x9231 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters
 *************************************************************************/
 void bqview_blk_0x92fa (struct SERIALSENDTASKBCB** pp);
 /*	@brief	: display parameters
  * *************************************************************************/
void bqview_blk_0x62 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters
 * *************************************************************************/

#endif

