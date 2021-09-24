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
void bqview_blk_0x0083 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters
 * *************************************************************************/
void bqview_cuv_cov_snap_0x0080_0x0081 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters
 * *************************************************************************/
 void bqview_cb_status2_0x0086_0x0087 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters
 * *************************************************************************/
void bqview_blk_0x0075_s16 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters
 * *************************************************************************/
 void bqview_blk_0x0071_u32 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters: Cell current and voltages
 * *************************************************************************/
 void bqview_blk_0x9335_14 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters: Balancing items
 * *************************************************************************/
void bqview_blk_0x14_u16 (struct SERIALSENDTASKBCB** pp);
/*	@brief	: display parameters: Cell voltages
 * *************************************************************************/
void bqview_balance1 (struct SERIALSENDTASKBCB** pp);
/* @brief    : display parameters: Cell deviation around average
 * *************************************************************************/
void bqview_balance_misc (struct SERIALSENDTASKBCB** pp);
/* @brief    : display parameters: Max, min, etc.
 * *************************************************************************/

#endif

