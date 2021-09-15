/******************************************************************************
* File Name          : getserialbuf.h
* Date First Issued  : 01/11/2019
* Description        : Get a buffer & control block for SerialTaskSend use
*******************************************************************************/

#ifndef __GETSERIALBUFY
#define __GETSERIALBUFY

#include "SerialTaskSend.h"

/* *************************************************************************/
struct SERIALSENDTASKBCB* getserialbuf( UART_HandleTypeDef* phuart, uint16_t maxsize);
/* @brief	: Create a buffer control block (BCB) for serial sending
 * @param	: phuart = usart handle (pointer)
 * @param	: size = number of uint8_t bytes for this buffer
 * @return	: pointer to BCB; NULL = failed
 * *************************************************************************/

#endif

