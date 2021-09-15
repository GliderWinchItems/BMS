/******************************************************************************
* File Name          : GevcuTask.h
* Date First Issued  : 06/25/2019
* Description        : Gevcu function w STM32CubeMX w FreeRTOS
*******************************************************************************/

#ifndef __GEVCUTASK
#define __GEVCUTASK

#include <stdint.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "adcparams.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "adc_idx_v_struct.h"
#include "CanTask.h"


/* 
=========================================      
CAN msgs: 
ID suffix: "_i" = incoming msg; "_r" response msg

RECEIVED msgs directed into GEVCUr function:
 (1) Command cid_gevcur_keepalive_i; // CANID_CMD_GEVCURKAI:U8_S8_S8_S8_S8 : GEVCUr: I KeepAlive and connect command
     payload[0] - reserved for commands
     payload[1] - Torque plus (pct of max)
     payload[2] - Motor speed (Vmax) (pct of max plus)
     payload[3] - Torque minus (pct of max)
     payload[4] - Motor speed (Vminus) (pct of max minus)
 
 (2) 
 (3) function command (diagnostic poll) "cid_cmd_i"
    
SENT by GEVCUr function:
 (1) command "cid_keepalive_r" (response to "cid_keepalive_i")
     payload[0]
       bit 7 - faulted (code in payload[2])
       bit 6 - warning: minimum pre-chg immediate connect.
              (warning bit only resets with power cycle)
		 bit[0]-[3]: Current main state code

     payload[1] = critical error state error code
         0 = No fault
         1 = 

 poll  (response to "cid_gps_sync") & heartbeat
 (2)  "cid_msg1" hv #1 : current #1  battery string voltage:current
 (3)	"cid_msg2" hv #2 : hv #3       DMOC+:DMOC- voltages

 function command "cid_cmd_r"(response to "cid_cmd_i")
 (4)  conditional on payload[0], for example(!)--
      ... (many and sundry)

 heartbeat (sent in absence of keep-alive msgs)
 (5)  "cid_hb1" Same as (2) above
 (6)  "cid_hb2" Same as (3) above
=========================================    
NOTES:
*/

/* Task notification bit assignments for GevcuTask. */
#define GEVCUBIT00	(1 << 0)  // ADCTask has new readings (SKIP!)
#define GEVCUBIT01	(1 << 1)  // SAFE/ACTIVE switch changed state
#define GEVCUBIT02	(1 << 2)  // CP_ZODOMTR switch changed state
#define GEVCUBIT03	(1 << 3)  // ZTENSION pb changed
#define GEVCUBIT04	(1 << 4)  // swtim1 (periodic timer)
#define GEVCUBIT05	(1 << 5)  // spare
// MailboxTask notification bits for CAN msg mailboxes
#define GEVCUBIT06 ( 1 <<  6) // cid_gps_sync
#define GEVCUBIT07 ( 1 <<  7) // cid_cntctr_keepalive_r
#define GEVCUBIT08 ( 1 <<  8) // cid_dmoc_actualtorq
#define GEVCUBIT09 ( 1 <<  9) // cid_dmoc_speed
#define GEVCUBIT10 ( 1 << 10) // cid_dmoc_dqvoltamp
#define GEVCUBIT11 ( 1 << 11) // cid_dmoc_torque
#define GEVCUBIT12 ( 1 << 12) // cid_dmoc_critical_f
#define GEVCUBIT13 ( 1 << 13) // cid_dmoc_hv_status
#define GEVCUBIT14 ( 1 << 14) // cid_dmoc_hv_temps
#define GEVCUBIT15 ( 1 << 15) // cid_gevcur_keepalive_i

/* Event status bit assignments (CoNtaCTor EVent ....) */
#define EVSWTIM1TICK (1 << 0) // 1 = timer1 timed out: counter incremented
#define EVCNTCTR     (1 << 1) // 1 = contactor keepalive timer timeout
#define EVCANCNTCTR  (1 << 2) // 1 = CAN rcv: contactor keepalive/command
#define CNCTEVCACMD  (1 << 3) // 1 = CAN rcv: general purpose command
#define CNCTEVCANKA  (1 << 4) // 1 = CAN rcv: Keep-alive/command
#define CNCTEVCAPOL  (1 << 5) // 1 = CAN rcv: Poll
#define CNCTEVCMDRS  (1 << 6) // 1 = Command to reset
#define CNCTEVCMDCN  (1 << 7) // 1 = Command to connect
#define CNCTEVHV     (1 << 8) // 1 = New HV readings
#define EVNEWADC     (1 << 9) // 1 = New ADC readings

/* Output status bit assignments */
#define CNCTOUT00K1  (1 << 0) // 1 = contactor #1 energized
#define CNCTOUT01K2  (1 << 1) // 1 = contactor #2 energized
#define CNCTOUT02X1  (1 << 2) // 1 = aux #1 closed
#define CNCTOUT03X2  (1 << 3) // 1 = aux #2 closed
#define CNCTOUT04EN  (1 << 4) // 1 = DMOC enable FET
#define CNCTOUT05KA  (1 << 5) // 1 = CAN msg queue: KA status
#define CNCTOUT06KAw (1 << 6) // 1 = contactor #1 energized & pwm'ed
#define CNCTOUT07KAw (1 << 7) // 1 = contactor #2 energized & pwm'ed
#define CNCTOUTUART3 (1 << 8) // 1 = uart3 rx with high voltage timer timed out

/* Number of different CAN id msgs this function sends. */
//#define NUMCANMSGS 6
#define NUMCANMSGS 7 // Add 7th for sending control_law_v1 desired speed

/* Indices for array below of "struct CANTXQMSG canmsg[NUMCANMSGS];" */
#define CID_GEVCUR_KEEPALIVE_R  0 // cid_gevcur_keepalive_r
#define CID_GEVCUR_CTL_LAWV1    6 // Desired speed commanded

/* Number of switches for GEVCU task */
#define NUMGEVCUPUSHBUTTONS 5
// Indices for switch struct pointers
#define PSW_ZTENSION 0 // Pushbutton
#define PSW_ZODOMTR  1 // Pushbutton
#define PSW_PB_ARM   2 // Pushbutton
#define PSW_PB_PREP  3 // Pushbutton
#define PSW_PR_SAFE  4 // Switch pair (SAFE/ACTIVE)

/* States */
enum GEVCU_STATE
{
	CANGATE_RDY,
	CL_CALIB,
	CONTACTOR_START,
	DMOC_HANDSHAKE,
	PC_PARAMS,
	IDLE,
	CL_ACTIVE,
};

/* Fault codes */
enum GEVCU_FAULTCODE
{
	GEVCU_NOFAULT,
};

/* Function command response payload codes. */
enum GEVCU_CMD_CODES
{
	ADCCTLLEVER,		// PC1 IN11 - CL readingswtim1ctr
	ADCRAW5V,         // PC4 IN14 - 5V 
	ADCRAW12V,        // PC2 IN12 - +12 Raw
	ADCSPARE,			// PC5 IN15 - 5V ratiometric spare
	ADCINTERNALTEMP,  // IN17     - Internal temperature sensor
	ADCINTERNALVREF,  // IN18     - Internal voltage reference
};

/* GevcuTask states. */
enum GEVCU_STATES
{
	GEVCU_INIT,   // 
	GEVCU_SAFE_TRANSITION,
	GEVCU_SAFE,
	GEVCU_ACTIVE_TRANSITION,
	GEVCU_ACTIVE,
	GEVCU_ARM_TRANSITION,
	GEVCU_ARM,
};

/* CAN msg array index names. */
enum CANCMD_R
{
	PLACEHOLDER,
};

/* Working struct for Gevcu function/task. */
// Prefixes: i = scaled integer, f = float
// Suffixes: k = timer ticks, t = milliseconds
struct GEVCUFUNCTION
{
   // Parameter loaded either by high-flash copy, or hard-coded subroutine
	struct GEVCULC lc; // Parameters for GEVCU, (lc = Local Copy)

	struct ADCFUNCTION* padc; // Pointer to ADC working struct

	/* swtim1 timer counter. */
	uint32_t swtim1ctr; // Running count
	uint32_t cntctr_ka_ctr; // Contactor keepalive swtim1 tick counter
	uint32_t cntctr_ka_to;  // Contactor keepalive ctr timeout

	/* Events status */
	uint32_t evstat;

	/* Output status */
	uint32_t outstat;
	uint32_t outstat_prev;

	/* Contactor control */
	uint8_t cntctr_cmd; // Payload[0] with command bits

	/* Current fault code */
	enum GEVCU_FAULTCODE faultcode;
	enum GEVCU_FAULTCODE faultcode_prev;

	uint32_t statusbits;
	uint32_t statusbits_prev;

	/* Timings in milliseconds. Converted later to timer ticks. */
	uint32_t ka_k;        // Gevcu polling timer
	uint32_t keepalive_k;

	uint32_t ka_gevcur_k; // GEVCUr keepalive/commmand from PC (ms) 
	uint32_t ka_dmoc_r_k; // DMOC sending keepalive/command (ms)
	uint32_t ka_dmoc_i_k; // DMOC failed to receive timeout (ms)
	uint32_t hbct_k;      // Heartbeat ct: ticks between sending

	TimerHandle_t swtimer1; // Software timer1: command/keep-alive
	TimerHandle_t swtimer2; // Software timer2: multiple purpose delay
	TimerHandle_t swtimer3; // Software timer3: uart RX/keep-alive

	uint32_t timesyncrcvctr; // Time sync CAN msgs received
	uint32_t timesynclimit;

	/* Pointers to incoming CAN msg mailboxes. */
	struct MAILBOXCAN* pmbx_cid_cntctr_keepalive_r; // CANID_CMD_CNTCTRKAR: U8_VAR: Contactor1: R KeepAlive response to pollcid_gevcur_keepalive_i;
	struct MAILBOXCAN* pmbx_cid_gevcur_keepalive_i; // CANID_CMD_GEVCURKAI:U8 : GEVCUr: I KeepAlive and connect command
	struct MAILBOXCAN* pmbx_cid_dmoc_actualtorq; // CANID_DMOC_ACTUALTORQ:I16,   DMOC: Actual Torque: payload-30000
	struct MAILBOXCAN* pmbx_cid_dmoc_speed;      // CANID_DMOC_SPEED:     I16_X6,DMOC: Actual Speed (rpm?)
	struct MAILBOXCAN* pmbx_cid_dmoc_dqvoltamp;  // CANID_DMOC_DQVOLTAMP: I16_I16_I16_I16','DMOC: D volt:amp, Q volt:amp
	struct MAILBOXCAN* pmbx_cid_dmoc_torque;     // CANID_DMOC_TORQUE:    I16_I16,'DMOC: Torque,-(Torque-30000)
	struct MAILBOXCAN* pmbx_cid_dmoc_critical_f; // CANID_DMOC_TORQUE:    NONE',   'DMOC: Critical Fault: payload = DEADB0FF
	struct MAILBOXCAN* pmbx_cid_dmoc_hv_status;  // CANID_DMOC_HV_STATUS: I16_I16_X6,'DMOC: HV volts:amps, status
	struct MAILBOXCAN* pmbx_cid_dmoc_hv_temps;   // CANID_DMOC_HV_TEMPS:  U8_U8_U8,  'DMOC: Temperature:rotor,invert,stator
	struct MAILBOXCAN* pmbx_cid_gps_sync;        // CANID_HB_TIMESYNC:  U8 : GPS_1: U8 GPS time sync distribution msg-GPS time sync msg
	struct MAILBOXCAN* pmbx_cid_drum_tst_stepcmd;// CANID_TST_STEPCMD: U8_FF DRUM1: U8: Enable,Direction, FF: CL position: E4600000

	/* LCD buffer(s) */
	struct SERIALSENDTASKBCB* pbuflcd1;
	struct SERIALSENDTASKBCB* pbuflcd2;
	struct SERIALSENDTASKBCB* pbuflcd3;

	/* Pointers to instantiated pushbutton structs. */
	struct SWITCHPTR* psw[NUMGEVCUPUSHBUTTONS];

	uint8_t safesw_prev; // Safe/Active switch previous state

	uint8_t state;      // Gevcu main state
	uint8_t substateA;  // 
	uint8_t substateB;  // spare substate 

	/* CAN msgs */
	struct CANTXQMSG canmsg[NUMCANMSGS];


};

/* *************************************************************************/
osThreadId xGevcuTaskCreate(uint32_t taskpriority);
/* @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: GevcuTaskHandle
 * *************************************************************************/
void StartGevcuTask(void const * argument);
/*	@brief	: Task startup
 * *************************************************************************/

extern struct GEVCUFUNCTION gevcufunction;
extern osThreadId GevcuTaskHandle;

#endif

