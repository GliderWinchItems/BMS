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
#include "DTW_counter.h"
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

extern uint32_t rtcregs_status; // 'main' saves rtc registers upon startup
extern uint32_t rtcregs_morse_code;
extern uint8_t rtcregs_OK; // 1 = rtc regs were OK; 0 = not useable.

extern struct CAN_CTLBLOCK* pctl0; // Pointer to CAN1 control block
extern CAN_HandleTypeDef hcan1;

static void do_req_codes(struct CANRCVBUF* pcan);
static uint8_t q_do(struct CANRCVBUF* pcan);



//static void canfilt(uint16_t mm, struct MAILBOXCAN* p);

#ifdef TEST_WALK_DISCHARGE_FET_BITS // See main.h
uint8_t dischgfet; // Test fet bit (0-17)
uint8_t walkdelay; // noteval == 0 counter delay
#endif

TaskHandle_t CanCommHandle;

struct CANRCVBUF  can_hb; // Dummy heart-beat request CAN msg
uint8_t hbctr; // Cell group seq number for dummy CAN msg

uint8_t rdyflag_cancomm; // Initialization complete and ready = 1

/* Delayed queue requests. 
When an incoming CAN msg requests a response that requires a reading
from the '1818 a request is queued to BMSTask and the incoming CAN msg
is saved along with a notification bit. The CanCommTask loop
continues and other requests could take place. When BMSTask completes
a queued request it notifies CanCommTask and the notification bit is
used to lookup the CAN msg that initiated the BMSTask request. 
*/
/* xTaskNotifyWait Notification bits */
#define CANCOMMBIT00 (1 <<  0) // EMC1 CAN msg
#define CANCOMMBIT01 (1 <<  1) // PC   CAN msg
#define CANCOMMBIT02 (1 <<  2) // EMC2 CAN msg

// The following reserves notification bits 7-12 (out of 0-31)
#define CANQEDSIZE 5   // Max number of BMSTask requests that can be queued
#define CANCOMMQUEUE 7 // Notification bit shift offset
// Mask notification word for bits assigned to queued linked list items
#define CANCOMMQUEUE_MASK (((1<<CANQEDSIZE)-1)<<CANCOMMQUEUE)

/* CANQED associates BMSTask queue request with requesting CAN msg. */
struct CANQED
{
	struct BMSREQ_Q bmsreq_c;
	struct BMSREQ_Q* pbmsreq_c;
	struct CANRCVBUF can;
	uint32_t time; // Check for failure to get notification
	uint8_t busy;
};
struct CANQED canqed[CANQEDSIZE];

static uint32_t canqed_time;

// Index for checking timeouts notification failures
uint8_t idxhb;



// Number of RTOS ticks for notification wait timeout
#define TIMEOUTPOLLRATE 4 // Four per second
#define TIMEOUTPOLL (1000/TIMEOUTPOLLRATE) // RTOS ticks between wait timeouts

/* *************************************************************************
 * static struct CANQED* canqedadd(void);
 *	@brief	: Get pointer to next circular buffer position
 *  @return : NULL = buffer full, otherwise pointer to position to be filled
 * *************************************************************************/
static struct CANQED* canqedadd(void)
{
		int i;
	// Search for first not-busy array item
	for (i = 0; i<CANQEDSIZE; i++)
	{
		if (canqed[i].busy == 0)
			break;
	}
	// None available check
	if (i >= CANQEDSIZE)
	{
		bqfunction.warning = 655;
morse_trap(655);
	 	return NULL;
	}
	return &canqed[i]; // Return ptr to position to be filled
}
/* *************************************************************************
 * static uint8_t for_us(struct CANRCVBUF* pcan, struct BQFUNCTION* p);
 *	@brief	: Check if CAN msg request is for this unit
 *  @param  : pcan = point to CAN msg struct
 *  @return : 0 = yes, not 0 = no
 * *************************************************************************/
static uint8_t for_us(struct CANRCVBUF* pcan, struct BQFUNCTION* p)
{
	uint32_t canid;
	uint8_t code;
	 /* Extract CAN id for unit to respond. */
    canid = (pcan->cd.uc[4] << 0)|(pcan->cd.uc[5] << 8)|
            (pcan->cd.uc[6] <<16)|(pcan->cd.uc[7] <<24);

	/* Extract code from CAN msg.  */ 
	// Code for which modules should respond: bits [7:6]
	// 11 = All modules respond
    // 10 = All modules on identified string respond
    // 01 = Only identified string and module responds
    // 00 = reserved        
	code = pcan->cd.uc[1] & 0xC0; // Extract identification code

    // Does this CAN node qualify for a response?
	if  ((((code == (3 << 6))) ||
		  ((code == (2 << 6)) && ((pcan->cd.uc[2] & (3 << 4)) == p->ident_string)) ||
/*		  ((code == (1 << 6)) && ((pcan->cd.uc[2] & 0x0F) == p->ident_onlyus)) || */
		  ((canid == p->lc.cid_msg_bms_cellvsmr))))
	{
		return 0; // Yes, respond.	
	}
	return 1; // Skip. This request is not for us.
}
/* *************************************************************************
 *  void CanComm_qreq(uint8_t reqcode, uint32_t setfets, uint8_t fetsw, struct CANRCVBUF* pcan);
 *	@brief	: Queue request to BMSTask.c
 *  @param  : reqcode = see BMSTask.h 
 *  @param  : setfets = bits to set discharge fets (if so commanded)
 *  @param  : fetsw = bits to turn fets on|off during reading
 *  @param  : idx = this queue request slot
 *  @param  : notebit = notification bit when BMSTask completes request
 * *************************************************************************/
void CanComm_qreq(uint8_t reqcode, uint32_t setfets, uint8_t fetsw, struct CANRCVBUF* pcan)
{
	struct CANQED* pcanqed;

	/* Get a pointer to a free position. */
    pcanqed = canqedadd();
    // NULL means buffer is full.
    if (pcanqed == NULL)
    {
morse_trap(654);    	
    	return; // Screwed.
	}

    pcanqed->busy = 1;
    pcanqed->time = DTWTIME;

	/* Setup request for BMSTask. */
	pcanqed->can = *pcan; // Save requesting CAN msg
	pcanqed->bmsreq_c.reqcode  = reqcode; // BMSTask request code
	pcanqed->bmsreq_c.setfets  = setfets; // Discharge fet bits to set
	pcanqed->bmsreq_c.fetsw    = fetsw;   // FET on|off bits (DUMP, etc.)
    pcanqed->bmsreq_c.noteyes  = 1; // Yes, we will wait for notification

    /* Code that sets rate for ADC conversion. */
    if ((pcan->cd.uc[2] >> 4) > 7)
    { // Keep within table lookup bounds
    	pcanqed->bmsreq_c.rate = 4; // 7 KHz normal mode
    }
    else
    {
    	pcanqed->bmsreq_c.rate = (pcan->cd.uc[2] >> 4);
    }

    // Timeout check for missing notifications (debugging?)
	canqed_time = xTaskGetTickCount(); // Update time
 
    // Queue request for BMSTask.c
 	int ret = xQueueSendToBack(BMSTaskReadReqQHandle, &pcanqed->pbmsreq_c, 0);
    if (ret != pdPASS) morse_trap(650);	
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
    p->pmbx_cid_uni_bms_emc1_i      = MailboxTask_add(pctl0,p->lc.cid_uni_bms_emc1_i, NULL, CANCOMMBIT00,0,U8); // 
    if (p->pmbx_cid_uni_bms_emc1_i == NULL) morse_trap(622);

    p->pmbx_cid_uni_bms_emc2_i      = MailboxTask_add(pctl0,p->lc.cid_uni_bms_emc2_i, NULL, CANCOMMBIT02,0,U8); // 
    if (p->pmbx_cid_uni_bms_emc2_i == NULL) morse_trap(622);

    p->pmbx_cid_uni_bms_pc_i      = MailboxTask_add(pctl0,p->lc.cid_uni_bms_pc_i,   NULL, CANCOMMBIT01,0,U8); // 
    if (p->pmbx_cid_uni_bms_pc_i == NULL) morse_trap(622);

    /* Add CAN msgs to incoming CAN hw filter. (Skip to allow all incoming msgs. */
 //   canfilt(603, p->pmbx_cid_uni_bms_emc_i);
 //   canfilt(603, p->pmbx_cid_uni_bms_pc_i);

	/* Pre-load fixed data ib CAN msg to be sent. */
	p->canmsg.pctl = pctl0;   // Control block for CAN module (CAN 1)
	p->canmsg.maxretryct = 4; //
	p->canmsg.bits       = 0; //
	p->canmsg.can.cd.ull = 0; // Clear playload jic
	p->canmsg.can.dlc    = 8; // Default payload size (might be modified when loaded and sent)
	p->canmsg.can.id     = p->lc.cid_msg_bms_cellvsmr; // 

	/* Pre-load a dummy CAN msg request for sending heartbeat CAN msg. 
	   This looks like an incoming CAN msg is polling this unit. */
	can_hb.id       = CANID_UNIT_99; // Dummy ID to signify as a heartbeat
	can_hb.dlc      = 8;
	can_hb.cd.ull   = 0; // Clear entire payload
	can_hb.cd.uc[0] = CMD_CMD_CELLPOLL;  // request code (initial, changed later)
	can_hb.cd.ui[1] = p->lc.cid_msg_bms_cellvsmr;
	
	/* Circular buffer for saving CAN msg for queued BMSTask requests */
	/* Pre-load fixed data into array for queuing BMSTask requests. */
	for (i = 0; i < CANQEDSIZE; i++)
	{
		// Notification: task handle
		canqed[i].bmsreq_c.bmsTaskHandle = xTaskGetCurrentTaskHandle();
		// Pointer to request struct since BMSTask queue is pointers.
		canqed[i].pbmsreq_c = &canqed[i].bmsreq_c;
		// Notification bit 
		canqed[i].bmsreq_c.tasknote  = (1 << (CANCOMMQUEUE + i));
		// Show not busy
		canqed[i].busy = 0;
		// 
		canqed[i].time = (DTWTIME + 8000000);
	}
	idxhb = 0; // Index for checking timeouts of notification failure
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
	struct CANQED* pcanqed;
	uint32_t noteval;
	uint32_t timeoutwait;
	int i;

	/* CAN communications parameter init. */
	CanComm_init(p);
	cancomm_items_init();

	extern CAN_HandleTypeDef hcan1;
	HAL_CAN_Start(&hcan1); // Start CAN1

osDelay(20); // Wait for ADCTask to get going.

	rdyflag_cancomm = 1; // Initialization complete and ready

	/* Send the rtc register status. */
/* Use dummy CAN msg, then it looks the same as a request CAN msg. */
	can_hb.cd.uc[0] = CMD_CMD_TYPE2; 
	can_hb.cd.uc[2] = MISCQ_MORSE_TRAP; // status code
	cancomm_items_sendcmdr(&can_hb);  // Handles as if incoming CAN msg.	

/* In the following 'for (;;)' loop xTaskNotifyWait will time out if there are no 
   incoming CAN msg notifications. The timeout is set to poll the timeouts for the
   two hearbeat msgs. The heartbeat status msg will be sent even when CAN msgs are
   coming in. The timeout for the heartbeat that sends cell readings is reset each
   time a set (of six) CAN msgs with cell readings is sent. The TIMEOUTPOLL sets
   the RTOS ticks to wair for the polling. */ 
	p->HBstatus_ctr = xTaskGetTickCount(); // Count RTOS ticks for hearbeat timing: status msg
	p->HBcellv_ctr  = p->HBstatus_ctr; // Count RTOS ticks for hearbeat timing: cellv msg
	timeoutwait     = TIMEOUTPOLL; // TaskNotifyWait timeout

/* ******************************************************************* */
	for (;;)
	{

/* PCB board discharge fet testing. */
#ifdef TEST_WALK_DISCHARGE_FET_BITS  // See main.h for #define
		timeoutwait = 789; // Avoid keep-alive calls disrupting fets
#endif		

/* Using noteval instead of 0xffffffff would take care of the possibility
of a BMSTask request completing before xTaskNotifyWait was entered. If BMSTask
completed before xTaskNotifyWait was entered using 0xfffffff would clear the 
notification and it would be lost. */

		xTaskNotifyWait(0,0xffffffff, &noteval, timeoutwait);
//		xTaskNotifyWait(0,noteval, &noteval, timeoutwait);//timeoutwait);
		
/* ******* CAN msg to all nodes. EMC poll msg. */
		if ((noteval & CANCOMMBIT00) != 0) // CAN id: cid_uni_bms_emc1_i [B0000000]
		{ // 
//morse_trap(777);
			pcan = &p->pmbx_cid_uni_bms_emc1_i->ncan.can;
			if (for_us(pcan,p) == 0)
			{ // This CAN msg includes us.
				do_req_codes(pcan);
			}
		}

		if ((noteval & CANCOMMBIT02) != 0) // CAN id: cid_uni_bms_emc2_i [B0200000]
		{ // 
//morse_trap(777);
			pcan = &p->pmbx_cid_uni_bms_emc2_i->ncan.can;
			if (for_us(pcan,p) == 0)
			{ // This CAN msg includes us.
				do_req_codes(pcan);
			}
		}

/* ******* CAN msg to all nodes. PC poll msg. */
		if ((noteval & CANCOMMBIT01) != 0) // CAN id: cid_uni_bms_pc_i [AEC00000]
		{ //   
//morse_trap(55);
			pcan = &p->pmbx_cid_uni_bms_pc_i->ncan.can;
			if (for_us(pcan,p) == 0)
			{ // This CAN msg includes us.
				do_req_codes(pcan);
			}
		}

/* ******* Heartbeat timing: status */
		if 	((int)(xTaskGetTickCount() - p->HBstatus_ctr) > 0)
		{
			p->HBstatus_ctr += p->hbct_k;
			{
			/* Use dummy CAN msg, then it looks the same as a request CAN msg. */
				can_hb.cd.ull   = 0xffffffff; // Clear entire payload
				can_hb.cd.uc[0] = CMD_CMD_TYPE2; // 
				can_hb.cd.uc[2] = MISCQ_STATUS;   // status code
				cancomm_items_sendcmdr(&can_hb);  // Handles as if incoming CAN msg.
			}	
		}			
/* ******* Heartbeat timing: cell readings */
		if 	((int)(xTaskGetTickCount() - p->HBcellv_ctr) > 0)
		{
			p->HBcellv_ctr = xTaskGetTickCount() + p->hbct_k; // Next HB
			// HBcellv_ctr set to (current + increment) each time cell readings sent.
		/* Use dummy CAN msg, then it looks the same as a request CAN msg. */
			/* Get new cell readings. Queue a BMS cell readings request. */
			can_hb.cd.uc[0] = CMD_CMD_CELLPOLL; // Cell readings request
			can_hb.cd.uc[2] = ((RATE26HZ << 8) | (0x0F & hbctr++));
			do_req_codes(&can_hb); // Cell readings will queue a BMSTask request
		}
#if 0
/* ******* Timeout notification. (Heartbeat) */
		if (noteval == 0)
		{ // Send heartbeat: status and six with cell readings
#ifndef TEST_WALK_DISCHARGE_FET_BITS // See main.h for #define 
		/* Normal hearbeat CAN msgs. */			
			/* Use dummy CAN msg, then it looks the same as a request CAN msg. */
			/* Send current status. */
			can_hb.cd.uc[0] = CMD_CMD_MISCHB; // Misc subcommands code
			can_hb.cd.uc[1] = MISCQ_STATUS;   // status code
			cancomm_items_sendcmdr(&can_hb);  // Handles as if incoming CAN msg.

			/* Get new cell readings. Queue a BMS cell readings request. */
			can_hb.cd.uc[0] = CMD_CMD_CELLPOLL; // 
			do_req_codes(&can_hb);
#else
	/* Step through discharge bits for testing. */
			walkdelay += 1;
			if (walkdelay > 10)
			{ // Use noteval == 0 to time delays
				walkdelay = 0;
				// Step to next FET position
				dischgfet += 1;
				if (dischgfet >= 18) dischgfet = 0;
			}
			// Set fet bit position to be turned on.
			can_hb.cd.uc[0] = CMD_CMD_TYPE2; // 
			can_hb.cd.uc[1] = MISCQ_SETFETBITS;
			can_hb.cd.ui[1] = (1 << dischgfet);
			do_req_codes(pcan);
#endif			
 		}	
#endif 	

	/* Check for queue requests that have timed out. 
	a failure means something with BMSTask notification failed. */
		// Check one entry each pass through
		if (((int)(DTWTIME - canqed[idxhb].time) > 8000000) && (canqed[idxhb].busy != 0))
		{ // Here, queued item is over 1/2 second old
			bqfunction.warning = (656);
			canqed[idxhb].busy = 0;
//	morse_trap(656);			
		}
		idxhb += 1; if (idxhb >= CANQEDSIZE) idxhb = 0;	

/* ******* BMSTask notification of queued item completed */		
/* ******* Check notification bits for a queued BMSTask completion. */
		i = 0;
		pcanqed = &canqed[0];
		while (((noteval & CANCOMMQUEUE_MASK) != 0) && (i < CANQEDSIZE))
 		{ 
			if (pcanqed->busy != 0)
			{
				if (pcanqed->bmsreq_c.tasknote == (noteval & (1 << (CANCOMMQUEUE + i))))
				{ // We found the BMSTask request completion
					pcanqed->busy = 0; // Set array position not busy
					noteval &= ~(1 << (CANCOMMQUEUE + i)); // Removed notification bit
					cancomm_items_sendcmdr(&(pcanqed->can)); // Handle request
				}
			}
			i += 1;
			pcanqed += 1;
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
     (128+64), NULL, taskpriority,\
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
#if 0
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
#endif

/* *************************************************************************
 * static uint8_t toosoon(struct CANRCVBUF* pcan);
 *	@brief	: If requests are too close together, don't respond
 *  @param  : pcan = point to CAN msg struct
 *  @return : 0 = Not too soon (so OK to respond); 1 = do not respond
 * *************************************************************************/
struct TOOSOON
{
	uint32_t cant;
	uint32_t cellt;
};
#define TOOSOONNEXT 40 // Minimum time (ms) between CAN msgs
#define TOOSOONSIZE 2
#define TS_EMC 0  // EMC
#define TS_PC  1  // PC
#if 0
struct TOOSOON toosoon[TOOSOONSIZE];

static uint8_t toosoonchk(struct CANRCVBUF* pcan)
{
	if ((pcan->id == bqfunction.lc.cid_uni_bms_emc1_i) &&
		(((int)(xTaskGetTickCount() - toosoon[TS_EMC].cant)) > 0))
	{ // EMC incoming CAN msg
		toosoon[TS_EMC ].cant = xTaskGetTickCount()+TOOSOONNEXT;
		return 0;
	}
	if ((pcan->id == bqfunction.lc.cid_uni_bms_pc_i) &&
		(((int)(xTaskGetTickCount() - toosoon[TS_PC].cant)) > 0))
	{ // PC incoming CAN msg
		// Set next time count that will allow a queued rerquest.
		toosoon[TS_PC].cant = xTaskGetTickCount()+TOOSOONNEXT;
		return 0;
	}
	if (pcan->id == bqfunction.lc.cid_msg_bms_cellvsmr)
	{ // Hearbeat dummy msg
		return 0;
	}
	// Note: this would catch a bogus CAN ID
	return 1; // Don't respond
};
#endif
/* *************************************************************************
 * static void do_req_codes(struct CANRCVBUF* pcan);
 *	@brief	: Respond to the CAN msg request code
 *  @param  : pcan = point to CAN msg struct
 * *************************************************************************/
static void do_req_codes(struct CANRCVBUF* pcan)
{
	/* First payload byte holds root request code. */
	switch (pcan->cd.uc[0])
	{
	case LDR_RESET: // Execute a RESET ###############################
		#define SCB_AIRCR 0xE000ED0C
		*(volatile unsigned int*)SCB_AIRCR = (0x5FA << 16) | 0x4;// Cause a RESET
//		while (1==1);
		break; // Redundant

	case CMD_CMD_CELLPOLL: // (42) Queue BSMTask read, then send cells when read completes
		// If sufficient time between requests, queue a BMSTask read. If
		// the readings are not stale BMSTask will do an immediate notification.
//		if (toosoonchk(pcan) == 0)
			// Delay a heartbeat
			CanComm_qreq(REQ_READBMS, 0, 0, pcan);
		break;

	case CMD_CMD_TYPE2: // (43) Misc: Some may not need queueing BMSTask
		if (q_do(pcan) == 0) // Queue BMSTask request if applicable to command
		{ // Here, queueing not needed, so handle request now.
			cancomm_items_sendcmdr(pcan);
		}
		break;

	default:
		if (pcan->cd.uc[0] > LDR_CHKSUM)
		{
			bqfunction.warning = 551;
morse_trap(551);
		}
		break;
	}
	return;
}
/* *************************************************************************
 * static uint8_t q_do(struct CANRCVBUF* pcan);
 *	@brief	: Queue a BMS task if necessary
 *  @param  : pcan = point to CAN msg struct\
 *  @return : 0 = No need for queuing; 1 = queue
 * *************************************************************************/
static uint8_t q_do(struct CANRCVBUF* pcan)
{
	uint8_t qsw = 0;
	/* Command code. */
	switch(pcan->cd.uc[1])
	{
	/* Group not requiring a BMSTask reading */
	case MISCQ_STATUS:      // 1 status
 	case MISCQ_CELLV_ADC:   // 3 cell voltage: adc counts 		send_bms_array(po, &p->raw_filt[0], p->lc.ncell);
 	case MISCQ_DCDC_V:      // 6 isolated dc-dc converter output voltageloadfloat(puc, &adc1.abs[ADC1IDX_PA4_DC_DC].filt);
 	case MISCQ_CHGR_V:      // 7 charger hv voltage loadfloat(puc, &adc1.abs[ADC1IDX_PA7_HV_DIV].filt);

	case MISCQ_TOPOFSTACK: // BMS top-of-stack voltage		send_bms_array(po, &bqfunction.cal_filt[19], 1);
 	case MISCQ_PROC_CAL: // Processor ADC calibrated readings
 	/*
 		for (i = 0; i < ADCDIRECTMAX; i++) // Copy struct items to float array
 			ftmp[i] = adc1.abs[i].filt;
 		ftmp[1] = adc1.common.degC; // Insert special internal temperature calibration 
 		send_bms_array(po, &ftmp[0], ADCDIRECTMAX);
	*/
 	case MISCQ_PROC_ADC: // Processor ADC raw adc counts for making calibrations
	/*	for (i = 0; i < ADCDIRECTMAX; i++) // Copy struct items to float array
 			ftmp[i] = adc1.abs[i].sumsave;
		send_bms_array(po, &ftmp[0], ADCDIRECTMAX); 	 */
	case MISCQ_R_BITS:      // 21 Dump, dump2, heater, discharge bits
		qsw = 0; // No need to queue a reading
		break;
/* #### RATE7KHZ needs updating for CAN msg request. */
	/* BMSTask: REQ_SETFETS */
 	/* ???? Does this require a read AUX registers? */
	case MISCQ_FETBALBITS: //  27 // Set FET on/off discharge bits
 		CanComm_qreq(REQ_SETFETS, pcan->cd.ui[1], 0, pcan);
 		break;

	/* Uses AUX registers */
 	case MISCQ_TEMP_CAL:    // 4 temperature sensor: calibrated send_bms_array(po, &bqfunction.cal_filt[16], 3);
 	case MISCQ_TEMP_ADC:    // 5 temperature sensor: adc counts send_bms_array(po, &p->raw_filt[16], 3);
	case MISCQ_CURRENT_CAL: // 24 Below cell #1 minus, current resistor: calibrated
	case MISCQ_CURRENT_ADC: // 25 Below cell #1 minus, current resistor: adc counts	
 	case MISCQ_HALL_CAL:    // 8 Hall sensor: calibrated
 	case MISCQ_HALL_ADC:    // 9 Hall sensor: adc counts
 		CanComm_qreq(REQ_READAUX, 0, 0, pcan); // queue BMS read AUX
 		qsw = 1; // Show BMS reading is queued
 		break;

	/* BMSTask: REQ_READBMS */
 	case MISCQ_CELLV_CAL:   // 2 cell voltage: calibrated
 	case MISCQ_CELLV_HI:   // 10 Highest cell voltage 		loaduint32(puc, p->cellv_high);
 	case MISCQ_CELLV_LO:   // 11 Lowest cell voltage 		loaduint32(puc, p->cellv_low);
 		CanComm_qreq(REQ_READBMS, 0, 0, pcan); // queue BMS read CELLS + GPIO 1,2
 		qsw = 1; // Show BMS reading is queued
 		break;
	}
	return qsw;
}