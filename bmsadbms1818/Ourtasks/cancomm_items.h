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
 #define MISCQ_HEARTBEAT   0   // reserved for heartbeat
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
 #define MISCQ_FETBALBITS 12 // Read FET on/off discharge bits
 #define MISCQ_DUMP_ON	  13 // Turn on Dump FET for no more than ‘payload [3]’ secs
 #define MISCQ_DUMP_OFF	  14 // Turn off Dump FET
 #define MISCQ_HEATER_ON  15 // Enable Heater mode to ‘payload [3] temperature
 #define MISCQ_HEATER_OFF 16 // Turn Heater mode off.
 #define MISCQ_TRICKL_OFF 17 // Turn trickle charger off for no more than ‘payload [3]’ secs
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage
 #define MISCQ_PROC_CAL   19 // Processor ADC calibrated readings
 #define MISCQ_PROC_ADC   20 // Processor ADC raw adc counts for making calibrations
 #define MISCQ_R_BITS     21 // Dump, dump2, heater, discharge bits
 #define MISCQ_CURRENT_CAL 24 // Below cell #1 minus, current resistor: calibrated
 #define MISCQ_CURRENT_ADC 25 // Below cell #1 minus, current resistor: adc counts
 #define MISCQ_UNIMPLIMENT 26 // Command requested is not implemented
 #define MISCQ_SETFETBITS  27 // Set FET on/off discharge bits

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
