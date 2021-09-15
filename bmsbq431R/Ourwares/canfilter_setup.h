/******************************************************************************
* File Name          : canfilter_setup.h
* Date First Issued  : 01/09/2019
* Description        : CAN FreeRTOS/ST HAL: Hardware filtering
*******************************************************************************/

#ifndef __CANFILTER_SETUP
#define __CANFILTER_SETUP

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_can.h"
#include "FreeRTOS.h"

/* *************************************************************************/
HAL_StatusTypeDef canfilter_setup_first(uint8_t cannum, CAN_HandleTypeDef *phcan, uint8_t slavebankdmarc);
/* @brief	: Sets Bank 0 to pass ==>all<== msgs to FIFO 0, 32b mask mode
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: slavebankdmarc = For CAN2, filter bank number demarcation, where CAN2 banks start
 * @return	: HAL_ERROR or HAL_OK
 * *************************************************************************/
HAL_StatusTypeDef canfilter_setup_add32b_mskmode(uint8_t cannum, \
	 CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint32_t msk,  \
    uint8_t  fifo  );
/* @brief	: Add a 32b  id and mask to a specified filter bank
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: id    = 32b CAN id
 * @param	: msk   = 32b mask (0's are don't cares)
 * @param	: fifo  = fifo: 0 or 1
 * @return	: HAL_ERROR or HAL_OK
 * *************************************************************************/
HAL_StatusTypeDef canfilter_setup_add32b_id(uint8_t cannum, CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint8_t  fifo );
/* @brief	: Add a 32b id, advance bank number & odd/even
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: id    = 32b CAN id
 * @param	: fifo  = fifo: 0 or 1
 * @return	: HAL_ERROR or HAL_OK
 * *************************************************************************/

#endif
