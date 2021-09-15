/******************************************************************************
* File Name          : serialinputsetup_slow.c
* Date First Issued  : 01/25/2019
* Description        : Most parameters for slow serial input (e.g. 115,200 baud)
*******************************************************************************/

/* *************************************************************************
 * BaseType_t serialinputsetup_slow(UART_HandleTypeDef* phuart,uint32_t  notebit,uint32_t* pnoteval);
 * @brief	: char-by-char: Check for line terminator and store; enter via interrupt
 * @param	: prbcb = pointer to buffer control block for uart causing callback
 * *************************************************************************/
BaseType_t serialinputsetup_slow(UART_HandleTypeDef* phuart,uint32_t  notebit,uint32_t* pnoteval)
{
	BaseType_t ret = xSerialTaskRxAdduart( phuart,0,notebit,pnoteval,\
		3, 128, 0);
	return ret;
}

