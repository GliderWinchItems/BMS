/******************************************************************************
* File Name          : CanCommTask.c
* Date First Issued  : 11/06/2021
* Description        : Can communications
*******************************************************************************/
/*
When xTaskNotifyWait exits a timeout signals the time to send a heartbeat.
Notifications from an incoming CAN msg as well as the heartbeat places a pointer
on a queue to BMSTask.c to perform an'1818 BMS operation, e.g. read the cell
voltages. When BMSTask completes the request the notification of the completion
sets the notificaion word/bit specified in the request. The exit of 
xTaskNotifyWait then supplies the notification to complete the request (which
would a heartbeat or the variety of CAN msg requests).
*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"
//#include "semphr.h"

#include "main.h"
#include "morse.h"
#include "bmsspi.h"
#include "BMSTask.h"
#include "CanCommTask.h"
#include "MailboxTask.h"
#include "CanTask.h"
#include "can_iface.h"
#include "canfilter_setup.h"
#include "BQTask.h"
#include "bq_items.h"
#include "cancomm_items.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

extern struct CAN_CTLBLOCK* pctl0; // Pointer to CAN1 control block
extern CAN_HandleTypeDef hcan1;

//static void canfilt(uint16_t mm, struct MAILBOXCAN* p);

TaskHandle_t CanCommHandle = NULL;

/* xTaskNotifyWait Notification bits */
#define CANCOMMBIT00 (1 << 0) // Send cell voltage  command
#define CANCOMMBIT01 (1 << 1) // Send misc reading command
#define CANCOMMBIT02 (1 << 2) // Multi-purpose incoming command
#define CANCOMMBIT03 (1 << 3) // BMSREQ_Q [0] complete: heartbeat
#define CANCOMMBIT04 (1 << 4) // BMSREQ_Q [1] complete: Send cell voltage
#define CANCOMMBIT05 (1 << 5) // BMSREQ_Q [2] complete: Send misc reading command
#define CANCOMMBIT06 (1 << 6) // BMSREQ_Q [3] complete: Multi-purpose incoming command
#define CANCOMMBIT07 (1 << 7) // Send cell voltage  command
#define CANCOMMBIT08 (1 << 8) // Send misc reading command
#define CANCOMMBIT09 (1 << 9) // Multi-purpose incoming command

struct CANRCVBUF  can_hb; // Dummy heart-beat request CAN msg
uint8_t hbseq; // heartbeat CAN msg sequence number
uint8_t rdyflag_cancomm = 0; // Initialization complete and ready = 1

static struct BMSREQ_Q bmsreq_c[4];
static struct BMSREQ_Q* pbmsreq_c;

static struct CANRCVBUF can05;

/* *************************************************************************
 * static void CanComm_qreq(uint8_t idx, uint32_t notebit);
 *	@brief	: Queue request to BMSTask.c
 * *************************************************************************/
static void CanComm_qreq(uint32_t reqcode, uint8_t idx, uint32_t notebit)
{
	bmsreq_c[idx].reqcode  = reqcode;
    bmsreq_c[idx].tasknote = notebit; // Notification bit for this request
    bmsreq_c[idx].noteyes  = 1; // Wait notification
    // Queue request for BMSTask.c
//   int ret = xQueueSendToBack(BMSTaskReadReqQHandle, &pbmsreq_c[idx], 0);
//    if (ret != pdPASS) morse_trap(650);	
    return;
}
/* *************************************************************************
 * void CanComm_init(struct BQFUNCTION* p );
 *	@brief	: Task startup
 * *************************************************************************/
void CanComm_init(struct BQFUNCTION* p )
{
	uint8_t i;

	/* Add CAN Mailboxes                               CAN     CAN ID             TaskHandle,Notify bit,Skip, Paytype */
    p->pmbx_cid_cmd_bms_cellvq_emc = MailboxTask_add(pctl0,p->lc.cid_cmd_bms_cellvq_emc, NULL, CANCOMMBIT00,0,U8); // Command: send cell voltages
    if (p->pmbx_cid_cmd_bms_cellvq_emc == NULL) morse_trap(620);
    p->pmbx_cid_cmd_bms_miscq_emc  = MailboxTask_add(pctl0,p->lc.cid_cmd_bms_miscq_emc,  NULL, CANCOMMBIT01,0,U8); // Command: many options	
    if (p->pmbx_cid_cmd_bms_miscq_emc == NULL) morse_trap(621);
    p->pmbx_cid_uni_bms_emc_i      = MailboxTask_add(pctl0,p->lc.cid_uni_bms_emc_i,      NULL, CANCOMMBIT02,0,U8); // universal
    if (p->pmbx_cid_uni_bms_emc_i == NULL) morse_trap(622);

    p->pmbx_cid_cmd_bms_cellvq_pc = MailboxTask_add(pctl0,p->lc.cid_cmd_bms_cellvq_pc, NULL, CANCOMMBIT07,0,U8); // Command: send cell voltages
    if (p->pmbx_cid_cmd_bms_cellvq_pc == NULL) morse_trap(620);
    p->pmbx_cid_cmd_bms_miscq_pc  = MailboxTask_add(pctl0,p->lc.cid_cmd_bms_miscq_pc,  NULL, CANCOMMBIT08,0,U8); // Command: many options	
    if (p->pmbx_cid_cmd_bms_miscq_pc == NULL) morse_trap(621);
    p->pmbx_cid_uni_bms_pc_i      = MailboxTask_add(pctl0,p->lc.cid_uni_bms_pc_i,      NULL, CANCOMMBIT09,0,U8); // universal
    if (p->pmbx_cid_uni_bms_pc_i == NULL) morse_trap(622);


    /* Add CAN msgs to incoming CAN hw filter. (Skip to allow all incoming msgs. */
 //   canfilt(601, p->pmbx_cid_cmd_bms_cellvq_emc);
 //   canfilt(602, p->pmbx_cid_cmd_bms_miscq_emc);
 //   canfilt(603, p->pmbx_cid_uni_bms_emc_i);
 //   canfilt(601, p->pmbx_cid_cmd_bms_cellvq_pc);
 //   canfilt(602, p->pmbx_cid_cmd_bms_miscq_pc);
 //   canfilt(603, p->pmbx_cid_uni_bms_pc_i);

    	/* Pre-load fixed data in all possible CAN msgs */
	for (i = 0; i < NUMCANMSGS; i++)
	{
		p->canmsg[i].pctl = pctl0;   // Control block for CAN module (CAN 1)
		p->canmsg[i].maxretryct = 4; //
		p->canmsg[i].bits = 0;       //
		p->canmsg[i].can.dlc = 8;    // Default payload size (might be modified when loaded and sent)
	}

	/* Preload fixed data for sending cell reading msgs. */
	for (i = 0; i < MAXNUMCELLMSGS; i++)
	{	
		/* Preload CAN IDs */
		p->canmsg[CID_MSG_CELLV01 + i].can.id = p->lc.cid_msg_bms_cellvsmr;

		/* Msg identification = cell readings. */
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[0]  = CMD_CMD_TYPE1;//

		/* Preload cell_number-1 for first payload in cell reading msg. */
		// 0,3,6,9,12,15 in upper nibble; lower order nibble (seq number) = 0;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[1] = ((i*3) << 4); 
	}

	/* Preload fixed data for sending "misc" readings. */
	p->canmsg[CID_CMD_MISC].can.id = p->lc.cid_msg_bms_cellvsmr;
	p->canmsg[CID_CMD_MISC].can.cd.uc[2] = p->ident_onlyus;

	/* Pre-load a dummy CAN msg request (from EMC) for sending heartbeat CAN msg. */
	can_hb.id       = p->lc.cid_uni_bms_emc_i;
	can_hb.cd.ull   = 0; // Clear entire payload
	can_hb.cd.uc[0] = CMD_CMD_TYPE1;  // request code (initial, changed later)
	can_hb.cd.uc[4] = p->lc.cid_msg_bms_cellvsmr >>  0; // Our CAN ID
	can_hb.cd.uc[5] = p->lc.cid_msg_bms_cellvsmr >>  8;
	can_hb.cd.uc[6] = p->lc.cid_msg_bms_cellvsmr >> 16;
	can_hb.cd.uc[7] = p->lc.cid_msg_bms_cellvsmr >> 24;

	return;
}
/* *************************************************************************
 * void StartCanComm(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartCanComm(void* argument)
{
	struct BQFUNCTION* p = &bqfunction;
	struct CANRCVBUF* pcan;
	uint32_t noteval;
	uint32_t timeoutwait;
	uint32_t timeoutnext;
	uint8_t code;

	/* CAN communications parameter init. */
	CanComm_init(p);

	cancomm_items_init();

	extern CAN_HandleTypeDef hcan1;
	HAL_CAN_Start(&hcan1); // CAN1

osDelay(20); // Wait for ADCTask to get going.

	rdyflag_cancomm = 1; // Initialization complete and ready
	timeoutnext = xTaskGetTickCount(); // Set starting tick reference
/* ******************************************************************* */
	for (;;)
	{
		/* Wait for notifications, or timeout for heart beat */
		/* Keep from accumulating delays in heart beat rate. */
		timeoutnext += bqfunction.hbct_k; // duration between HB (ticks)
		timeoutwait = xTaskGetTickCount() - timeoutnext;
		xTaskNotifyWait(0,0xffffffff, &noteval,1000);//timeoutwait);

/* ******* CAN msg request for sending CELL VOLTAGES. */
			// Code for which modules should respond bits [7:6]
	   		// 11 = All modules respond
       		// 10 = All modules on identified string respond
       		// 01 = Only identified string and module responds
       		// 00 = spare; no response expected
		if ((noteval & CANCOMMBIT00)  != 0) // cid_cmd_bms_cellvq
		{  // Send cell voltages, but first get present readings
morse_trap(6666);			
			pcan = &p->pmbx_cid_cmd_bms_cellvq_emc->ncan.can;
			code = pcan->cd.uc[0] & 0xC0; // Extract identification code
			if (((code == (3 << 6))) ||
				((code == (2 << 6)) && ((pcan->cd.uc[0] & 0x30) == p->ident_string)) ||
				((code == (1 << 6)) && ((pcan->cd.uc[0] & 0x3F) == p->ident_onlyus)) )
			{ // Here, respond to request, otherwise, ignore
				// But get readings before sending msgs.
				CanComm_qreq(REQ_READBMS, 1, CANCOMMBIT04); // Queue request
			}			
		}
		if ((noteval & CANCOMMBIT07)!= 0)
		{
morse_trap(6661);			
			pcan = &p->pmbx_cid_cmd_bms_cellvq_pc->ncan.can;
			code = pcan->cd.uc[0] & 0xC0; // Extract identification code
			if (((code == (3 << 6))) ||
				((code == (2 << 6)) && ((pcan->cd.uc[0] & 0x30) == p->ident_string)) ||
				((code == (1 << 6)) && ((pcan->cd.uc[0] & 0x3F) == p->ident_onlyus)) )
			{ // Here, respond to request, otherwise, ignore
				// But get readings before sending msgs.
				CanComm_qreq(REQ_READBMS, 1, CANCOMMBIT04); // Queue request
			}
		}
/* ******* CAN msg request for sending MISC READINGS. */
			// Code for which modules should respond bits [7:6]
	   		// 11 = All modules respond
       		// 10 = All modules on identified string respond
       		// 01 = Only identified string and module responds
       		// 00 = spare; no response expected
		if ((noteval & CANCOMMBIT01) != 0) // cid_cmd_bms_miscq
		{
morse_trap(5555);
			/* Get present readings. */
			pcan = &p->pmbx_cid_cmd_bms_miscq_emc->ncan.can;
			code = pcan->cd.uc[0] & 0xC0; // Extract identification code
			if (((code == (3 << 6))) ||
				((code == (2 << 6)) && ((pcan->cd.uc[0] & 0x30) == p->ident_string)) ||
				((code == (1 << 6)) && ((pcan->cd.uc[0] & 0x3F) == p->ident_onlyus)) )
			{ // Here, respond to request, but get readings before sending msgs.
				can05 = *pcan; // Save while BMSTask request take place
				CanComm_qreq(REQ_READBMS, 2, CANCOMMBIT05); // Queue request
			}
		}
		if ((noteval & CANCOMMBIT08) != 0) // cid_cmd_bms_miscq
		{
//morse_trap(5551);
			/* Get present readings. */
			pcan = &p->pmbx_cid_cmd_bms_miscq_pc->ncan.can;
			code = pcan->cd.uc[0] & 0xC0; // Extract identification code
			if (((code == (3 << 6))) ||
				((code == (2 << 6)) && ((pcan->cd.uc[0] & 0x30) == p->ident_string)) ||
				((code == (1 << 6)) && ((pcan->cd.uc[0] & 0x3F) == p->ident_onlyus)) )
			{ // Here, respond to request, but get readings before sending msgs.
				can05 = *pcan; // Save while BMSTask request take place
				CanComm_qreq(REQ_READBMS, 2, CANCOMMBIT05); // Queue request
			}
		}		
/* ******* CAN msg to all nodes. */
		if ((noteval & CANCOMMBIT02) != 0) // CAN id: cid_uni_bms_i
		{ // 
			pcan = &p->pmbx_cid_uni_bms_emc_i->ncan.can;
			if (pcan->cd.uc[0] == LDR_RESET)
			{ // Execute a RESET
				#define SCB_AIRCR 0xE000ED0C
				*(volatile unsigned int*)SCB_AIRCR = (0x5FA << 16) | 0x4;	// Cause a RESET
				while (1==1);
			}
		}
/* ******* CAN msg to all nodes. */
		if ((noteval & CANCOMMBIT09) != 0) // CAN id: cid_uni_bms_pc_i
		{ //       
			pcan = &p->pmbx_cid_uni_bms_pc_i->ncan.can;
			if (pcan->cd.uc[0] == LDR_RESET)
			{ // Execute a RESET
				#define SCB_AIRCR 0xE000ED0C
				*(volatile unsigned int*)SCB_AIRCR = (0x5FA << 16) | 0x4;	// Cause a RESET
				while (1==1);
morse_trap(5);
			}
		}

/* ******* Timeout notification. */
		if (noteval == 0)
		{ // Send heartbeat, but first get present readings.
//morse_trap(34);	
			CanComm_qreq(REQ_READBMS, 0, CANCOMMBIT03);		
 		}	
		if ((noteval & CANCOMMBIT03) != 0) // BMSREQ_Q complete: heartbeat
		{ // Timeoutout BMS request has been completed.
			/* Use dummy CAN msg, then it looks the same as a request CAN msg. */
			can_hb.cd.uc[0] = CMD_CMD_TYPE2;  // Misc subcommands code
			can_hb.cd.uc[1] = MISCQ_STATUS;   // status code
			cancomm_items_uni_bms(&can_hb,&bqfunction.cal_filt[0]); // filtered

			/* Use dummy CAN msg, then it looks the same as a request CAN msg. */
			can_hb.cd.uc[0] = CMD_CMD_TYPE1;  // cell readings code
			can_hb.cd.uc[1] = (hbseq & 0x0f); // Group sequence number
			cancomm_items_uni_bms(&can_hb, &bqfunction.cal_filt[0]); // filtered

			/* Increment 4 bit CAN msg group sequence counter .*/
			hbseq += 1;
   			xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);   
		}
		if ((noteval & CANCOMMBIT04) != 0) // BMSREQ_Q complete: Send cell voltage
		{
			cancomm_items_uni_bms(&can_hb, &bqfunction.cellv[0]);
		}		
		if ((noteval & CANCOMMBIT05) != 0) // BMSREQ_Q complete: Send misc reading command
		{
			cancomm_items_sendcmdr(&can05);	
		}		
		if ((noteval & CANCOMMBIT06) != 0) // BMSREQ_Q complete: Multi-purpose incoming command		
		{
			cancomm_items_uni_bms(&p->pmbx_cid_uni_bms_emc_i->ncan.can, NULL);

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
     (128+32), NULL, taskpriority,\
     &CanCommHandle);
	if (ret != pdPASS) return NULL;

	return CanCommHandle;
}
/* *************************************************************************
 * static void canfilt(uint16_t mm, struct MAILBOXCAN* p);
 * @brief	: Setup CAN hardware filter with CAN addresses to receive
 * @param	: p    = pointer to ContactorTask
 * @param   : mm = morse_trap numeric number
 * *************************************************************************/
static void canfilt(uint16_t mm, struct MAILBOXCAN* p)
{
/*	HAL_StatusTypeDef canfilter_setup_add32b_id(uint8_t cannum, CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint8_t  fifo );
 @brief	: Add a 32b id, advance bank number & odd/even
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: id    = 32b CAN id
 * @param	: fifo  = fifo: 0 or 1
 * @return	: HAL_ERROR or HAL_OK
*/
	HAL_StatusTypeDef ret;	
	ret = canfilter_setup_add32b_id(1,&hcan1,p->ncan.can.id,0);
	if (ret == HAL_ERROR) morse_trap(mm);	
	return;
}
