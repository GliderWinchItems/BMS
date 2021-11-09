/******************************************************************************
* File Name          : cancomm_items.h
* Date First Issued  : 11/07/2021
* Description        : CanCommTask adjuncts
*******************************************************************************/
#ifndef __CANCOMM_ITEMS
#define __CANCOMM_ITEMS

#include "can_iface.h"

// payload [1] U8: Command code
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



/* *************************************************************************/
 void cancomm_items_sendcell(struct CANRCVBUF* pcan);
/*	@brief	: Prepare and queue CAN msgs for sending cell voltage array
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/
 void cancomm_items_sendcmdr(struct CANRCVBUF* pcan);
/*	@brief	: Prepare and send a response to a received command
 *  @param  : pcan = pointer to struct CANRCVBUF from mailbox 
 * *************************************************************************/

#endif
