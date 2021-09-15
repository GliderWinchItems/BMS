/******************************************************************************
* File Name          : gevcu_func_init.h
* Date First Issued  : 10/09/2019
* Description        : uart input
*******************************************************************************/

#ifndef __GEVCUFUNCINIT
#define __GEVCUFUNCINIT

#include "iir_filter_lx.h"
#include "adcparams.h"

/* *************************************************************************/
void gevcu_func_init_init(struct GEVCUFUNCTION* p, struct ADCFUNCTION* padc);
/*	@brief	: Initialize working struct for ContactorTask
 * @param	: p    = pointer to ContactorTask
 * @param	: padc = pointer to ADC working struct
 * *************************************************************************/
void gevcu_func_init_canfilter(struct GEVCUFUNCTION* p);
/*	@brief	: Setup CAN hardware filter with CAN addresses to receive
 * @param	: p    = pointer to ContactorTask
 * *************************************************************************/

#endif

