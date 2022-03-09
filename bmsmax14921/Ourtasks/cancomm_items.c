/******************************************************************************
* File Name          : cancomm_items.c
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#include "can_iface.h"
#include "CanTask.h"
#include "BQTask.h"
#include "cancomm_items.h"
#include "adcparams.h"
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"
#include "ADCTask.h"

void cancomm_items_sendcell(struct CANRCVBUF* pcan);

static void loadfloat(uint8_t* puc, float* pf);
static void cellv_cal(struct CANRCVBUF* pcan);
static void cellv_adc(struct CANRCVBUF* pcan);
static void status_group(void);

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
 * static uint32 extractint_32(uint8_t* p);
 *	@brief	: Extract 32b int from payload
 *  @param  : p = pointer to CAN msg payload
 *  @param  : n = unit32_t to be loaded
 * *************************************************************************/
static uint32_t extract_int32(uint8_t* p)
{
	return (
		(*(p+0) <<  0) | 
		(*(p+1) <<  8) |
		(*(p+2) << 16) |
		(*(p+3) << 24) );	
}
/* *************************************************************************
 * void cancomm_items_uni_bms(struct CANRCVBUF* pcan);
 *	@brief	: UNIversal multi-purpose command (CANCOMMBIT02)
 *  @param  : pcan = pointer to struct CANRCVBUF with request CAN msg
 * *************************************************************************/
void cancomm_items_uni_bms(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;
	uint32_t canid; // CANID requested to respond, if applicable
	uint8_t code;

	/* Skip if this request is not for us. */
	code = pcan->cd.uc[2] & 0xC0;
	// Code for which modules should respond bits [7:6]
	// 11 = All modules respond
    // 10 = All modules on identified string respond
    // 01 = Only identified string and module responds
    // 00 = spare; no response expected
    canid = extract_int32(&pcan->cd.uc[4]); 
    // Respond if CAN ID for this node was sent
//	if (!(((code == (3 << 6))) ||
//		  ((code == (2 << 6)) && ((pcan->cd.uc[2] & (3 << 4)) == p->ident_string)) ||
//		  ((code == (1 << 6)) && ((pcan->cd.uc[2] & 0x0F) == p->ident_onlyus)) ||
//		  ((canid == p->lc.cid_msg_bms_cellvsmr))))
//		return; // Skip. This request is not for us.

	if (canid != p->lc.cid_msg_bms_cellvsmr) return;

	/* Set up response to command. */
	switch(pcan->cd.uc[0])
	{
	case CMD_CMD_TYPE1: // Send Cell readings
		cancomm_items_sendcell(pcan);
		break;

	case CMD_CMD_TYPE2: // Respond to command
		cancomm_items_sendcmdr(pcan);
		break;

	case CMD_CMD_TYPE3:
		break;
	}
	return;
}
/* *************************************************************************
 * void cancomm_items_sendcell(struct CANRCVBUF* pcan);
 *	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/
void cancomm_items_sendcell(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;
	uint16_t* pcell = &p->cellv_latest[0];
	uint8_t i;
	uint8_t n;

	/* Load cell data into CAN msgs. */
	for (i = 0; i < MAXNUMCELLMSGS; i++)
	{	
		// Set sequence number sent by requesting CAN msgs
//		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[1] &= ~0x0f;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.uc[1] = (pcan->cd.uc[1] & 0x0f) | ((i*3) << 4);

		p->canmsg[CID_MSG_CELLV01 + i].can.cd.us[1] = *(pcell+0);
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.us[2] = *(pcell+1);
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.us[3] = *(pcell+2);
		pcell += 3;

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
		p->canmsg[CID_MSG_CELLV05].can.dlc = 6;
		n = 5;
		break;
	case 12: // 12 cells
		p->canmsg[CID_MSG_CELLV04].can.dlc = 8;
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

	/* Pointer to payload 4 byte value is used often. */
	uint8_t* puc = &p->canmsg[CID_CMD_MISC].can.cd.uc[4];

	/* Data in payload is always X4 (4 bytes, any format) */
	p->canmsg[CID_CMD_MISC].can.dlc = 8;
	// Return what was requested: copy four bytes
	p->canmsg[CID_CMD_MISC].can.cd.ui[0] = pcan->cd.ui[0];
/*	 Byte-at-a-time is slow and fattening.
	p->canmsg[CID_CMD_MISC].can.cd.uc[0] = CMD_CMD_TYPE2;
	// Code for response
	p->canmsg[CID_CMD_MISC].can.cd.uc[1] = pcan->cd.uc[1];
	// Module identification
	p->canmsg[CID_CMD_MISC].can.cd.uc[2] = pcan->cd.uc[2];
	// Item number, or thermistor number, or ...
	p->canmsg[CID_CMD_MISC].can.cd.uc[3] = pcan->cd.uc[3];
*/
	/* Command code. */
	switch(p->canmsg[CID_CMD_MISC].can.cd.uc[1])
	{
	case MISCQ_STATUS:      // 1 status
		status_group();
		break;

 	case MISCQ_CELLV_CAL:   // 2 cell voltage: calibrated
 		cellv_cal(pcan);
 			break;

 	case MISCQ_CELLV_ADC:   // 3 cell voltage: adc counts
 		cellv_adc(pcan);
 			break;

 	case MISCQ_TEMP_CAL:    // 4 temperature sensor: calibrated
 			break;

 	case MISCQ_TEMP_ADC:    // 5 temperature sensor: adc counts
 			break;

 	case MISCQ_DCDC_V:      // 6 isolated dc-dc converter output voltage
 		loadfloat(puc, &adc1.abs[ADC1IDX_PA4_DC_DC].filt);
 			break;

 	case MISCQ_CHGR_V:      // 7 charger hv voltage
 		loadfloat(puc, &adc1.abs[ADC1IDX_PA7_HV_DIV].filt);
 			break;

 	case MISCQ_HALL_CAL:    // 8 Hall sensor: calibrated
 			break;

 	case MISCQ_HALL_ADC:    // 9 Hall sensor: adc counts
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
// 		loaduint32(&p->canmsg[CID_CMD_MISC].can.cd.uc[3],blk_0x0083_u16[0]);
		break;
	}
	/* Queue CAN msg response. */
//	returncmd(&p->canmsg[CID_CMD_MISC], pcan); // Return command bytes
	xQueueSendToBack(CanTxQHandle,&p->canmsg[CID_CMD_MISC],4);
	return;
}
/* *************************************************************************
 * static void cellv_cal(struct CANRCVBUF* pcan);
 *	@brief	: Prepare and send a response to a received command
 *  @param  : pcan = pointer to CAN msg requesting response
 * *************************************************************************/
static void cellv_cal(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;
	int32_t  tmp;

	/* Get index into array for requested reading. */
	uint8_t idx = pcan->cd.uc[3];
	if (idx >= (p->lc.ncell - 1))
		tmp = 99999999;//x80000000; // Bogus
	else
		tmp = p->cellv_latest[idx];

	p->canmsg[CID_CMD_MISC].can.cd.uc[3] = idx;

	// Reading into 4 byte payload
	loaduint32(&p->canmsg[CID_CMD_MISC].can.cd.uc[4], tmp);
	return;
}
/* *************************************************************************
 * static void cellv_adc(struct CANRCVBUF* pcan);
 *	@brief	: Prepare and send a response to a received command
 *  @param  : pcan = pointer to CAN msg requesting response
 * *************************************************************************/
static void cellv_adc(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;
	uint32_t* pcell;
	uint32_t  bogus = 99999999;//0x80000000; // Bogus request reading

	/* Get index into array for requested reading. */
	uint8_t idx = pcan->cd.uc[3];
	if (idx > (p->lc.ncell - 1)) 
		pcell = &bogus; // Requested cell number out-of-range
	else
		pcell = &cellcts.vi[idx].v; 

	p->canmsg[CID_CMD_MISC].can.cd.uc[3] = idx;

	// Load 4 byte reading into 4 byte payload
	loaduint32(&p->canmsg[CID_CMD_MISC].can.cd.uc[4], *pcell);
	return;
}
/* *************************************************************************
 * static void status_group(void);
 *	@brief	: Load data for status
 * *************************************************************************/
static void status_group(void)
{
	/*
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 5v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Charging in progress
#define BSTATUS_DUMPTOV   (1 << 6)  // Dump to a voltage in progress

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
	// Bits for discharge FETs (U16)
	p->canmsg[CID_CMD_MISC].can.cd.us[3] = p->cellbal;
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