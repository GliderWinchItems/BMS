/******************************************************************************
* File Name          : cancomm_items.h
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#ifndef __CANCOMM_ITEMS
#define __CANCOMM_ITEMS

#include "can_iface.h"

/* *************************************************************************/
 void cancomm_items_sendcell(struct CANRCVBUF* pcan);
/*	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/

#endif
