/******************************************************************************
* File Name          : cancomm_items.c
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#include "can_iface.h"
#include "CanTask.h"
#include "BQTask.h"

/* *************************************************************************
 * void cancomm_items_sendcell(struct CANRCVBUF* pcan);
 *	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/
void cancomm_items_sendcell(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;
	int16_t* pcell = &p->cellv_latest[0];
	uint8_t i;
	uint8_t n;
		for (i = 0; i < MAXNUMCELLMSGS; i++)
	{	
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[2] = *pcell >> 0;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[3] = *pcell >> 8;
		pcell += 1;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[4] = *pcell >> 0;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[5] = *pcell >> 8;
		pcell += 1;		
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[6] = *pcell >> 0;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[7] = *pcell >> 8;
		pcell += 1;

		// Set sequence number sent by requesting CAN msgs
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[1] = pcan->cd.uc[1];

		// DLC is the same except possibly last msg
		p->canmsg[CID_MSG_CELLV01 + i].can.dlc = 8;
	}
	/* Adjust dlc if less than 18 cells. */
	switch (p->lc.ncell)
	{ // 
	case 16: //16 cells
		p->canmsg[CID_MSG_CELLV06].can.dlc = 4;
		n = 6;
		break;
	case 14: // 14 cells
		p->canmsg[CID_MSG_CELLV06].can.dlc = 6;
		n = 5;
		break;
	case 12: // 12 cells
		p->canmsg[CID_MSG_CELLV06].can.dlc = 8;
		n = 4;
		break;
	default: // 18 cells (or garbage)
		p->canmsg[CID_MSG_CELLV06].can.dlc = 8;
		n = 6;
	}

	/* Queue CAN msgs for output. */
	for (i = 0; i < n; i++)
	{
		xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_MSG_CELLV01 + i],4);   
	}

	return;
}
