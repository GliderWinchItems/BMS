/******************************************************************************
* File Name          : payload_extract.h
* Date First Issued  : 02/23/2019
* Description        : Extract payload from CAN msg
*******************************************************************************/

#ifndef __PAYLOADEXTRACT
#define __PAYLOADEXTRACT

#include "can_iface.h"
#include "MailboxTask.h"

/* *************************************************************************/
void payload_extract(struct MAILBOXCAN* pmbx);
/*	@brief	: Lookup CAN ID and load mailbox with extract payload reading(s)
 * @param	: pmbx  = pointer to mailbox
 * *************************************************************************/

#endif

