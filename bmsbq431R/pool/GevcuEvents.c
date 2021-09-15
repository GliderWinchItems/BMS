/******************************************************************************
* File Name          : GevcuEvents.c
* Date First Issued  : 07/01/2019
* Description        : Events in Gevcu function w STM32CubeMX w FreeRTOS
*******************************************************************************/
/*
The CL calibration and ADC->pct position is done via ADC new readings notifications.


*/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "ADCTask.h"
#include "adctask.h"
#include "gevcu_idx_v_struct.h"
#include "GevcuEvents.h"
#include "morse.h"
#include "SerialTaskReceive.h"
#include "GevcuTask.h"
#include "can_iface.h"
#include "gevcu_cmd_msg.h"
#include "gevcu_msgs.h"
#include "MailboxTask.h"
#include "main.h"
#include "stepper_items.h"

/* *************************************************************************
 * void GevcuEvents_00(void);
 * @brief	: ADC readings available
 * *************************************************************************/
void GevcuEvents_00(void)
{
//	gevcufunction.evstat |= EVNEWADC; // Show new readings ready
	
	return;
}
/* *************************************************************************
 * void GevcuEvents_01(void);
 * @brief	: Switch pair: SAFE/ACTIVE
 * *************************************************************************/
/*
   When the SAFE/ACTIVE sw is goes from ACTIVE to SAFE, all states except
   the initial startup terminate and the transition to SAFE state begins.
*/
void GevcuEvents_01(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_02(void);
 * @brief	: Z_ODOMTR
 * @param	: psw = pointer to switch struct
 * *************************************************************************/

void GevcuEvents_02(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_03(void);
 * @brief	: Torque reversal pushbutton
 * @param	: psw = pointer to switch struct
 * *************************************************************************/

void GevcuEvents_03(void)
{ 
	return;
}
/* *************************************************************************
 * void GevcuEvents_04(void);
 * @brief	: TIMER1: Software timer 1
 * *************************************************************************/
uint32_t dbgev04;


void GevcuEvents_04(void)
{
	gevcufunction.swtim1ctr += 1;
	gevcufunction.evstat |= EVSWTIM1TICK; // Timer tick

	/* Stepper CL keep-alive timeout. */
	stepper_items_timeout();

	/* Stepper heartbeat. */
	stepper_items_CANsend();

	return;
}
/* *************************************************************************
 * void GevcuEvents_05(void);
 * @brief	: TIMER2: Software timer 2
 * *************************************************************************/
void GevcuEvents_05(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_06(void);
 * @brief	: CAN: cid_gps_sync
 * *************************************************************************/
void GevcuEvents_06(void)
{
//	gevcu_cmd_msg_i(pcf); // Build and send CAN msg with data requested
	return;
}
/* *************************************************************************
 * void GevcuEvents_07(void);
 * @brief	: CAN: cid_cntctr_keepalive_r
 * *************************************************************************/
struct MAILBOXCAN* pdbg07mbx;

void GevcuEvents_07(void)
{
return;

// Debugging: Copy mailbox for defaultTask display
pdbg07mbx = gevcufunction.pmbx_cid_cntctr_keepalive_r; 

	gevcufunction.evstat |= EVCANCNTCTR; // Show New Contactor CAN msg 
			
	return;
}	
/* *************************************************************************
 * void GevcuEvents_08(void);
 * @brief	: CAN: cid_dmoc_actualtorq
 * *************************************************************************/
void GevcuEvents_08(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_09(void);
 * @brief	: CAN: cid_dmoc_speed,     NULL,GEVCUBIT09,0,I16_X6);
 * *************************************************************************/
void GevcuEvents_09(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_10(void);
 * @brief	: CAN: cid_dmoc_dqvoltamp, NULL,GEVCUBIT10,0,I16_I16_I16_I16);
 * *************************************************************************/
void GevcuEvents_10(void)
{

	return;
}
/* *************************************************************************
 * void GevcuEvents_11(void);
 * @brief	: CAN: cid_dmoc_torque, NULL,GEVCUBIT11,0,I16_I16); 
 * *************************************************************************/
void GevcuEvents_11(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_12(void);
 * @brief	: CAN: cid_dmoc_critical_f,NULL,GEVCUBIT12,0,NONE);
 * *************************************************************************/
void GevcuEvents_12(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_13(void);
 * @brief	: CAN: cid_dmoc_hv_status, NULL,GEVCUBIT13,0,I16_I16_X6);
 * *************************************************************************/
void GevcuEvents_13(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_14(void);
 * @brief	: CAN: cid_dmoc_hv_temps,  NULL,GEVCUBIT14,0,U8_U8_U8);
 * *************************************************************************/
void GevcuEvents_14(void)
{
	return;
}
/* *************************************************************************
 * void GevcuEvents_15(void);
 * @brief	: CAN: cid_gevcur_drum_CL,U8_FF);
 * *************************************************************************/
void GevcuEvents_15(void)
{
	/* Received CAN msg with Control Lever position, direction and enable bits */
	stepper_items_clupdate(&gevcufunction.pmbx_cid_drum_tst_stepcmd->ncan.can);
	
	return;
}

