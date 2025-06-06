/******************************************************************************
* File Name          : cancomm_items.h
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#ifndef __CANCOMM_ITEMS
#define __CANCOMM_ITEMS

#include "can_iface.h"

// payload [1] U8: TYPE2 Command code
 #define MISCQ_HEARTBEAT   0   // reserved for heartbeat
 #define MISCQ_STATUS      1 // status
 #define MISCQ_CELLV_CAL   2 // cell voltage: calibrated
 #define MISCQ_CELLV_ADC   3 // cell voltage: adc counts
 #define MISCQ_TEMP_CAL    4 // temperature sensor: calibrated
 #define MISCQ_TEMP_ADC    5 // temperature sensor: adc counts
 #define MISCQ_DCDC_V      6 // isolated dc-dc converter output voltage
 #define MISCQ_CHGR_V      7 // charger hv voltage
 #define MISCQ_HALL_CAL    8 // Hall sensor: calibrated
 #define MISCQ_HALL_ADC    9 // Hall sensor: adc counts
 #define MISCQ_CELLV_HI   10 // Highest cell voltage
 #define MISCQ_CELLV_LO   11 // Lowest cell voltage
 #define MISCQ_FETBALBITS 12 // FET on/off discharge bits
 #define MISCQ_DUMP_ON	  13 // Turn on Dump FET for no more than ‘payload [3]’ secs
 #define MISCQ_DUMP_OFF	  14 // Turn off Dump FET
 #define MISCQ_HEATER_ON  15 // Enable Heater mode to ‘payload [3] temperature
 #define MISCQ_HEATER_OFF 16 // Turn Heater mode off.
 #define MISCQ_TRICLE_OFF 17 // Turn trickle charger off fofor no more than ‘payload [3]’ secs



/* *************************************************************************/
 void cancomm_items_sendcell(struct CANRCVBUF* pcan);
/*	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/
 void cancomm_items_sendcmdr(struct CANRCVBUF* pcan);
/*	@brief	: Prepare and send a response to a received command
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/
void cancomm_items_uni_bms(struct CANRCVBUF* pcan);
/*	@brief	: Multi-purpose command (CANCOMMBIT02)
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/

#endif
