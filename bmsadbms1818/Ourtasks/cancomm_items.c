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
static void status_group(struct CANRCVBUF* po);
static void send_bms_array(struct CANRCVBUF* po, float* pout, uint8_t n);
static void send_allfets(struct CANRCVBUF* po);
static void not_implemented(struct CANRCVBUF* po);

static uint8_t skip;
/* *************************************************************************
 * void cancomm_items_init(void);
 * @brief	: Initialization
 * *************************************************************************/
void cancomm_items_init(void)
{
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
 * void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf);
 *	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 *  @param  : pf = pointer cell array
 * *************************************************************************/
#include "DTW_counter.h"
uint32_t dbgsendcelldtw;
uint32_t dbgsendcellctr;

void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf)
{
dbgsendcelldtw  = DTWTIME;
dbgsendcellctr += 1;

	struct BQFUNCTION* p = &bqfunction;
	uint8_t i;
	uint8_t j;

	// DLC is the same for all
	p->canmsg.can.dlc = 8;

	/* Load CAN msg payload with three cell readings, or codes that is a voltage 
		   reading that is not possible. */
	for (i = 0; i < MAXNUMCELLMSGS; i++) // 6 CAN msgs are sent.
	{	
		// Set group sequence number sent by requesting CAN msgs
		p->canmsg.can.cd.uc[1] = (pcan->cd.uc[2] & 0x0f) | ((i*3) << 4);

		/*	Three cells per CAN msg, and 18 possible readings are sent 
			regardless of the number of installed cells (which will be
			18,16, or maybe 12)*/
		for (j = 0; j < 3; j++) // Load 
		{
			if (p->lc.cellpos[i*3+j] == CELLNONE)
			{ // Here, cell was not installed in this position. Set code.
				p->canmsg.can.cd.us[j+1] = 65534;//CELLVNONE;
			}
			else
			{ // Here, cell is installed
				if ((bqfunction.cellvopenbits & (1 << (i*3+j))) != 0)
				{ // Here, unexpected open cell.
					p->canmsg.can.cd.us[j+1] = CELLVOPEN;
				}
				else
				{ // Cell voltage is in normal range.
					p->canmsg.can.cd.us[j+1] = (uint16_t)(*(pf+j));
				}
			}
		}
		pf += 3;
		xQueueSendToBack(CanTxQHandle,&p->canmsg,4);
	}
   	p->HBcellv_ctr = xTaskGetTickCount() + p->hbct_k; // Next HB for cellv gets delayed	
	return;
}
/* *************************************************************************
 * void cancomm_items_sendcmdr(struct CANRCVBUF* pi);
 *  @brief	: Prepare and send a response to a received command CAN msg
 *  @param  : pi = pointer to incoming CAN msg struct CANRCVBUF from mailbox 
 * *************************************************************************/
void cancomm_items_sendcmdr(struct CANRCVBUF* pi)
{
	struct BQFUNCTION* p = &bqfunction;
	struct CANRCVBUF* po = &p->canmsg.can;
	float ftmp[ADCDIRECTMAX];
	uint8_t i;

	/* Pointer to payload 4 byte value is used often. */
	uint8_t* puc = &po->cd.uc[4];

	skip = 0;

   if (pi->cd.uc[0] == CMD_CMD_CELLPOLL)// (code 42) Request to send cell
   {
   	// Set code in response that identifies who polled
   	 if (pi->id == p->lc.cid_uni_bms_emc_i)
   	 	po->cd.uc[0] = CMD_CMD_CELLEMC; // EMC polled cell voltages
   	 else if (pi->id == p->lc.cid_uni_bms_pc_i)
   	 	po->cd.uc[0] = CMD_CMD_CELLEMC; // PC polled cell voltages
   	 else if (pi->id == p->lc.cid_msg_bms_cellvsmr)
   	 	{
			po->cd.uc[0] = CMD_CMD_CELLHB; // Heartbeat timeout cell voltages
//			bqfunction.hbseq += 1; // Group sequence number
   	 	}  
   	 	else
   	 	{
   	 		// Warning: Unexpectd CAN ID
   	 	}
   	// Send cell voltages: 6 CAN msgs
 	cancomm_items_sendcell(pi, &p->cellv[0]);

 	return;
   }
      	// Set code in response that identifies who polled
   	 if (pi->id == p->lc.cid_uni_bms_emc_i)
   	 	po->cd.uc[0] = CMD_CMD_MISCPC; // EMC polled cell voltages
   	 else if (pi->id == p->lc.cid_uni_bms_pc_i)
   	 	po->cd.uc[0] = CMD_CMD_MISCEMC; // PC polled cell voltages
   	 else if (pi->id == p->lc.cid_msg_bms_cellvsmr)
   	 	{
			po->cd.uc[0] = CMD_CMD_MISCHB; // Heartbeat timeout cell voltages
//			bqfunction.hbseq += 1; // Group sequence number
   	 	}  
   	 	else
   	 	{
   	 		// Warning: Unexpectd CAN ID
   	 	}

	/* Command code. 
	If a BMSTask request was needed this would have been taken care of
	in CanCommTask before this routine is called. To queue a request
	here could result in an endless loop. 
	*/
	switch(pi->cd.uc[1])
	{
	case MISCQ_STATUS:      // 1 status
		status_group(po);
		break;

 	case MISCQ_CELLV_CAL:   // 2 cell voltage: calibrated
//		CanComm_qreq(REQ_READBMS, 0, pi); // Read cells + GPIO 1 & 2
 		break;

 	case MISCQ_CELLV_ADC:   // 3 cell voltage: adc counts
 		send_bms_array(po, &p->raw_filt[0], p->lc.ncell);
 		break;

 	case MISCQ_TEMP_CAL:    // 4 temperature sensor: calibrated
//		CanComm_qreq(REQ_TEMPERATURE, 0, pi); // Read AUX
 		send_bms_array(po, &bqfunction.cal_filt[16], 3);
 		break;

 	case MISCQ_TEMP_ADC:    // 5 temperature sensor: adc counts
 		send_bms_array(po, &p->raw_filt[16], 3);
 		break;

 	case MISCQ_DCDC_V:      // 6 isolated dc-dc converter output voltage
 		loadfloat(puc, &adc1.abs[ADC1IDX_PA4_DC_DC].filt);
 		break;

 	case MISCQ_CHGR_V:      // 7 charger hv voltage
 		loadfloat(puc, &adc1.abs[ADC1IDX_PA7_HV_DIV].filt);
 		break;

 	case MISCQ_HALL_CAL:    // 8 Hall sensor: calibrated
 		not_implemented(po);
 		break;

 	case MISCQ_HALL_ADC:    // 9 Hall sensor: adc counts
 		not_implemented(po);
 		break;

 	case MISCQ_CELLV_HI:   // 10 Highest cell voltage
 		po->cd.uc[2] = p->cellx_high;
 		loaduint32(puc, p->cellv_high);
 		break;

 	case MISCQ_CELLV_LO:   // 11 Lowest cell voltage
 		po->cd.uc[2] = p->cellx_low;
 		loaduint32(puc, p->cellv_low);
 		break;

 	case MISCQ_FETBALBITS: // 12 FET on/off discharge bits
 		loaduint32(puc,bqfunction.cellbal);
		break;

	case MISCQ_TOPOFSTACK: // BMS top-of-stack voltage
		send_bms_array(po, &bqfunction.cal_filt[19], 1);
		break;		

 	case MISCQ_PROC_CAL: // Processor ADC calibrated readings
 		for (i = 0; i < ADCDIRECTMAX; i++) // Copy struct items to float array
 			ftmp[i] = adc1.abs[i].filt;
 		ftmp[1] = adc1.common.degC; // Insert special internal temperature calibration 
 		send_bms_array(po, &ftmp[0], ADCDIRECTMAX);
 		break;

 	case MISCQ_PROC_ADC: // Processor ADC raw adc counts for making calibrations
		for (i = 0; i < ADCDIRECTMAX; i++) // Copy struct items to float array
 			ftmp[i] = adc1.abs[i].sumsave;
		send_bms_array(po, &ftmp[0], ADCDIRECTMAX); 	
 		break;

	case MISCQ_R_BITS:      // 21 Dump, dump2, heater, discharge bits
		send_allfets(po);
 		break;

	case MISCQ_CURRENT_CAL: // 24 Below cell #1 minus, current resistor: calibrated
		not_implemented(po);
 		break;

	case MISCQ_CURRENT_ADC: // 25 Below cell #1 minus, current resistor: adc counts		
		not_implemented(po);
 		break;

	}
	if (skip == 0)
	{ // Here, single CAN msg has not been queued for sending
		/* Queue CAN msg response. */
		xQueueSendToBack(CanTxQHandle, po, 4);
	}
	return;
}
/* *************************************************************************
 * static void send_bms_array(struct CANRCVBUF* po, float* pout, uint8_t n);
 *	@brief	: Prepare and send a series of CAN msgs 
 *  @param  : po = pointer response CAN msg
 *  @param  : pout = pointer to output array of floats for first reading
 *  @param  : n = number of readings (CAN msgs) to send
 * *************************************************************************/
static void send_bms_array(struct CANRCVBUF* po, float* pout, uint8_t n)
{
//	struct BQFUNCTION* p = &bqfunction;
	uint8_t i;

	for (i = 0; i < n; i++)
	{
		po->cd.uc[3] = i;

		// Reading into 4 byte payload
		loadfloat(&po->cd.uc[4], pout);

		// Queue CAN msg
		xQueueSendToBack(CanTxQHandle,po,4);
		pout += 1;
	}
	skip = 1;
	return;
}
/* *************************************************************************
 * static void send_allfets(struct CANRCVBUF* po);
 *	@brief	: Prepare and send CAN msgs for FET status
 *  @param  : po = pointer to CAN msg requesting response
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
static void send_allfets(struct CANRCVBUF* po)
{
	struct BQFUNCTION* p = &bqfunction;

	/* FET status: (see cellball.c) */
	po->cd.uc[3]  = 0;	
	po->cd.ui[1]  = p->cellbal;
	// Add Dump, Dump2, Heater, trickle FET status. */
	po->cd.uc[7]  = p->fet_status;
	xQueueSendToBack(CanTxQHandle,&p->canmsg,4);	

	/* Unexpected open cell voltage (see cellball.c) */
	po->cd.uc[3]  = 1;	
	po->cd.ui[1]  = p->cellvopenbits;
	xQueueSendToBack(CanTxQHandle,&p->canmsg,4);	

	/* Cells installed (see bq_idx_v_struct.c,h) */
	po->cd.uc[3]  = 2;	
	po->cd.ui[1]  = p->cellspresent;
	xQueueSendToBack(CanTxQHandle,&p->canmsg,4);	

	skip = 1;
	return;
}
/* *************************************************************************
 * static void status_group(struct CANRCVBUF* po);
 *	@brief	: Load data for status
 *  @param  : po = pointer to outgoing CAN msg 
 * *************************************************************************/
static void status_group(struct CANRCVBUF* po)
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
	po->cd.uc[3] = 0;
	// Status bytes (U8)
	po->cd.uc[4] = p->battery_status;
	po->cd.uc[5] = p->fet_status;
	skip = 0;
	return;
}
/* *************************************************************************
 * static void not_implemented(struct CANRCVBUF* po);
 *	@brief	: Send response to not implemented command code
 *  @param  : po = pointer to outgoing CAN msg
 * *************************************************************************/

static void not_implemented(struct CANRCVBUF* po)
{
	po->cd.uc[1] = MISCQ_UNIMPLIMENT;
	return;
}