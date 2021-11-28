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

extern uint16_t blk_0x0083_u16[3]; // BQ cell balancing

static void loadfloat(uint8_t* puc, float* pf);
static void cellv_cal(struct CANRCVBUF* pcan);
static void cellv_adc(struct CANRCVBUF* pcan);
static void status_group(void);

/* *************************************************************************
 * static void returncmd(struct CANTXQMSG* pmsg);
 *	@brief	: Copy request bytes
 *  @param  : pmsg = pointer to CAN msg
 * *************************************************************************/
static void returncmd(struct CANTXQMSG* pmsg, struct CANRCVBUF* pcan)
{
	pmsg->can.cd.uc[1] = (pcan->cd.uc[0] & 0xC0) | (bqfunction.ident_onlyus);
	pmsg->can.cd.uc[1] = pcan->cd.uc[1]; // Command code
	return;
}
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
		// Set sequence number sent by requesting CAN msgs
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[0] &= ~0x7;
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[0] |= (pcan->cd.uc[1] & 0x7);

		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[1] = *(pcell+0);
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[2] = *(pcell+1);
		p->canmsg[CID_MSG_CELLV01 + i].can.cd.ui[3] = *(pcell+2);

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
extern float adcsumfilt[2][ADC1IDX_ADCSCANSIZE]; 
void cancomm_items_sendcmdr(struct CANRCVBUF* pcan)
{
	struct BQFUNCTION* p = &bqfunction;

	/* Pointer to payload 4 byte value is used often. */
	uint8_t* puc = &p->canmsg[CID_CMD_MISC].can.cd.uc[3];
	
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
 		loadfloat(puc, &adcsumfilt[0][ADC1IDX_PA4_DC_DC]);
 			break;

 	case MISCQ_CHGR_V:      // 7 charger hv voltage
 		loadfloat(puc, &adcsumfilt[0][ADC1IDX_PA7_HV_DIV]);
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
 		loaduint32(&p->canmsg[CID_CMD_MISC].can.cd.uc[3],blk_0x0083_u16[0]);
		break;
	}
	/* Queue CAN msg response. */
	returncmd(&p->canmsg[CID_CMD_MISC], pcan); // Return command bytes
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
	uint8_t idx = pcan->cd.uc[2];
	if (idx > p->lc.ncell) 
		tmp = 0x80000000; // Bogus
	else
		tmp = p->cellv_latest[idx];

	p->canmsg[CID_CMD_MISC].can.cd.uc[2] = idx;

	// Reading into 4 byte payload
	loaduint32(&p->canmsg[CID_CMD_MISC].can.cd.uc[3], tmp);
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
	uint32_t  bogus = 0x80000000;

	/* Get index into array for requested reading. */
	uint8_t idx = pcan->cd.uc[2];
	if (idx > p->lc.ncell) 
		pcell = &bogus;
	else
		pcell = &cellcts.vi[idx].v;

	p->canmsg[CID_CMD_MISC].can.cd.uc[2] = idx;

	// Load 4 byte reading into 4 byte payload
	loaduint32(&p->canmsg[CID_CMD_MISC].can.cd.uc[3], *pcell);
	return;
}
/* *************************************************************************
 * static void status_group(void);
 *	@brief	: Prepare and send a response to a received command
 * *************************************************************************/
static void status_group(void)
{
	struct BQFUNCTION* p = &bqfunction;

	// Load status bytes
	p->canmsg[CID_CMD_MISC].can.cd.uc[3] = p->battery_status;
	p->canmsg[CID_CMD_MISC].can.cd.uc[4] = p->fet_status;
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