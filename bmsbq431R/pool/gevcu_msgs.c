/******************************************************************************
* File Name          : gevcu_msgs.c
* Date First Issued  : 11/29/2019
* Description        : Setup and send CAN msgs
*******************************************************************************/
/*
SENT by gevcu function:
 (1) gevcu command "cid_keepalive_r" (response to "cid_keepalive_i")
     payload[0]
       bit 7 - faulted (code in payload[2])
       bit 6 - warning: minimum pre-chg immediate connect.
              (warning bit only resets with power cycle)
		 bit[0]-[3]: Current main state code

 
*/

#include "gevcu_msgs.h"
#include "MailboxTask.h"

/* *************************************************************************/
void gevcu_msgs_contactorka(struct GEVCUFUNCTION* p);
/*	@brief	: Send CAN msg to CONTACTOR
 * @param	: pcf = pointer to everything good
 * *************************************************************************/
void gevcu_msgs_contactorka(struct GEVCUFUNCTION* p)
{
	



}
