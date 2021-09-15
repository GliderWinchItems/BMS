/******************************************************************************
* File Name          : drum_items.h
* Date First Issued  : 09/08/2020
* Description        : Drum function
*******************************************************************************/

#ifndef __DRUM_ITEMS
#define __DRUM_ITEMS

#include <stdint.h>
#include "DrumTask.h"

/* *************************************************************************/
void drum_items_computespeed(struct DRUMFUNCTION* p, uint8_t reqnum);
/* @brief       : Compute speed for the encoder channel
 * @param      : p = pointer to data for channel
 * @param      : reqnum = requester number (0 - n)
* *************************************************************************/
 void drum_items_init(struct DRUMFUNCTION* p);
/* @brief       : Initialization of parameters
 * @param  	: p = pointer to struct with everything for this drum
 * *************************************************************************/

#endif