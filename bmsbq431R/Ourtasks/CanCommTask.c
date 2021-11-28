/******************************************************************************
* File Name          : CanCommTask.c
* Date First Issued  : 11/06/2021
* Description        : Can communications
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"
#include "CanCommTask.h"
#include "MailboxTask.h"
#include "CanTask.h"
#include "can_iface.h"
#include "canfilter_setup.h"
#include "BQTask.h"
#include "cancomm_items.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

extern struct CAN_CTLBLOCK* pctl0; // Pointer to CAN1 control block

#define CANCOMMBIT00 (1 << 0) // Send cell voltage  command
#define CANCOMMBIT01 (1 << 1) // Send misc reading command

uint8_t rdyflag_cancomm = 0; // Initialization complete and ready = 1

TaskHandle_t CanCommHandle = NULL;
/* *************************************************************************
 * void StartCanComm(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartCanComm(void* argument)
{
//while(1==1) osDelay(100);

	/* We use the parameters from BQ for this task. */
	struct BQFUNCTION* p = &bqfunction;
	struct CANRCVBUF* pcan;

	/* A notification copies the internal notification word to this. */
	uint32_t noteval = 0;    // Receives notification word upon an API notify

	uint8_t i;
	uint8_t code;


	/* Add CAN Mailboxes                               CAN     CAN ID             TaskHandle,Notify bit,Skip, Paytype */
    p->pmbx_cid_cmd_bms_cellvq = MailboxTask_add(pctl0,p->lc.cid_cmd_bms_cellvq, NULL, CANCOMMBIT00,0,U8); // Command: send cell voltages
    p->pmbx_cid_cmd_bms_miscq  = MailboxTask_add(pctl0,p->lc.cid_cmd_bms_miscq,  NULL, CANCOMMBIT01,0,U8); // Command: many options	

    	/* Pre-load fixed data in CAN msgs */
	for (i = 0; i < NUMCANMSGS; i++)
	{
		p->canmsg[i].pctl = pctl0;   // Control block for CAN module (CAN 1)
		p->canmsg[i].maxretryct = 8; //
		p->canmsg[i].bits = 0;       //
		p->canmsg[i].can.dlc = 8;    // Default payload size (might be modified when loaded and sent)
	}


	/* Preload fixed data for sending cell reading msgs. */
	for (i = 0; i < MAXNUMCELLMSGS; i++)
	{	
		/* Preload CAN IDs */
		p->canmsg[CID_MSG_CELLV01 + i].can.id = p->lc.cid_msg_bms_cellvsmr;

	/* Preload string and module number. */
		/*  payload [0-1] U16 – Payload Identification
  			[15:14] Winch (0 - 3)(winch #1 - #4)
  			[13:12] Battery string (0 – 3) (string #1 - #4)
  			[11:8] Module (0 – 15) (module #1 - #16)
  			[7:3] Cell (0 - 31) (cell #1 - #32)
  			[2:0] Group sequence number (0 - 7) */
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[0]  = p->cellvpayident;

		/* Preload cell number-1 for first payload cell reading.. */
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[0] |= (i*3 << 3); // 0,3,6,8,12,15
	}

	/* Preload fixed data for sending "misc" readings. */
	p->canmsg[CID_CMD_MISC].can.id = p->lc.cid_cmd_bms_miscq;
	p->canmsg[CID_CMD_MISC].can.cd.uc[0] = p->ident_onlyus;

	while (CANTaskreadyflag == 0) osDelay(1);

extern CAN_HandleTypeDef hcan1;
	HAL_CAN_Start(&hcan1); // CAN1

	rdyflag_cancomm = 1; // Initialization complete and ready

	for (;;)
	{
				/* Wait for notifications */
		xTaskNotifyWait(0,0xffffffff, &noteval, p->hbct_k);

		/* CAN msg request for sending CELL VOLTAGES. */
			// Code for which modules should respond bits [7:6]
	   		// 11 = All modules respond
       		// 10 = All modules on identified string respond
       		// 01 = Only identified string and module responds
       		// 00 = spare; no response expected
		if ((noteval & CANCOMMBIT00) != 0)
		{
			pcan = &p->pmbx_cid_cmd_bms_cellvq->ncan.can;
			code = pcan->cd.uc[0] & 0xC0; // Extract identification code
			if (((code == (3 << 6))) ||
				((code == (2 << 6)) && ((pcan->cd.uc[0] & 0x30) == p->ident_string)) ||
				((code == (1 << 6)) && ((pcan->cd.uc[0] & 0x3F) == p->ident_onlyus)) )
			{ // Here, respond to request, otherwise, ignore.
				cancomm_items_sendcell(pcan);
			}
		}
		/* CAN msg request for sending MISC READINGS. */
			// Code for which modules should respond bits [7:6]
	   		// 11 = All modules respond
       		// 10 = All modules on identified string respond
       		// 01 = Only identified string and module responds
       		// 00 = spare; no response expected
		if ((noteval & CANCOMMBIT01) != 0)
		{
			pcan = &p->pmbx_cid_cmd_bms_miscq->ncan.can;
			code = pcan->cd.uc[0] & 0xC0; // Extract identification code
			if (((code == (3 << 6))) ||
				((code == (2 << 6)) && ((pcan->cd.uc[0] & 0x30) == p->ident_string)) ||
				((code == (1 << 6)) && ((pcan->cd.uc[0] & 0x3F) == p->ident_onlyus)) )
			{ // Here, respond to request
				cancomm_items_sendcmdr(pcan);
			}
		}
		if (noteval == 0)
		{ // Send heartbeat
			  /* Queue CAN msg to send. */
			p->canmsg[CID_CMD_MISC].can.cd.uc[0] &= ~0xC0; 
			p->canmsg[CID_CMD_MISC].can.cd.uc[1]  = 0; 
			p->canmsg[CID_CMD_MISC].can.dlc    = 2; 
   			xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);   
		}	
	} 	
}

/* *************************************************************************
 * TaskHandle_t xCanCommCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: CanCommHandle
 * *************************************************************************/
TaskHandle_t xCanCommCreate(uint32_t taskpriority)
{
	BaseType_t ret = xTaskCreate(StartCanComm, "CanComm",\
     (128), NULL, taskpriority,\
     &CanCommHandle);
	if (ret != pdPASS) return NULL;

	return CanCommHandle;
}
