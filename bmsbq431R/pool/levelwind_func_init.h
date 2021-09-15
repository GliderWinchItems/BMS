/******************************************************************************
* File Name          : levelwind_func_init.h
* Date First Issued  : 09/23/2020
* Description        : LevelwindTask initialize function struct
*******************************************************************************/

#ifndef __LEVELWINDFUNCINIT
#define __LEVELWINDFUNCINIT

#include "LevelwindTask.h"

/* *************************************************************************/
void levelwind_func_init_init(struct LEVELWINDFUNCTION* p);
/*	@brief	: Initialize working struct for ContactorTask
 * @param	: p    = pointer to ContactorTask
 * *************************************************************************/
void levelwind_func_init_canfilter(struct LEVELWINDFUNCTION* p);
/*	@brief	: Setup CAN hardware filter with CAN addresses to receive
 * @param	: p    = pointer to ContactorTask
 * *************************************************************************/

#endif

