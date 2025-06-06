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
#include "bms_items.h"

extern uint32_t dbgwarning; // Bits for various warnings

extern uint32_t rtcregs_status; // 'main' saves rtc registers upon startup
extern uint32_t rtcregs_morse_code;
extern uint8_t rtcregs_OK; // 1 = rtc regs were OK; 0 = not useable.

extern uint8_t  cmd_pwm_set;  // pwm setting being forced
extern uint32_t cmd_pwm_ctr;  // Running ctr: pwm forced flag

extern uint8_t fanspeed;
extern float fanrpm;

void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf);

static void loadfloat(uint8_t* puc, float* f);
static void status_group(struct CANRCVBUF* po);
static void send_bms_array(struct CANRCVBUF* po, float* pout, uint8_t n);
static void send_allfets(struct CANRCVBUF* po);
static void not_implemented(struct CANRCVBUF* po);
static void send_bms_one(struct CANRCVBUF* po, float* pout, uint8_t k);
static void send_bms_crcorchk(struct CANRCVBUF* po, uint8_t k);


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
 * static int req_set(uint8_t idx, struct CANRCVBUF* pi);
 *	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : idx: (0,1,2):(REQ_HEATER, REQ_DUMP, REQ_DUMP2)
 *  @param  : pi = pointer to input CAN msg
 *  @return : 0 = OK; -1 = rejected
 * *************************************************************************/
static int req_set(uint8_t idx, struct CANRCVBUF* pi)
{	// Check if command is OFF = 0| ON = 1
	struct BQFUNCTION* p = &bqfunction;
	if (pi->cd.uc[3] > 1)
		return -1; // Bogus argument
	p->bqreq[idx].on  = pi->cd.uc[3]; // Set request ON|OFF
	p->bqreq[idx].req = 1; // Show request is active
	// Set timeout of request
	p->bqreq[idx].tim = xTaskGetTickCount() + pdMS_TO_TICKS(CANSETFET_TIM);
	return 0;
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
	//uint8_t uctmp;

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
	p->HBcellv_ctr = xTaskGetTickCount() + p->hbct_k; // Next HB time
	return;
}

void cancomm_items_sendcmdr(struct CANRCVBUF* pi)
{
	struct BQFUNCTION* p = &bqfunction;
	struct CANRCVBUF* po = &p->canmsg.can;
	float ftmp[ADCDIRECTMAX];
	float fdeg;
	float fcur;
	uint8_t i;

	/* Pointer to payload 4 byte value is used often. */
	uint8_t* puc = &po->cd.uc[4];

	/* Response carries "our" CAN ID. */
	po->id = p->lc.cid_msg_bms_cellvsmr;

	skip = 0;

   if (pi->cd.uc[0] == CMD_CMD_CELLPOLL)// (code 42) Request to send cell
   {
   	// Set code in response that identifies who polled
   	 if (pi->id == p->lc.cid_uni_bms_emc1_i)
   	 	po->cd.uc[0] = CMD_CMD_CELLEMC1; // EMC polled cell voltages
   	 else if (pi->id == p->lc.cid_uni_bms_emc2_i)
   	 	po->cd.uc[0] = CMD_CMD_CELLEMC2; // EMC polled cell voltages
   	 else if (pi->id == p->lc.cid_uni_bms_pc_i)
   	 	po->cd.uc[0] = CMD_CMD_CELLPC; // PC polled cell voltages
   	 else if (pi->id == CANID_UNIT_99)
   	 	{ // Here Dummy CAN ID means heartbeat timeout trigger this
			po->cd.uc[0] = CMD_CMD_CELLHB; // 45 Heartbeat timeout cell voltages
//			bqfunction.hbseq += 1; // Group sequence number
   	 	}  
   	 	else
   	 	{
   	 		// Warning: Unexpectd CAN ID
   	 		dbgwarning |= DBG1;
   	 	}
   	 // Send cell voltages: 6 CAN msgs
   	 bqfunction.HBcellv_ctr = xTaskGetTickCount() + bqfunction.hbct_k;
 	 cancomm_items_sendcell(pi, &p->cellv[0]);

 	 return;
    }
   if (pi->cd.uc[0] == CMD_CMD_TYPE2) // (code 43) Request to send cell
   {    
		// Set code in response that identifies who polled
		if (pi->id == p->lc.cid_uni_bms_emc1_i)
			po->cd.uc[0] = CMD_CMD_MISCEMC1; // EMC polled cell voltages
		else if (pi->id == p->lc.cid_uni_bms_emc2_i)
			po->cd.uc[0] = CMD_CMD_MISCEMC2; // EMC polled cell voltages
		else if (pi->id == p->lc.cid_uni_bms_pc_i)
		 	po->cd.uc[0] = CMD_CMD_MISCPC; // PC polled cell voltages
		else if (pi->id == CANID_UNIT_99)
		{ // Here Dummy CAN ID means heartbeat timeout trigger this
		 	po->cd.uc[0] = CMD_CMD_MISCHB; // 45 Heartbeat timeout cell voltages
		//			bqfunction.hbseq += 1; // Group sequence number
		}  
		else
		{
		 	// Warning?: Unexpectd CAN ID2
		 	dbgwarning |= DBG2;
		}

		/* Command code. 
		If a BMSTask request was needed this would have been taken care of
		in CanCommTask before this routine is called. To queue a request
		here could result in an endless loop. 
		*/
		po->cd.uc[1] = pi->cd.uc[2]; // MISCQ code
		switch(pi->cd.uc[2])
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
	 		po->cd.uc[2] = 0xA5; // Not used
	 		send_bms_one(po, &bqfunction.lc.thermcal[0].temp,0);
	 		send_bms_one(po, &bqfunction.lc.thermcal[1].temp,1);
	 		send_bms_one(po, &bqfunction.lc.thermcal[2].temp,2);
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
			fcur = bms_items_current_sense_Hall(); // Calibrate current sense readings
			send_bms_one(po, &fcur, 0);
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
	 		po->cd.us[1] = 0; // Clear payload bytes [2],[3]
	 		loaduint32(puc,bqfunction.cellbal);

		case MISCQ_SET_DUMP: // 13 Set DUMP fet ON|OFF
			req_set(REQ_DUMP,pi);
			break;

		case MISCQ_SET_DUMP2: // 14 Set DUMP2 fet ON|OFF
			req_set(REQ_DUMP2,pi);
			break;

		case MISCQ_SET_HEATER: // 15 Set HEATER fet ON|OFF
			req_set(REQ_HEATER,pi);
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
			fcur = bms_items_current_sense(); // Calibrate current sense readings
			send_bms_one(po, &fcur, 0);
	 		break;

		case MISCQ_CURRENT_ADC: // 25 Below cell #1 minus, current resistor: adc counts		
			not_implemented(po);
	 		break;

	 	case MISCQ_SET_DCHGTST: // 28 Set discharge test with heater fet load
		 	if (pi->cd.uc[3] == 0)
		 	{ // Here turn discharge test off
		 		p->hyster_sw = 1;  // Stop self-discharge if sw is on.
		 		p->discharge_test_sw = 0;//
		 	}
		 	else
		 	{ // Here, maybe turn discharge test on, otherwise bogus uc[3]
		 		if (pi->cd.uc[3] == 1)
		 		{ // Here, definitely turn test on
	 		 		p->hyster_sw = 0;  // Stop self-discharge if sw is on.
	 		 		p->discharge_test_sw = 1; // Enable heater when hyster_sw comes on	
		 		}
		 	}
			break;

		case MISCQ_SET_SELFDCHG: // 31 Self-discharge mode on|off
		 	if (pi->cd.uc[3] == 1)
		 	{ // Here set self-discharge on
		 		p->hyster_sw_trip = 1; // trip hyster_sw to self-discharge mode
		 	}
		 	else
		 	{ // Here, set self-discharge off (charge for balancing)
			 	if (pi->cd.uc[3] == 0) // Skip bogus value if [3] not 1 or 0
	 		 		p->hyster_sw_trip = 0; // Trip to charge/balance mode
		 	}
			break;

		case MISCQ_SET_DCHGFETS: // 30 Set discharge FETs all on, all off, of single
			if (pi->cd.uc[3] == 111)
			{ // 111 is code for ALL ON
				p->cansetfet = 0x3ffff;
				p->cansetfet_tim = xTaskGetTickCount() + pdMS_TO_TICKS(CANSETFET_TIM);
			}
			else if ((pi->cd.uc[3] > 0) || (pi->cd.uc[3] < 19))
			{ // 1 - 18 sets one FET
				p->cansetfet = (1 << (pi->cd.uc[3] - 1));
				p->cansetfet_tim = xTaskGetTickCount() + pdMS_TO_TICKS(CANSETFET_TIM);
			}
			else if (pi->cd.uc[3] == 0)
			{ // Code for all OFF. Timeout does not matter.
				p->cansetfet = 0;
			}
			// [3] not mtching in the above is ignored.
			break;

		case MISCQ_PRM_MAXCHG: // 32 Get Parameter: Max charging current
			po->cd.us[2] = bqfunction.lc.maxchrgcurrent; // Maximum charge current (0.1a)
			po->cd.us[2] = 0; // Reserved
			skip = 0;
			break;	

		case MISCQ_READ_AUX:// BMS responds with A,B,C,D AUX register readings (12 msgs)
		 	po->cd.uc[2] = 0xAA; // Not used
			for (i = 0; i < 12; i += 2) 		 
			{		
				po->cd.uc[3] = i;				
				po->cd.us[2] = bmsspiall.auxreg[i];
				po->cd.us[3] = bmsspiall.auxreg[i+1];
	 			xQueueSendToBack(CanTxQHandle, po, 4);
	 		}
	 		skip = 1;
			break;

		case MISCQ_PROC_TEMP: // 36 Processor calibrated internal temperature (deg C)
			fdeg = adcparams_caltemp();
			send_bms_one(po, &fdeg,0);
			break;

		case MISCQ_CHG_LIMITS: // 37 Show params: Module V max, Ext chg current max, Ext. chg bal
			po->cd.us[1] = 0; // uc[2]-[3] cleared
			po->cd.uc[4] = p->lc.maxchrgcurrent;  // Maximum charge current (0.1a) (255 is >= 25.5a)
			po->cd.uc[5] = p->lc.chrgcurrent_bal; // Charge current for module balancing (0.1a)
			po->cd.us[3] = p->lc.maxmodule_v;     // Module voltage max (0.1v)
			break;	

		case MISCQ_MORSE_TRAP: // 38 Show params: Module V max, Ext chg current max, Ext. chg bal
			po->cd.us[1] = 0; // uc[2]-[3] cleared
			po->cd.uc[3] = rtcregs_OK;
			po->cd.ui[1] = rtcregs_morse_code;

		case MISCQ_FAN_STATUS: // 39  Retrieve fan: pct and rpm 
			po->cd.us[1] = 0; // uc[2]-[3] cleared
			send_bms_one(po, &fanrpm, bqfunction.fanspeed);
			break;

		case MISCQ_FAN_SET_SPD: // 40 Set fan: pct (0 - 100)
			cmd_pwm_ctr += 1; // Flag fanop.c for a new request
			cmd_pwm_set  = pi->cd.uc[3]; // Update request value
			skip = 1; // This is a set, so no response
			break;			
		case MISCQ_PROG_CRC: // 41 Retrieve installed program's: CRC
			send_bms_crcorchk(po, 0);
			break;
		case MISCQ_PROG_CHKSUM: // 42 Retrieve installed program's: Checksum	
			send_bms_crcorchk(po, 1);
			break;
		case MISCQ_PROG_CRCCHK: // 43 Retrieve for both 41 and 42 (two msgs)
			po->cd.uc[1] = MISCQ_PROG_CRC;
			send_bms_crcorchk(po, 0);
			po->cd.uc[1] = MISCQ_PROG_CHKSUM;
			send_bms_crcorchk(po, 1);
			break;			
		}		
	}
	if (skip == 0)
	{ // Here, single CAN msg has not been queued for sending
		/* Queue CAN msg response. */
		xQueueSendToBack(CanTxQHandle, po, 4);
	}
	return;
}
/* *************************************************************************
 * static void send_bms_one(struct CANRCVBUF* po, float* pout, uint8_t k);
 *	@brief	: Prepare and send one CAN msgs w float
 *  @param  : po = pointer response CAN msg
 *  @param  : pout = pointer float
 *  @param  : k = identification number (e.g. thermistor #1)
 * *************************************************************************/
static void send_bms_one(struct CANRCVBUF* po, float* pout, uint8_t k)
{
	po->cd.uc[3] = k;
	loadfloat(&po->cd.uc[4], pout);// Reading into 4 byte payload
	xQueueSendToBack(CanTxQHandle,po,4);// Queue (copy) CAN msg
	skip = 1;
	return;
}
/* *************************************************************************
 * static void send_bms_crcorchk(struct CANRCVBUF* po, uint8_t k);
 *	@brief	: Prepare and send CAN msg with CRC or CHKSUM
 *  @param  : po = pointer response CAN msg
 *  @param  : pout = pointer float
 *  @param  : k = word offset at end of program: 0 for CRC; 1 for CHKSUM
 * *************************************************************************/
//uint32_t* k0;
//uint32_t* k1;

extern void* __appjump; // Defined in ldr.ld file
static void send_bms_crcorchk(struct CANRCVBUF* po, uint8_t k)
{
	uint32_t* pend_add;
	uint32_t* pend_endx;
	uint32_t* pentry =__appjump; // App jump address
	pend_add = (uint32_t*)((uint32_t)pentry & ~1UL); // Clear jump flag        
	pend_add -= 1; // Back up to word ahead of jump entry
	pend_endx = (uint32_t*)(*pend_add); // Get pointer to end of program
	po->cd.ui[1] = *(pend_endx + k); // Select word following end-of-program



	po->cd.us[1] = 0; // uc[2]-[3] cleared not used payload bytes
	xQueueSendToBack(CanTxQHandle,po,4);// Queue (copy) CAN msg
	skip = 1; // No need for default send
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

Mode status bits 'mode_status' --
#define MODE_SELFDCHG  (1 << 0) // 1 = Self discharge; 0 = charging
#define MODE_CELLTRIP  (1 << 1) // 1 = One or more cells tripped max


Temperature sensors--
#define TEMPTUR_OVMAX  (1 << 0) // 1 = Temperature sensor 1 above max threshold
#define TEMPTUR_OVMAX  (1 << 1) // 1 = Temperature sensor 2 above max threshold
#define TEMPTUR_OVMAX  (1 << 2) // 1 = Temperature sensor 3 above max threshold

*/
	struct BQFUNCTION* p = &bqfunction;
	po->cd.uc[1] = MISCQ_STATUS; // 
	po->cd.us[1] = 0; // uc[2]-[3] cleared
	// Data payload bytes [4]-[7]
	po->cd.ui[1] = 0; // Clear
	po->cd.uc[4] = p->battery_status;
	po->cd.uc[5] = p->fet_status;
	po->cd.uc[6] = p->mode_status;
	po->cd.uc[7] = p->temp_status;
	skip = 0;

	p->HBstatus_ctr = xTaskGetTickCount() + p->hbct_k; // Next HB time	
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