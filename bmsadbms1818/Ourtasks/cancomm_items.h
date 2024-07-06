/******************************************************************************
* File Name          : cancomm_items.h
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#ifndef __CANCOMM_ITEMS
#define __CANCOMM_ITEMS

#include "can_iface.h"

#define MAXNUMCELLMSGS 6 // Number of CAN msgs to send all cell readings

#define CANCOMMITEMSNOTE00 (1<<0) // CanCommTask TaskWait notification bit

// payload [1] U8: TYPE2 Command code
// NOTE: Skipped codes are available for new subcommands
 #define MISCQ_HEARTBEAT   0 // reserved for heartbeat
 #define MISCQ_STATUS      1 // status
 #define MISCQ_CELLV_CAL   2 // cell voltage: calibrated
 #define MISCQ_CELLV_ADC   3 // cell voltage: adc counts
 #define MISCQ_TEMP_CAL    4 // temperature sensor: calibrated
 #define MISCQ_TEMP_ADC    5 // temperature sensor: adc counts for making calibration
 #define MISCQ_DCDC_V      6 // isolated dc-dc converter output voltage
 #define MISCQ_CHGR_V      7 // charger hv voltage
 #define MISCQ_HALL_CAL    8 // Hall sensor: calibrated
 #define MISCQ_HALL_ADC    9 // Hall sensor: adc counts for making calibration
 #define MISCQ_CELLV_HI   10 // Highest cell voltage
 #define MISCQ_CELLV_LO   11 // Lowest cell voltage
 #define MISCQ_FETBALBITS 12 // Read FET on|off discharge bits
 #define MISCQ_SET_DUMP	  13 // Set ON|OFF DUMP FET on|off
 #define MISCQ_SET_DUMP2  14 // Set ON|OFF DUMP2 FET FET: on|off
 #define MISCQ_SET_HEATER 15 // Set ON|OFF HEATER FET on|off
 #define MISCQ_TRICKL_OFF 17 // Turn trickle charger off for no more than ‘payload [3]’ secs
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage
 #define MISCQ_PROC_CAL   19 // Processor ADC calibrated readings
 #define MISCQ_PROC_ADC   20 // Processor ADC raw adc counts for making calibrations
 #define MISCQ_R_BITS     21 // Dump, dump2, heater, discharge bits
 #define MISCQ_CURRENT_CAL 24 // Below cell #1 minus, current resistor: calibrated
 #define MISCQ_CURRENT_ADC 25 // Below cell #1 minus, current resistor: adc counts
 #define MISCQ_UNIMPLIMENT 26 // Command requested is not implemented
 #define MISCQ_SET_FETBITS  27 // Set FET on/off discharge bits
 #define MISCQ_SET_DCHGTST  28 // Set discharge test via heater fet load on|off
 #define MISCQ_SET_DCHGFETS 30 // Set discharge FETs: all, on|off, or single
 #define MISCQ_SET_SELFDCHG 31 // Set ON|OFF self-discharge mode
 #define MISCQ_PRM_MAXCHG   32 // Get Parameter: Max charging current
 #define MISCQ_SET_ZEROCUR  33 // 1 = Zero external current in effect; 0 = maybe not.
 #define MISCQ_READ_AUX     34 // BMS responds with A,B,C,D AUX register readings (12 msgs)
 #define MISCQ_READ_ADDR    35 // BMS responds with 'n' bytes sent in [3]
 #define MISCQ_PROC_TEMP    36 // Processor calibrated internal temperature (deg C)
 #define MISCQ_CHG_LIMITS   37 // Show params: Module V max, Ext chg current max, Ext. chg bal
 #define MISCQ_MORSE_TRAP   38 // Retrieve stored morse_trap code.
 #define MISCQ_FAN_STATUS   39 // Retrieve fan: pct and rpm 



/* Keep alive for incoming CAN msgs that cause battery loads. */
#define CANSETFET_TIM 5000 // Timeout (ms) for MISCQ_SET_DCHGFETS

/* MISCQ_SET_DCHGFETS Sub code for sending request. 
Requester payload[3] 
  0 = All FETs off
  1-18 = FET number to turn ON.
  111  = All FETs on
*/  


/* *************************************************************************/
 void cancomm_items_init(void);
/* @brief	: Initialization
 * *************************************************************************/
void cancomm_items_sendcell(struct CANRCVBUF* pcan, float *pf);
/*	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 *  @param  : pf = pointer cell array
 * *************************************************************************/
 void cancomm_items_sendcmdr(struct CANRCVBUF* pi);
/*  @brief	: Prepare and send a response to a received command CAN msg
 *  @param  : pi = pointer to incoming CAN msg struct CANRCVBUF from mailbox 
 * *************************************************************************/
 void cancomm_items_filter(uint16_t* pi);
/*	@brief	: Pass raw readings through filter 
 *  @param  : pi = pointer to array with uint16_t raw readings (sequence correct)
 * *************************************************************************/

#endif
