/******************************************************************************
* File Name          : cancomm_items.c
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#include "can_iface.h"
#include "CanTask.h"
#include "BQTask.h"
#include "cancomm_items.h"
#include "CanCommTask.h"
#include "adcparams.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"
#include "ADCTask.h"
#include "iir_f1.h"
#include "BMSTask.h"
#include "morse.h"

void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf);

static void loadfloat(uint8_t* puc, float* f);
static void status_group(void);
static void send_bms_array(struct CANRCVBUF* pcan, float* pout, uint8_t n);
static void send_allfets(struct CANRCVBUF* pcan);

static uint8_t skip;
static struct BMSREQ_Q  bmstask_q_readbms;

/* *************************************************************************
 * static void cancomm_items_q(uint8_t reqcode);
 * @brief	: Queue request (for BMSTask handling)
 * *************************************************************************/
static void cancomm_items_q(uint8_t reqcode)
{
return;
	BaseType_t ret;
	uint32_t noteval1;
	struct BMSREQ_Q* pq = &bmstask_q_readbms;

	bmstask_q_readbms.reqcode = reqcode;
	bmstask_q_readbms.noteyes = 0; // Do not notify calling task
	bmstask_q_readbms.done = 1; // Show request queued
	ret = xQueueSendToBack(BMSTaskReadReqQHandle, &pq, 0);
	if (ret != pdPASS) morse_trap(201);

	xTaskNotifyWait(0,0xffffffff, &noteval1, 3000);
	if (noteval1 == 0) morse_trap(202);
}
/* *************************************************************************
 * void cancomm_items_init(void);
 * @brief	: Initialization
 * *************************************************************************/
void cancomm_items_init(void)
{
	bmstask_q_readbms.bmsTaskHandle = xTaskGetCurrentTaskHandle();
	bmstask_q_readbms.tasknote = CANCOMMITEMSNOTE00;
	return;
}
/* *************************************************************************
 * static void returncmd(struct CANTXQMSG* pmsg);
 *	@brief	: Copy request bytes
 *  @param  : pmsg = pointer to CAN msg
 * *************************************************************************/
#ifdef USE_RETURNCMD
static void returncmd(struct CANTXQMSG* pmsg, struct CANRCVBUF* pcan)
{
	pmsg->can.cd.uc[1] = (pcan->cd.uc[0] & 0xC0) | (bqfunction.ident_onlyus);
	pmsg->can.cd.uc[1] = pcan->cd.uc[1]; // Command code
	return;
}
#endif
/* *************************************************************************
 * static void loaduint32(uint8_t* puc, uint32_t n);
 *	@brief	: Prepare and send a response to a received command
 *  @param  : puc = pointer to CAN msg payload
 *  @param  : n = unit32_t to be loaded
 * *************************************************************************/
static void loaduint32(uint8_t* puc, uint32_t n)
{
	*(puc+0) = n >>  0;
	*(puc+1) = n >>  8;
	*(puc+2) = n >> 16;
	*(puc+3) = n >> 24;
	return;
}
/* *************************************************************************
 * static void loadfloat(uint8_t* puc, float* pf);
 *	@brief	: Prepare and send a response to a received command
 *  @param  : puc = pointer to CAN msg payload
 *  @param  : pf = pointer to float to be loaded
 * *************************************************************************/
static void loadfloat(uint8_t* puc, float* pf)
{
	union UF
	{
		uint8_t uc[4];
		float f;
	}uf;

	uf.f = *pf;
	*(puc+0) = uf.uc[0];
	*(puc+1) = uf.uc[1];
	*(puc+2) = uf.uc[2];
	*(puc+3) = uf.uc[3];
	return;
}
/* *************************************************************************
 * void cancomm_items_uni_bms(struct CANRCVBUF* pcan, float* pf);
 *	@brief	: UNIversal multi-purpose command (CANCOMMBIT02)
 *  @param  : pcan = pointer to struct CANRCVBUF with request CAN msg
 *  @param  : pf = pointer to array for output
 * *************************************************************************/
void cancomm_items_uni_bms(struct CANRCVBUF* pcan, float* pf)
{
	struct BQFUNCTION* p = &bqfunction;
	uint32_t canid; // CANID requested to respond, if applicable
	uint8_t code;

	 /* Extract CAN id for unit to respond. */
    canid = (pcan->cd.uc[4] << 0)|(pcan->cd.uc[5] << 8)|
            (pcan->cd.uc[6] <<16)|(pcan->cd.uc[7] <<24);

	// Code for which modules should respond: bits [7:6]
	// 11 = All modules respond
    // 10 = All modules on identified string respond
    // 01 = Only identified string and module responds
    // 00 = spare; no response expected
	code = pcan->cd.uc[2] & 0xC0;

    // Respond if CAN ID for this node is in request
	if (!(((code == (3 << 6))) ||
		  ((code == (2 << 6)) && ((pcan->cd.uc[2] & (3 << 4)) == p->ident_string)) ||
		  ((code == (1 << 6)) && ((pcan->cd.uc[2] & 0x0F) == p->ident_onlyus)) ||
		  ((canid == p->lc.cid_msg_bms_cellvsmr))))
		return; // Skip. This request is not for us.
    // Simplified for TEST
	//if (canid != p->lc.cid_msg_bms_cellvsmr) return;

	/* Set up response to command. */
	switch(pcan->cd.uc[0])
	{
	case CMD_CMD_TYPE1: // Send Cell readings
		cancomm_items_q(REQ_READBMS); // Read cells + GPIO 1 & 2
		cancomm_items_sendcell(pcan, pf);
		break;

	case CMD_CMD_TYPE2: // Respond according to the code
		cancomm_items_sendcmdr(pcan);
		break;

	case CMD_CMD_TYPE3: // spare types
	case CMD_CMD_TYPE4:
		break;
	}
	return;
}
/* *************************************************************************
 * void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf);
 *	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 *  @param  : pf = pointer cell array
 * *************************************************************************/
	/* Note: The CAN tx queue is a separate task, so until that task 
	   copies the msg into its (priority sorted) array, the CAN msg
	   has to remain unchanged. Hence the following loads six CAN
	   msgs. This routine will not get called again until the EMC
	   sends a request for a CAN msg readout. 
	*/
void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf)
{
	struct BQFUNCTION* p = &bqfunction;
	uint8_t i;
	uint8_t j;

	/* Load CAN msg payload with three cell readings, or codes that is a voltage 
		   reading that is not possible. */
	for (i = 0; i < MAXNUMCELLMSGS; i++) // 6 CAN msgs are sent.
	{	
		// DLC is the same for all
		p->canmsg[CID_MSG_CELLV01 + i].can.dlc = 8;

		// Set sequence number sent by requesting CAN msgs
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[1] = (pcan->cd.uc[1] & 0x0f) | ((i*3) << 4);

		/*
			Three cells per CAN msg, and 18 possible readings are sent 
			regardless of the number of installed cells (which will be
			18,16, or maybe 12)*/
		for (j = 0; j < 3; j++) // Load 
		{
			if (p->lc.cellpos[i*3+j] == CELLNONE)
			{ // Here, cell was not installed in this position. Set code.
				p->canmsg[CID_MSG_CELLV01 + i].can.cd.us[j+1] = 65534;//CELLVNONE;
			}
			else
			{ // Here, cell is installed
				if ((bqfunction.cellvopenbits & (1 << (i*3+j))) != 0)
				{ // Here, unexpected open cell.
					p->canmsg[CID_MSG_CELLV01 + i].can.cd.us[j+1] = CELLVOPEN;
				}
				else
				{ // Cell voltage is in normal range.
					p->canmsg[CID_MSG_CELLV01 + i].can.cd.us[j+1] = (uint16_t)(*(pf+j) * 10000);
				}
			}
		}
		xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_MSG_CELLV01 + i],4);
		pf += 3;
	}
	return;
}
/* *************************************************************************
 * void cancomm_items_sendcmdr(struct CANRCVBUF* pcan);
 *	@brief	: Prepare and send a response to a received command
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/
/*
CANID_CMD_BMS_MISCQ'
payload [0] U8: Module identification
    [7:6] 
       11 = All modules respond
       10 = All modules on identified string respond
       01 = Only identified string and module responds
       00 = spare; no response expected
    [5:4] Battery string number (0 – 3) (string #1 - #4)
    [3:0] Module number (0 – 7) (module #1 - #16)

payload [1] U8: Command code
    0 = reserved for heartbeat
    1 = status
    2 = cell voltage: calibrated
    3 = cell voltage: adc counts
    4 = temperature sensor: calibrated
    5 = temperature sensor: adc counts
    6 = isolated dc-dc converter output voltage
    7 = charger hv voltage
    8 = Hall sensor: calibrated
    9 = Hall sensor: adc counts
  10 = Highest cell voltage
  11 = Lowest cell voltage
  12 = FET on/off discharge bits
*/ 
void cancomm_items_sendcmdr(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;
	float ftmp[ADCDIRECTMAX];
	uint8_t i;

	/* Pointer to payload 4 byte value is used often. */
	uint8_t* puc = &p->canmsg[CID_CMD_MISC].can.cd.uc[4];

	skip = 0;

	/* Data in payload is always X4 (4 bytes, any format) */
	p->canmsg[CID_CMD_MISC].can.dlc = 8;
	
/*	Return what was requested: copy as a uint32_t since,
	byte-at-a-time is slow and fattening, e.g.,
	p->canmsg[CID_CMD_MISC].can.cd.uc[0] = CMD_CMD_TYPE2;
	// Code for response
	p->canmsg[CID_CMD_MISC].can.cd.uc[1] = pcan->cd.uc[1];
	// Module identification
	p->canmsg[CID_CMD_MISC].can.cd.uc[2] = pcan->cd.uc[2];
	// Item number, or thermistor number, or ...
	p->canmsg[CID_CMD_MISC].can.cd.uc[3] = pcan->cd.uc[3]; */
	p->canmsg[CID_CMD_MISC].can.cd.ui[0] = pcan->cd.ui[0];

	/* Add string and module number to response. It may or
	   may not have been included in the request. */
	p->canmsg[CID_CMD_MISC].can.cd.uc[2] = 
			(p->canmsg[CID_CMD_MISC].can.cd.uc[2] & 0xC0) | 
			((p->lc.stringnum-1) << 4) | (p->lc.modulenum -1);

	/* Command code. */
	switch(p->canmsg[CID_CMD_MISC].can.cd.uc[1])
	{
	case MISCQ_STATUS:      // 1 status
		status_group();
		break;

 	case MISCQ_CELLV_CAL:   // 2 cell voltage: calibrated
		cancomm_items_q(REQ_READBMS); // Read cells + GPIO 1 & 2
 		send_bms_array(pcan, &bqfunction.cal_filt[0], p->lc.ncell);
 		break;

 	case MISCQ_CELLV_ADC:   // 3 cell voltage: adc counts
 		send_bms_array(pcan, &p->raw_filt[0], p->lc.ncell);
 		break;

 	case MISCQ_TEMP_CAL:    // 4 temperature sensor: calibrated
		cancomm_items_q(REQ_TEMPERATURE); // Read cells + GPIO 1 & 2
 		send_bms_array(pcan, &bqfunction.cal_filt[16], 3);
 		break;

 	case MISCQ_TEMP_ADC:    // 5 temperature sensor: adc counts
 		send_bms_array(pcan, &p->raw_filt[16], 3);
 		break;

 	case MISCQ_DCDC_V:      // 6 isolated dc-dc converter output voltage
 		loadfloat(puc, &adc1.abs[ADC1IDX_PA4_DC_DC].filt);
 		break;

 	case MISCQ_CHGR_V:      // 7 charger hv voltage
 		loadfloat(puc, &adc1.abs[ADC1IDX_PA7_HV_DIV].filt);
 		break;

 	case MISCQ_HALL_CAL:    // 8 Hall sensor: calibrated
 		skip = 1;
 		break;

 	case MISCQ_HALL_ADC:    // 9 Hall sensor: adc counts
 		skip = 1;
 		break;

 	case MISCQ_CELLV_HI:   // 10 Highest cell voltage
 		p->canmsg[CID_CMD_MISC].can.cd.uc[2] = p->cellx_high;
 		loaduint32(puc, p->cellv_high);
 		break;

 	case MISCQ_CELLV_LO:   // 11 Lowest cell voltage
 		p->canmsg[CID_CMD_MISC].can.cd.uc[2] = p->cellx_low;
 		loaduint32(puc, p->cellv_low);
 		break;

 	case MISCQ_FETBALBITS: // 12 FET on/off discharge bits
 		loaduint32(puc,bqfunction.cellbal);
		break;

	case MISCQ_TOPOFSTACK: // BMS top-of-stack voltage
		send_bms_array(pcan, &bqfunction.cal_filt[19], 1);
		break;		

 	case MISCQ_PROC_CAL: // Processor ADC calibrated readings
 		for (i = 0; i < ADCDIRECTMAX; i++) // Copy struct items to float array
 			ftmp[i] = adc1.abs[i].filt;
 		ftmp[1] = adc1.common.degC; // Insert special intermal temperature calibration 
 		send_bms_array(pcan, &ftmp[0], ADCDIRECTMAX);
 		break;

 	case MISCQ_PROC_ADC: // Processor ADC raw adc counts for making calibrations
		for (i = 0; i < ADCDIRECTMAX; i++) // Copy struct items to float array
 			ftmp[i] = adc1.abs[i].sumsave;
		send_bms_array(pcan, &ftmp[0], ADCDIRECTMAX); 	
 		break;

	case MISCQ_R_BITS:      // 21 Dump, dump2, heater, discharge bits
		send_allfets(pcan);
 		break;

	case MISCQ_CURRENT_CAL: // 24 Below cell #1 minus, current resistor: calibrated
		skip = 1;
 		break;

	case MISCQ_CURRENT_ADC: // 25 Below cell #1 minus, current resistor: adc counts		
		skip = 1;
 		break;

	}
	if (skip == 0)
	{ // Here, single CAN msg has not been queued for sending
		/* Queue CAN msg response. */
		xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);
	}
	return;
}
/* *************************************************************************
 * static void send_bms_array(struct CANRCVBUF* pcan, float* pout, uint8_t n);
 *	@brief	: Prepare and send a series of CAN msgs 
 *  @param  : pcan = pointer to CAN msg requesting response
 *  @param  : pout = pointer to output array of floats for first reading
 *  @param  : n = number of readings (CAN msgs) to send
 * *************************************************************************/
static void send_bms_array(struct CANRCVBUF* pcan, float* pout, uint8_t n)
{
	struct BQFUNCTION* p = &bqfunction;
	uint8_t i;

	for (i = 0; i < n; i++)
	{
		p->canmsg[CID_CMD_MISC].can.cd.uc[3] = i;

		// Reading into 4 byte payload
		loadfloat(&p->canmsg[CID_CMD_MISC].can.cd.uc[4], pout);

		// Queue CAN msg
		xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);
		pout += 1;
	}
	skip = 1;
	return;
}
/* *************************************************************************
 * static void send_bms_array(struct CANRCVBUF* pcan, float* pout, uint8_t n);
 *	@brief	: Prepare and send CAN msgs for FETs
 *  @param  : pcan = pointer to CAN msg requesting response
 * *************************************************************************/
/*
Send three CAN msgs with fet and cell wiring status.

payload [3] Defines payload data in payload [4-7]:
  0 = all fets (1 = ON status)
    [4-6] cells #1 - #18: bits 0-17
    [7] 
    	#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
		#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
		#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
		#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
		#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

  1 = unexpected open cells likely (1 = open wire)
    [4-7] cells #1 - #18: bits 0-17

  2 = installed cells (1 = installed)
      [4-7] cells #1 - #18: bits 0-17
*/
static void send_allfets(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;

	/* FET status: (see cellball.c) */
	p->canmsg[CID_CMD_MISC].can.cd.uc[3]  = 0;	
	p->canmsg[CID_CMD_MISC].can.cd.ui[1]  = p->cellbal;
	// Add Dump, Dump2, Heater, trickle FET status. */
	p->canmsg[CID_CMD_MISC].can.cd.uc[7]  = p->fet_status;
	xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);	

	/* Unexpected open cell voltage (see cellball.c) */
	p->canmsg[CID_CMD_MISC].can.cd.uc[3]  = 1;	
	p->canmsg[CID_CMD_MISC].can.cd.ui[1]  = p->cellvopenbits;
	xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);	

	/* Cells installed (see bq_idx_v_struct.c,h) */
	p->canmsg[CID_CMD_MISC].can.cd.uc[3]  = 2;	
	p->canmsg[CID_CMD_MISC].can.cd.ui[1]  = p->cellspresent;
	xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);	

	skip = 1;
	return;
}
/* *************************************************************************
 * static void status_group(void);
 *	@brief	: Load data for status
 * *************************************************************************/
static void status_group(void)
{
/* Status bits (see BQTask.h)
Battery--
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 5v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Charging in progress
#define BSTATUS_DUMPTOV   (1 << 6)  // Dump to a voltage in progress
FETS--
#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate
*/

	struct BQFUNCTION* p = &bqfunction;

	// Reserved byte
	p->canmsg[CID_CMD_MISC].can.cd.uc[3] = 0;
	// Status bytes (U8)
	p->canmsg[CID_CMD_MISC].can.cd.uc[4] = p->battery_status;
	p->canmsg[CID_CMD_MISC].can.cd.uc[5] = p->fet_status;

	return;
}

