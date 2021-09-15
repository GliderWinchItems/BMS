/******************************************************************************
* File Name          : gevcu_func_init.c
* Date First Issued  : 10/09/2019
* Description        : uart input
*******************************************************************************/

#include "gevcu_func_init.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "MailboxTask.h"
#include "CanTask.h"
#include "can_iface.h"
#include "CanTask.h"
#include "GevcuTask.h"
#include "main.h"
#include "stm32f4xx_hal_tim.h"
#include "morse.h"
#include "canfilter_setup.h"
#include "gevcu_idx_v_struct.h"
#include "../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* From 'main.c' */
extern struct CAN_CTLBLOCK* pctl0;	// Pointer to CAN1 control block
extern TIM_HandleTypeDef htim4;
extern CAN_HandleTypeDef hcan1;

/* *************************************************************************
 * void gevcu_func_init_init(struct GEVCUFUNCTION* p, struct ADCFUNCTION* padc);
 *	@brief	: Initialize working struct for ContactorTask
 * @param	: p    = pointer to ContactorTask
 * @param	: padc = pointer to ADC working struct
 * *************************************************************************/

void gevcu_func_init_init(struct GEVCUFUNCTION* p, struct ADCFUNCTION* padc)
{
	int i;

	/* Pointer to ADC working parameters. */
	p->padc = padc;

	p->cntctr_ka_to = (SWTIM1TICKPERSEC/3); // Contactor keepalive (three per sec)
	p->cntctr_ka_ctr = 0; // Contactor keepalive timer tick counter

	/* Convert gevcu_idx_v_struct times to timer ticks. */
	p->ka_k       = (p->lc.ka_t);   // Gevcu polling timer (configTICK_RATE_HZ = 512)
	p->keepalive_k= (p->lc.keepalive_t); // keep-alive timeout (timeout delay ms)
	p->hbct_k     = (p->lc.hbct_t); // Heartbeat ct: ticks between sending msgs hv1:cur1
	
// Skip CAN msg arrival notifications for lines that are commented out.
	/* Add CAN Mailboxes                               CAN     CAN ID             TaskHandle,Notify bit,Skip, Paytype */
	p->pmbx_cid_gps_sync          =  MailboxTask_add(pctl0,p->lc.cid_gps_sync,          NULL,GEVCUBIT06,0,U8);
	p->pmbx_cid_drum_tst_stepcmd   = MailboxTask_add(pctl0,p->lc.cid_drum_tst_stepcmd,  NULL,GEVCUBIT15,0,U8_FF);
//	p->pmbx_cid_cntctr_keepalive_r = MailboxTask_add(pctl0,p->lc.cid_cntctr_keepalive_r,NULL,GEVCUBIT07,0,U8_U8_U8);
//	p->pmbx_cid_dmoc_actualtorq    = MailboxTask_add(pctl0,p->lc.cid_dmoc_actualtorq,   NULL,GEVCUBIT08,0,I16);
//	p->pmbx_cid_dmoc_speed         = MailboxTask_add(pctl0,p->lc.cid_dmoc_speed,        NULL,GEVCUBIT09,0,I16_X6);
//	p->pmbx_cid_dmoc_dqvoltamp     = MailboxTask_add(pctl0,p->lc.cid_dmoc_dqvoltamp,    NULL,GEVCUBIT10,0,I16_I16_I16_I16);
//	p->pmbx_cid_dmoc_torque        = MailboxTask_add(pctl0,p->lc.cid_dmoc_torque,       NULL,GEVCUBIT11,0,I16_I16);
//	p->pmbx_cid_dmoc_critical_f    = MailboxTask_add(pctl0,p->lc.cid_dmoc_critical_f,   NULL,GEVCUBIT12,0,NONE);
//	p->pmbx_cid_dmoc_hv_status     = MailboxTask_add(pctl0,p->lc.cid_dmoc_hv_status,    NULL,GEVCUBIT13,0,I16_I16_X6);
//	p->pmbx_cid_dmoc_hv_temps      = MailboxTask_add(pctl0,p->lc.cid_dmoc_hv_temps,     NULL,GEVCUBIT14,0,U8_U8_U8);
//	p->pmbx_cid_gevcur_keepalive_i = MailboxTask_add(pctl0,p->lc.cid_gevcur_keepalive_i,NULL,GEVCUBIT15,0,23);

	/* Pre-load fixed data in CAN msgs */
	for (i = 0; i < NUMCANMSGS; i++)
	{
		p->canmsg[i].pctl = pctl0;   // Control block for CAN module (CAN 1)
		p->canmsg[i].maxretryct = 8; //
		p->canmsg[i].bits = 0;       //
		p->canmsg[i].can.dlc = 8;    // Default payload size (might be modified when loaded and sent)
	}

	/* Pre-load CAN ids & in some cases, dlc. */
   	// Contactor keepalive/command msg.
	p->canmsg[CID_GEVCUR_KEEPALIVE_R].can.id  = p->lc.cid_cntctr_keepalive_i;
	p->canmsg[CID_GEVCUR_KEEPALIVE_R].can.dlc = 1;  // Single byte status

	   // Control Law V1: Desired (commanded) speed (float), and integrator (float) */
	p->canmsg[CID_GEVCUR_CTL_LAWV1].can.id    = p->lc.cid_gevcur_ctllawv1;
	p->canmsg[CID_GEVCUR_CTL_LAWV1].can.dlc   = 8;	// Two 4 byte fields


 
	return;
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
/* *************************************************************************
 * void gevcu_func_init_canfilter(struct GEVCUFUNCTION* p);
 *	@brief	: Setup CAN hardware filter with CAN addresses to receive
 * @param	: p    = pointer to Gevcu function parameters
 * *************************************************************************/
void gevcu_func_init_canfilter(struct GEVCUFUNCTION* p)
{
	canfilt(661,p->pmbx_cid_gps_sync);
	canfilt(660,p->pmbx_cid_drum_tst_stepcmd);
//	canfilt(662,p->pmbx_cid_cntctr_keepalive_r);
//	canfilt(663,p->pmbx_cid_dmoc_actualtorq);
//	canfilt(664,p->pmbx_cid_dmoc_speed);
//	canfilt(665,p->pmbx_cid_dmoc_dqvoltamp);
//	canfilt(666,p->pmbx_cid_dmoc_torque);
//	canfilt(667,p->pmbx_cid_dmoc_critical_f);
//	canfilt(668,p->pmbx_cid_dmoc_hv_status);
//	canfilt(669,p->pmbx_cid_dmoc_hv_temps);
	return;
}
