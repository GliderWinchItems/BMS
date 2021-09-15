/******************************************************************************
* File Name          : can_iface.c
* Date First Issued  : 05-28-2015
* Board              : F103 or F4
* Description        : Interface CAN FreeRTOS to STM32CubeMX HAL can driver 
*******************************************************************************/
/*
01/02/2019 - Hack "can_driver" to inferface with STM32CubeMX FreeRTOS HAL CAN driver

Instead of a common CAN msg block pool for all CAN modules, this version has separate
linked lists for each CAN module for TX.  RX0, RX1 use a common FreeRTOS queue.  
This simplifies the issue of disabling of interrupts

06/02/2016 - Add rejection of loading bogus CAN ids.

06/14/2015 rev 720: can.driver.[ch] replaced with can.driverR.[ch] and 
  old can.driver[ch] deleted from svn.
*/

/* The following sends all outgoing CAN msgs back into FreeRTOS CAN receive queue */
//#define CANMSGLOOPBACKSALL

#ifdef CHEATINGONHAL
#include "stm32f407.h" 	// **** CHEATING (processor dependent) ****
#endif

#include <malloc.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_can.h"
#include "can_iface.h"
#include "DTW_counter.h"

/* Debugging */
#include "morse.h"
extern struct CAN_CTLBLOCK* pctl0;
extern struct CAN_CTLBLOCK* pctl1;
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;


/* Abort feature--which may hang TX! */
#define YESABORTCODE

/* Uncomment to cause TX msgs bit set to appear as received after sent. */
#define CANMSGLOOPBACKALL

/* subroutine declarations */
static void loadmbx2(struct CAN_CTLBLOCK* pctl);
static void moveremove2(struct CAN_CTLBLOCK* pctl);

#define MAXCANMODULES	4	// Max number of CAN modules + 1
/* Pointers to control blocks for each CAN module */
static struct CAN_CTLBLOCK* pctllist[MAXCANMODULES];
static struct CAN_CTLBLOCK** ppctllist = NULL;	// Pointer to end of active pctllist

/* *************************************************************************
 * static void canmsg_compress(struct CANRCVBUF *pcan, CAN_RxHeaderTypeDef *phal, uint8_t *pdat);
 * @brief	: Convert silly HAL expanded format to hardware compressed format
 * @param	: pcan = pointer to useful hardware format output
 * @param	: phal = pointer to HAL header input
 * @param	: pdat = pointer to HAL payload data array input
 * *************************************************************************/
static void canmsg_compress(struct CANRCVBUF *pcan, CAN_RxHeaderTypeDef *phal, uint8_t *pdat)
{
	if (phal->IDE != 0)
	{ // Extended 29b CAN id
		pcan->id = phal->IDE | (phal->ExtId << 3);
	}
	else
	{ // Standard 11b CAN id
		pcan->id = phal->StdId << 21;
	}
	pcan->id |= phal->RTR;
	
	pcan->dlc = (phal->DLC & 0xf);

	pcan->cd.uc[0] = *(pdat+0);
	pcan->cd.uc[1] = *(pdat+1);
	pcan->cd.uc[2] = *(pdat+2);
	pcan->cd.uc[3] = *(pdat+3);
	pcan->cd.uc[4] = *(pdat+4);
	pcan->cd.uc[5] = *(pdat+5);
	pcan->cd.uc[6] = *(pdat+6);
	pcan->cd.uc[7] = *(pdat+7);
	return;
}
/******************************************************************************
 * struct CANTAKEPTR* can_iface_add_take(struct CAN_CTLBLOCK*  pctl);
 * @brief 	: Create a 'take' pointer for accessing CAN msgs in the circular buffer
 * @param	: pctl = pointer to our CAN control block
 * @return	: pointer to pointer pointing to 'take' location in circular CAN buffer
 * 			:  NULL = Failed 
*******************************************************************************/
struct CANTAKEPTR* can_iface_add_take(struct CAN_CTLBLOCK*  pctl)
{
	struct CANTAKEPTR* p;
	
taskENTER_CRITICAL();
	/* Get one measily pointer */
	p = (struct CANTAKEPTR*)calloc(1, sizeof(struct CANTAKEPTR));
	if (p == NULL){ taskEXIT_CRITICAL();return NULL;}

	/* Initialize the pointer to current add location of the circular buffer. */
   /* Given 'p', the beginning, end, and location CAN msgs are being added
      can be accessed. */
	p->pcir  = &pctl->cirptrs;

	/* Start the 'take' pointer at the position in the circular buffer where
      CAN msgs are currently being added. */
	p->ptake = pctl->cirptrs.pwork;

taskEXIT_CRITICAL();
	return p;
}
/******************************************************************************
 * struct CANTAKEPTR* can_iface_mbx_init(struct CAN_CTLBLOCK*  pctl, osThreadId tskhandle, uint32_t notebit);
 * @brief 	: Initialize the mailbox task notification and get a 'take pointer for it.
 * @param	: tskhandle = task handle that will be used for notification; NULL = use current task
 * @param	: notebit = notification bit if notifications used
 * @return	: pointer to pointer pointing to 'take' location in circular CAN buffer 
*******************************************************************************/
struct CANTAKEPTR* can_iface_mbx_init(struct CAN_CTLBLOCK* pctl, osThreadId tskhandle, uint32_t notebit)
{
	if (tskhandle == NULL)
	{ // Here, use the current running Task
		tskhandle = xTaskGetCurrentTaskHandle();
	}

	/* Notification of CAN msgs added to the circular buffer are only for one task. */
	pctl->tsknote.tskhandle = tskhandle;
	pctl->tsknote.notebit   = notebit;

	/* The 'add' pointer was setup in 'can_iface_init' below */
	
	/* Get a 'take' pointer into the circular buffer */
	return can_iface_add_take(pctl);
}
/******************************************************************************
 * struct CANRCVBUFN* can_iface_get_CANmsg(struct CANTAKEPTR* p);
 * @brief 	: Get a pointer to the next available CAN msg and step ahead in the circular buffer
 * @brief	: p = pointer to struct with 'take' and 'add' pointers
 * @return	: pointer to CAN msg struct; NULL = no msgs available.
*******************************************************************************/
 struct CANRCVBUFN* can_iface_get_CANmsg(struct CANTAKEPTR* p)
{
	struct CANRCVBUFN* ptmp = NULL;
	if (p->pcir->pwork == p->ptake) return ptmp;

	ptmp = p->ptake;
	p->ptake += 1;
	if (p->ptake == p->pcir->pend) p->ptake = p->pcir->pbegin;

	return ptmp;	
}
/******************************************************************************
 * struct CAN_CTLBLOCK* can_iface_init(CAN_HandleTypeDef *phcan, uint8_t canidx, uint16_t numtx, uint16_t numrx);
 * @brief 	: Setup linked list for TX priority sorted buffering
 * @param	: phcan = Pointer "handle" to HAL control block for CAN module
 * @param	: cannum = CAN module index, CAN1 = 0, CAN2 = 1, CAN3 = 2
 * @param	: numtx = number of CAN msgs for TX buffering
 * @param	: numrx = number of incoming (and loopback) CAN msgs in circular buffer
 * @return	: Pointer to our knows-all control block for this CAN
 *		:  NULL = calloc failed
 *		:  Pointer->ret = pointer to CAN control block for this CAN unit
*******************************************************************************/
/*
'main' initialization makes a call to this routine for each CAN module, i.e. CAN1, CAN2, CAN3.
Return is a pointer to the control block.  Since the unmodified STM32CubeMX routines only
pass their CAN module "handle" (pointer) upon interrupt a lookup is required to obtain
the pointer to the buffers.  Therefore, these pointers are also saved.
*/
struct CAN_CTLBLOCK* can_iface_init(CAN_HandleTypeDef *phcan, uint8_t canidx, uint16_t numtx, uint16_t numrx)
{
	int i;

	struct CAN_CTLBLOCK*  pctl;
	struct CAN_CTLBLOCK** ppx;

	struct CAN_POOLBLOCK* plst;
	struct CAN_POOLBLOCK* ptmp;

	struct CANRCVBUFN* pcann;

taskENTER_CRITICAL();
	/* Get a control block for this CAN module. */
	pctl = (struct CAN_CTLBLOCK*)calloc(1, sizeof(struct CAN_CTLBLOCK));
	if (pctl == NULL){ taskEXIT_CRITICAL();return NULL;}

	/* Add HAL CAN control block "handle" to our control block */
	pctl->phcan = phcan; 

	/* Save CAN module index (CAN1 = 0). */
	pctl->canidx = canidx;

	/* Add new control block to list of control blocks */
	if (ppctllist != NULL) // Not first time?
	{ // Yes. Check for duplicates, i.e. check for bozo programmers
		ppx = &pctllist[0];	// NOTE: don't confuse ppctllist with pctllist
		while (ppx != ppctllist)
		{
			if ( (*ppx)->phcan == phcan)
			{
				taskEXIT_CRITICAL();
				return NULL; // Duplicate
			}
			ppx++;;
		}
	}
	else
	{
		ppctllist = &pctllist[0];
	}
	/* Save control block pointer on list, and advance list pointer */
	*ppctllist = pctl;	
	ppctllist++;
	if (ppctllist == &pctllist[MAXCANMODULES]) { taskEXIT_CRITICAL();return NULL;} //JIC too many entries
	
	/* Now that we have control block in memory, we can use it to return errors. 
	   by setting the error code in pctl->ret. */

	/* Get CAN xmit linked list. */	
	if (numtx == 0)  {pctl->ret = -1; return pctl;} // Bogus tx buffering count
	ptmp = (struct CAN_POOLBLOCK*)calloc(numtx, sizeof(struct CAN_POOLBLOCK));
	if (ptmp == NULL){pctl->ret = -2; taskEXIT_CRITICAL(); return NULL;} // Get buff failed

	/* Initialize links.  All are in the "free" list. */
	// Item: the last block is left with NULL in plinknext
	plst = &pctl->frii;
	for (i = 0; i < numtx; i++)
	{
		plst->plinknext = ptmp;
		plst = ptmp++;
	}

	/* Setup circular buffer for receive CAN msgs */
	if (numrx == 0)  {pctl->ret = -3; return pctl;} // Bogus rx buffering count
	pcann = (struct CANRCVBUFN*)calloc(numrx, sizeof(struct CANRCVBUFN));
	if (pcann == NULL){pctl->ret = -4; taskEXIT_CRITICAL(); return NULL;} // Get buff failed

	/* Initialize pointers for "add"ing CAN msgs to the circular buffer */
	pctl->cirptrs.pbegin = pcann;
	pctl->cirptrs.pwork  = pcann;
	pctl->cirptrs.pend   = pcann + numrx;

	/* NOTE: pctl->tsknote gets initialized
      when 'MailboxTask' calls 'can_iface_mbx_init' */

taskEXIT_CRITICAL();

	return pctl;	// Return pointer to control block
}
/******************************************************************************
 * int can_driver_put(struct CAN_CTLBLOCK* pctl,struct CANRCVBUF *pcan,uint8_t maxretryct,uint8_t bits);
 * @brief	: Get a free slot and add CAN msg
 * @param	: pctl = pointer to control block for this CAN modules
 * @param	: pcan = pointer to msg: id, dlc, data (common_can.h)
 * @param	: maxretryct =  0 = use TERRMAXCOUNT; not zero = use this value.
 * @param	: bits = Use these bits to set some conditions (see .h file)
 * @return	:  0 = OK; 
 *				: -1 = Buffer overrun (no free slots for the new msg)
 *				: -2 = Bogus CAN id rejected
 *				: -3 = control block pointer NULL
 ******************************************************************************/

extern uint32_t debugTX1c;

int can_driver_put(struct CAN_CTLBLOCK* pctl,struct CANRCVBUF *pcan,uint8_t maxretryct,uint8_t bits)
{
	volatile struct CAN_POOLBLOCK* pnew;
	volatile struct CAN_POOLBLOCK* pfor; 	// Loop pointer for the 'for’ loop.

	if (pctl == NULL) return -3;

	/* Reject CAN msg if CAN id is "bogus". */
	// If 11b is specified && bits in extended address are present it is bogus
	if (((pcan->id & CAN_ID_EXT) == 0) && ((pcan->id & CAN_EXTENDED_MASK) != 0))
	{
		pctl->bogusct += 1;
		return -2;
	}

	/* Get a free block from the free list. */
//	disable_TXints(pctl, save);	// TX interrupt might move a msg to the free list.
	taskENTER_CRITICAL();

	pnew = pctl->frii.plinknext;
	if (pnew == NULL)
	{ // Here, either no free list blocks OR this TX reached its limit
//		reenable_TXints(save);
		taskEXIT_CRITICAL();
		pctl->can_errors.can_msgovrflow += 1;	// Count overflows
		return -1;	// Return failure: no space & screwed
	}	
	pctl->frii.plinknext = pnew->plinknext;

//	reenable_TXints(save);

	/* 'pnew' now points to the block that is free (and not linked). */

	/* Build struct/block for addition to the pending list. */
	// retryct    xb[0]  // Counter for number of retries for TERR errors
	// maxretryct xb[1]  // Maximum number of TERR retry counts
	// bits	     xb[2]  // Use these bits to set some conditions (see below)
	// nosend     xb[3]  // Do not send: 0 = send; 1 = do NOT send on CAN bus (internal use only)
	pnew->can     = *pcan;	// Copy CAN msg.
	pnew->x.xb[1] = maxretryct;	// Maximum number of TERR retry counts
	pnew->x.xb[2] = bits;// Use these bits to set some conditions (see .h file)
	pnew->x.xb[3] = 0;   // not used for now
	pnew->x.xb[0] = 0;   // Retry counter for TERRs

	/* Find location to insert new msg.  Lower value CAN ids are higher priority, 
           and when the CAN id msg to be inserted has the same CAN id as the 'pfor' one
           already in the list, then place the new one further down so that msgs with 
           the same CAN id do not get their order of transmission altered. */
//	disable_TXints(pctl, save);

	for (pfor = &pctl->pend; pfor->plinknext != NULL; pfor = pfor->plinknext)
	{
		if (pnew->can.id < (pfor->plinknext)->can.id) // Pay attention: "value" vs "priority"
			break;
	}

	/* Add new msg to pending list. (TX interrupt is still disabled) */
	pnew->plinknext = pfor->plinknext; 	// Insert new msg into 
	pfor->plinknext = pnew;			//   pending list.

	if (pctl->pxprv == NULL) // Is sending complete?
	{ // pxprv == NULL means CAN mailbox did not get loaded, so CAN is idle.
		loadmbx2(pctl); // Start sending
	}
	else
	{ // CAN sending is in progress.
		if ((pctl->pxprv)->plinknext == pnew) // Does pxprv need adjustment?
		{ // Here yes. We inserted a msg between 'pxprv' and 'pxprv->linknext'
			pctl->pxprv = pnew;	// Update 'pxprv' so that it still points to msg TX using.
			pctl->can_errors.can_pxprv_fwd_one += 1;	// Count: Instances that pxprv was adjusted in 'for' loop
		}
		/* Check if new msg is higher CAN priority than msg in mailbox */
#ifdef CHEATINGONHAL
		if ( (pctl->pend.plinknext)->can.id < (pctl->phcan->Instance->sTxMailBox[0] & ~0x1)  )
#else
		if ( (pctl->pend.plinknext)->can.id < (pctl->mbx0 & ~0x1)  ) // Use mailbox shadow id
#endif
		{ // Here, new msg has higher CAN priority than msg in mailbox
/* &&&&&&&&&&&&&& BEGIN ABORT MODS &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
#ifdef YESABORTCODE
			pctl->abortflag = 1;	// Set flag for interrupt routine use
		taskEXIT_CRITICAL(); // ==> NOTE: allow interrupts before setting abort!
			HAL_CAN_AbortTxRequest(pctl->phcan, CAN_TX_MAILBOX0);
//		taskEXIT_CRITICAL(); // ==> AFTER! Which fails!
			return 0;
#endif
		}
/* &&&&&&&&&&&&&& END ABORT MODS &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
	}
	taskEXIT_CRITICAL(); // Re-enable interrupts
	return 0;	// Success!
}
/*---------------------------------------------------------------------------------------------
 * static void loadmbx2(struct CAN_CTLBLOCK* pctl)
 * @brief	: Load mailbox
 ----------------------------------------------------------------------------------------------*/
static void loadmbx2(struct CAN_CTLBLOCK* pctl)
{
	uint32_t uidata[2];
	uint32_t TxMailbox;
	CAN_TxHeaderTypeDef halmsg;

	volatile struct CAN_POOLBLOCK* p = pctl->pend.plinknext;

	if (p == NULL)
	{
		pctl->pxprv = NULL;
		return; // Return if no more to send
	}

	pctl->pxprv = &pctl->pend;	// Save in a static var

#ifdef CHEATINGONHAL
	/* Load the mailbox with the message.  CAN ID low bit starts xmission. */
	pctl->phcan->sTxMailBox[0].TDTR = p->can.dlc;	 	// CAN_TDT0R:  mailbox 0 time & length
	pctl->phcan->sTxMailBox[0].TDLR = p->can.cd.ui[0];	// CAN_TDL0RL: mailbox 0 data low  register
	pctl->phcan->sTxMailBox[0].TDHR = p->can.cd.ui[1];	// CAN_TDL0RH: mailbox 0 data low  register
	/* Load CAN ID with TX Request bit set */
	pctl->phcan->sTxMailBox[0].TIR = (p->can.id | 0x1); 	// CAN_TI0R:   mailbox 0 identifier register
#else
	/* Expand hardware friendly format to HAL format (which gets changed back to hardware friendly) */
	halmsg.StdId = (p->can.id >> 21);
	halmsg.ExtId = (p->can.id >>  3);
	halmsg.IDE   = (p->can.id & CAN_ID_EXT);
	halmsg.RTR   = (p->can.id & CAN_RTR_REMOTE);
	halmsg.DLC   = (p->can.dlc & 0xf);
	uidata[0]   = p->can.cd.ui[0];
	uidata[1]   = p->can.cd.ui[1];
	pctl->mbx0  = p->can.id;	// Shadow MBX0 ID
   HAL_CAN_AddTxMessage(pctl->phcan, &halmsg, (uint8_t*)uidata, &TxMailbox);
#endif
	return;
}
/* --------------------------------------------------------------------------------------
* static void moveremove2(struct CAN_CTLBLOCK* pctl);
* @brief	: Remove msg from pending list and add to free list
  --------------------------------------------------------------------------------------- */
static void moveremove2(struct CAN_CTLBLOCK* pctl)
{
	volatile struct CAN_POOLBLOCK* pmov;
//	uint32_t save[2];

//	disable_TXints(pctl, save);	// TX or RX(other) interrupts might remove a msg from the free list.
// Each CAN module has its own linked list and RX0,1 does not use the linked list, so disabling interrupts is not needed.

	/* Remove from pending; move to free list. */
	pmov = pctl->pxprv->plinknext;	// Pts to removed item
	pctl->pxprv->plinknext = pmov->plinknext;

	// Adding to free list
	pmov->plinknext = pctl->frii.plinknext; 
	pctl->frii.plinknext  = pmov;

//	reenable_TXints(save);
	return;
}

/*#######################################################################################
 * ISR CAN Callback routines
 *####################################################################################### */


/* *********************************************************************
 * struct CAN_CTLBLOCK* getpctl(CAN_HandleTypeDef *phcan);
 * @brief	: Look up CAN control block pointer, given 'MX CAN handle from callback
 * @param	: phcan = pointer to 'MX CAN handle (control block)
 * @return	: Pointer to our CAN control bock
 * *********************************************************************/
extern CAN_HandleTypeDef hcan1;
struct CAN_CTLBLOCK* getpctl(CAN_HandleTypeDef *phcan)
{
//if (pctl == pctl1) morse_trap(73);
//if (phcan != &hcan1) while(1==1);

	struct CAN_CTLBLOCK** ppx = &pctllist[0];
	while (ppx != ppctllist) // Step through list of pointers 
	{
		if ( (*ppx)->phcan == phcan) break;
		ppx++;
	}
	return *ppx;
}

/* Transmission Mailbox 0 complete callback. */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *phcan)
{
	struct CAN_CTLBLOCK* pctl = getpctl(phcan); // Lookup our pointer

	/* Loop back CAN =>TX<= msgs. */
volatile	struct CAN_POOLBLOCK* p = pctl->pxprv->plinknext;//pctl->pend.plinknext;
	struct CANRCVBUFN ncan;
	ncan.pctl = pctl;
	ncan.can = p->can;
	
	/* Either loop back all, or msg-by-msg select loopback */
#ifndef CANMSGLOOPBACKALL
	// Check of loopback bit in msg is set
	if ( (p->x.xb[2] & CANMSGLOOPBACKBIT) != 0)
#endif
   {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			*pctl->cirptrs.pwork = ncan;
			pctl->cirptrs.pwork++;
			if (pctl->cirptrs.pwork == pctl->cirptrs.pend) pctl->cirptrs.pwork = pctl->cirptrs.pbegin;

			if (pctl->tsknote.tskhandle != NULL)
			{ // Here, one task will be notified a msg added to circular buffer
				xTaskNotifyFromISR(pctl->tsknote.tskhandle,\
					pctl->tsknote.notebit, eSetBits,\
					&xHigherPriorityTaskWoken );
			}
	}

	moveremove2(pctl);	// remove from pending list, add to free list
	pctl->abortflag = 0;
	loadmbx2(pctl);		// Load mailbox 0.  Mailbox should be available/empty.
//portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); // Trigger scheduler
}

/* Transmission Mailbox 0 Abort callback. */
void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *phcan)
{
#ifdef YESABORTCODE
	struct CAN_CTLBLOCK* pctl = getpctl(phcan);
	loadmbx2(pctl);		// Load mailbox 0.  Mailbox should be available/empty.
	pctl->abortflag = 0;
#endif
}

/* Error callback */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *phcan)
{
	struct CAN_CTLBLOCK* pctl = getpctl(phcan);
	if ((phcan->ErrorCode & HAL_CAN_ERROR_TX_ALST0) != 0 )
	{
		pctl->can_errors.can_tx_alst0_err += 1; // Running ct of arb lost: Mostly for debugging/monitoring
		if ((pctl->pxprv->plinknext->x.xb[2] & SOFTNART) != 0)
		{ // Here this msg was not to be re-sent, i.e. NART
			moveremove2(pctl);	// Remove msg from pending queue
		}
debugTX1c += 1;
	}
	else if ((phcan->ErrorCode & HAL_CAN_ERROR_TX_TERR0) != 0 )
	{
		pctl->pxprv->plinknext->x.xb[0] += 1;	// Count errors for this msg
		if (pctl->pxprv->plinknext->x.xb[0] > pctl->pxprv->plinknext->x.xb[1])
		{ // Here, too many error, remove from list
			pctl->can_errors.can_tx_bombed += 1;	// Number of bombouts
			moveremove2(pctl);	// Remove msg from pending queue
		}
	}	
	loadmbx2(pctl);		// Load mailbox 0.  Mailbox should be available/empty.
	return;
}
/* *********************************************************************
 * static void unloadfifo(CAN_HandleTypeDef *phcan, uint32_t RxFifo);
 * @brief	: Empty FIFOx hardware buffer of msgs and place on queue
 * @param	: phcan = pointer to 'MX CAN handle (control block)
 * @return	: Pointer to our CAN control bock
 * *********************************************************************/
uint32_t debug1;

static void unloadfifo(CAN_HandleTypeDef *phcan, uint32_t RxFifo)
{
	struct CANRCVBUFN ncan; // CAN msg plus pctl
	ncan.toa = DTWTIME;
debug1 += 1;
	HAL_StatusTypeDef ret;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	CAN_RxHeaderTypeDef header;
	uint8_t data[8];

	struct CAN_CTLBLOCK* pctl = getpctl(phcan); // Lookup pctl given phcan

/* This is placed here as part of the debugging of the USB startup
   producing a Hard_Fault. The stack dump showed that R0 was loaded
   with an addresses above the sram limit. */
if (pctl == NULL) morse_trap (557);
//if (pctl == NULL) while(1==1);

	do /* Unload hardware RX FIFO */
	{
// NOTE: this could be done directly and avoid the expand/compress overhead
// but it would become processor dependent and would cheat on HAL.
		ret = HAL_CAN_GetRxMessage(pctl->phcan, RxFifo, &header, &data[0]);
		if (ret == HAL_OK)
		{
			/* Setup msg with pctl for our format */
			ncan.pctl = pctl;
			canmsg_compress(&ncan.can, &header, &data[0]);

			/* Place on queue for Mailbox task to filter, distribute, notify, etc. */
			*pctl->cirptrs.pwork = ncan; // Copy struct
			pctl->cirptrs.pwork++;       // Advance 'add' pointer
			if (pctl->cirptrs.pwork == pctl->cirptrs.pend) pctl->cirptrs.pwork = pctl->cirptrs.pbegin;

			if (pctl->tsknote.tskhandle != NULL)
			{ // Here, notify one task a new msg added to circular buffer
				xTaskNotifyFromISR(pctl->tsknote.tskhandle,\
					pctl->tsknote.notebit, eSetBits,\
					&xHigherPriorityTaskWoken );
			}
		}
	} while (ret == HAL_OK); //JIC there is more than one in the hw fifo
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); // Trigger scheduler
	return;
}
/* Rx FIFO 0 message pending callback. */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *phcan)
{
	unloadfifo(phcan, CAN_RX_FIFO0);
	return;
}
/* Rx FIFO 1 message pending callback. */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *phcan)
{
	unloadfifo(phcan, CAN_RX_FIFO1);
	return;
}
