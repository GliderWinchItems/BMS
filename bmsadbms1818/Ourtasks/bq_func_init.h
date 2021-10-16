/******************************************************************************
* File Name          : bq_func_init.h
* Date First Issued  : 10/01/2021
* Board              :
* Description        : Init function struc
*******************************************************************************/

#ifndef __BQ_FUNC_INIT
#define __BQ_FUNC_INIT

#include <stdint.h>
#include "BQTask.h"
#include "bq_idx_v_struct.h"

/* *************************************************************************/
 void bq_func_init(struct BQFUNCTION* p);
/* @brief	: Init struct with working parameters
 * @param   : p = pointer to struct will all parameters for BQ function
 * *************************************************************************/

#endif