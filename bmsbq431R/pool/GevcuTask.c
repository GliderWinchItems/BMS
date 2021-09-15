/******************************************************************************
* File Name          : GevcuTask.c
* Date First Issued  : 10/08/2019
* Description        : Gevcu function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "ADCTask.h"
#include "adctask.h"
#include "morse.h"
#include "SerialTaskReceive.h"
#include "GevcuTask.h"
#include "gevcu_idx_v_struct.h"
#include "GevcuEvents.h"
#include "GevcuStates.h"
#include "GevcuUpdates.h"
#include "gevcu_func_init.h"
//#include "calib_control_lever.h"
//#include "contactor_control.h"
//#include "dmoc_control.h"

#include "main.h"
#include "morse.h"
#include "getserialbuf.h"
#include "yprintf.h"
//#include "shiftregbits.h"
//#include "spiserialparallelSW.h"
#include "stepper_items.h"
#include "drum_items.h"


/* From 'main.c' */
extern UART_HandleTypeDef huart3;

#define SWTIM1 500

osThreadId GevcuTaskHandle;

struct GEVCUFUNCTION gevcufunction;

/* *************************************************************************
 * void swtim1_callback(TimerHandle_t tm);
 * @brief	: Software timer 1 timeout callback
 * *************************************************************************/
static void swtim1_callback(TimerHandle_t tm)
{
	xTaskNotify(GevcuTaskHandle, GEVCUBIT04, eSetBits);
	return;
}
/* *************************************************************************
 * osThreadId xGevcuTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: GevcuTaskHandle
 * *************************************************************************/
osThreadId xGevcuTaskCreate(uint32_t taskpriority)
{
 	osThreadDef(GevcuTask, StartGevcuTask, osPriorityNormal, 0, (312-14));
	GevcuTaskHandle = osThreadCreate(osThread(GevcuTask), NULL);
	vTaskPrioritySet( GevcuTaskHandle, taskpriority );
	return GevcuTaskHandle;
}
/* *************************************************************************
 * void StartGevcuTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
struct SWITCHPTR* psw_safeactivex; // Debugging

void StartGevcuTask(void const * argument)
{
	/* A notification copies the internal notification word to this. */
	uint32_t noteval = 0;    // Receives notification word upon an API notify
	uint32_t noteuse = 0xffffffff;

	/* Setup serial receive for uart */
	/* Get buffer control block for incoming uart lines. */
	// 2 line buffers of 48 chars, no dma buff, char-by-char line mode
//	gevcufunction.prbcb3  = xSerialTaskRxAdduart(&huart3,0,GEVCUBIT01,&noteval,2,48,0,0);
//	if (gevcufunction.prbcb3 == NULL) morse_trap(47);

	/* Init struct with working params */
	gevcu_idx_v_struct_hardcode_params(&gevcufunction.lc);

	/* Initialize working struc for GevcuTask. */
	extern struct ADCFUNCTION adc1;
	gevcu_func_init_init(&gevcufunction, &adc1);

	/* CAN hardware filter: restrict incoming to necessary CAN msgs. */
	// If gateway is included, then all CAN msgs are needed.
#ifndef GATEWAYTASKINCLUDED
	gevcu_func_init_canfilter(&gevcufunction);
#endif

	/* Create timer Auto-reload/periodic (128 per sec) */
	gevcufunction.swtimer1 = xTimerCreate("swtim1",gevcufunction.ka_k,pdTRUE,\
		(void *) 0, swtim1_callback);
	if (gevcufunction.swtimer1 == NULL) {morse_trap(404);}

	/* Initial startup state */
	gevcufunction.state = GEVCU_INIT;

	/* Stepper initialization starts timers. */
	stepper_items_init();

	/* Drum intialization. */
	drum_items_init(&drumstuff);

	/* Start command/keep-alive timer */
	BaseType_t bret = xTimerReset(gevcufunction.swtimer1, 10);
	if (bret != pdPASS) {morse_trap(405);}

extern CAN_HandleTypeDef hcan1;
	HAL_CAN_Start(&hcan1); // CAN1

  /* Infinite loop */
  for(;;)
  {
		/* Wait for notifications */
		xTaskNotifyWait(0,0xffffffff, &noteval, portMAX_DELAY);
		noteuse = 0;	// Accumulate bits in 'noteval' processed.
  /* ========= Events =============================== */
// NOTE: this could be made into a loop that shifts 'noteval' bits
// and calls from table of addresses.  This would have an advantage
// if the high rate bits are shifted out first since a test for
// no bits left could end the testing early.
		// Check notification and deal with it if set.
		if ((noteval & GEVCUBIT00) != 0)
		{ // ADC readings ready
//			GevcuEvents_00(); // Skip and let ADCTask to the CL work
			noteuse |= GEVCUBIT00;
		}
		if ((noteval & GEVCUBIT01) != 0)
		{ // 
			GevcuEvents_01();
			noteuse |= GEVCUBIT01;
		}
		if ((noteval & GEVCUBIT02) != 0)
		{ //  
//			GevcuEvents_02();
			noteuse |= GEVCUBIT02;
		}
		if ((noteval & GEVCUBIT03) != 0)
		{ // 
			GevcuEvents_03();			
			noteuse |= GEVCUBIT03;
		}
		if ((noteval & GEVCUBIT04) != 0)
		{ // TIMER1:  (periodic)
			GevcuEvents_04();
			noteuse |= GEVCUBIT04;
		}
		if ((noteval & GEVCUBIT05) != 0)
		{ // spare (notification not expected)
//			GevcuEvents_05();
			noteuse |= GEVCUBIT05;
		}
		if ((noteval & GEVCUBIT06) != 0) 
		{ // CAN:  
			GevcuEvents_06();
			noteuse |= GEVCUBIT06;
		}
		if ((noteval & GEVCUBIT07) != 0) 
		{ // CAN: cid_cntctr_keepalive_r (contactor cmd response)
			GevcuEvents_07();
			noteuse |= GEVCUBIT07;
		}
		if ((noteval & GEVCUBIT08) != 0) 
		{ // CAN:  cid_dmoc_actualtorq
			GevcuEvents_08();
			noteuse |= GEVCUBIT08;
		}
		if ((noteval & GEVCUBIT09) != 0) 
		{ // CAN:  cid_dmoc_speed
			GevcuEvents_09();
			noteuse |= GEVCUBIT09;
		}
		if ((noteval & GEVCUBIT10) != 0) 
		{ // CAN:  cid_dmoc_dqvoltamp (see * below)
			GevcuEvents_10();
			noteuse |= GEVCUBIT10;
		}
		if ((noteval & GEVCUBIT11) != 0) 
		{ // CAN: cid_dmoc_torque (see * below)
			GevcuEvents_11();
			noteuse |= GEVCUBIT11;
		}
		if ((noteval & GEVCUBIT12) != 0) 
		{ // CAN:  cid_dmoc_critical_f (see * below)
			GevcuEvents_12();
			noteuse |= GEVCUBIT12;
		}
		if ((noteval & GEVCUBIT13) != 0) 
		{ // CAN:  cid_dmoc_hv_status
			GevcuEvents_13();
			noteuse |= GEVCUBIT13;
		}
		if ((noteval & GEVCUBIT14) != 0) 
		{ // CAN:  cid_dmoc_hv_temps
			GevcuEvents_14();
			noteuse |= GEVCUBIT14;
		}
		if ((noteval & GEVCUBIT15) != 0) 
		{ // CAN:  CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000
			GevcuEvents_15();
			noteuse |= GEVCUBIT15;
		}
// * = gevcu_func_init.c: this CAN msg not initialized for a Mailbox */

  /* ========= States =============================== */

		switch (gevcufunction.state)
		{
		case GEVCU_INIT:
			GevcuStates_GEVCU_INIT();
			break;

		case GEVCU_SAFE_TRANSITION:
			GevcuStates_GEVCU_SAFE_TRANSITION();
			break;
		case GEVCU_SAFE:
			GevcuStates_GEVCU_SAFE();
			break;

		case GEVCU_ACTIVE_TRANSITION:
			GevcuStates_GEVCU_ACTIVE_TRANSITION();
			break;
		case GEVCU_ACTIVE:
			GevcuStates_GEVCU_ACTIVE();
			break;

		case GEVCU_ARM_TRANSITION:
			GevcuStates_GEVCU_ARM_TRANSITION();
			break;
		case GEVCU_ARM:
			GevcuStates_GEVCU_ARM();
			break;

		default:
			break;
		}
  /* ========= Update outputs ======================= */
		GevcuUpdates();
  }
// Never should come here
}

