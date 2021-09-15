/******************************************************************************
* File Name          : drum_items.c
* Date First Issued  : 09/08/2020
* Description        : Drum function
*******************************************************************************/

#include "stm32f4xx_hal.h"
#include "drum_items.h"

extern TIM_HandleTypeDef htim2;

/* *************************************************************************
 * void drum_idx_v_struct_hardcode_params(struct DRUMFUNCTION* p);
 * @brief       : Initialization of parameters
 * *************************************************************************/
void drum_idx_v_struct_hardcode_params(struct DRUMLC* p)
{
	p->Kencodercountperrev = 720; // Encoder counts per revolution
	p->Kdrumdia    = 0.9947f;     // Drum diameter (meters)
	p->Kgearratio  = 3.80f;       // Encoder:drum axle gear ratio
	return;
}
/* *************************************************************************
 * void drum_items_init(struct DRUMFUNCTION* p);
 * @brief       : Initialization of parameters
 * @param  	: p = pointer to struct with everything for this drum
 * *************************************************************************/
void drum_items_init(struct DRUMFUNCTION* p)
{
	/* Load parameters */
	drum_idx_v_struct_hardcode_params(&p->lc);

	/* Computed from parameters. */
	// Factor: Encoder counts to encoder rev/min
	p->Fspeed_rpm_encoder  = ( (60.0f * 84000000.0f) / p->lc.Kencodercountperrev);

	// Factor: Cable distance = (drum_dia * pi) / (enccoder_counts_per_rev * encoder:drum_axle ratio)
	p->Fcable_distance  = (p->lc.Kdrumdia * 3.14159265f) / (p->lc.Kencodercountperrev * p->lc.Kgearratio);

	// Factor: Cable speed = encoder_rpm * ((drum_dia * pi) / (gear_ratio * 60(sec/min) )
	p->Fspeed_cable     = (p->lc.Kdrumdia * 3.14159265f) / (p->lc.Kgearratio * 60.0f);

	p->cablezeroref  = 0; // Encoder counter reference/offset for zero cable

	return;	
}

/* *************************************************************************
 * void drum_items_computespeed(struct DRUMFUNCTION* p, uint8_t reqnum);
 * @brief       : Compute speed for the encoder channel
 * @param 		: p = pointer to data for channel
 * @param  		: reqnum = requester number (0 - n)
 * *************************************************************************/
void drum_items_computespeed(struct DRUMFUNCTION* p, uint8_t reqnum)
{
	/* Copy latest time and count, and be sure it didn't change mid-copy. */
	do
	{ 
		 p->decA[reqnum].tmp =  p->decA[reqnum].cur; // Copy time and count set by ISR

		// Loop until re-read results in same two readings.
	} while ( ( p->decA[reqnum].tmp.tim !=  p->decA[reqnum].cur.tim) || ( p->decA[reqnum].tmp.cnt !=  p->decA[reqnum].cur.cnt) );

	/* Get input capture time now */
	//p->sampletim = htim2.Instance->CNT;

	/* Compute number of encoder ticks between latest encoder count
	     and latest encoder count on previous pass thru this routine.  */
	 p->decA[reqnum].diff.cnt =  p->decA[reqnum].tmp.cnt -  p->decA[reqnum].prev.cnt;
	if ( p->decA[reqnum].diff.cnt == 0)
	{ // Here, no encoder counts between sample times.
		p->Cspeed_rpm_encoder = 0; // Assume the speed is zero.
		p->Cspeed_cable = 0;
		return;
	}
	/* Here, one or more encoder counts between sample times. */

	/* Compute time difference between latest encoder input capture
	     and latest encoder input capture on previous pass thru 
	     this routine */
	 p->decA[reqnum].diff.tim =  p->decA[reqnum].tmp.tim -  p->decA[reqnum].prev.tim;

	/* The current readings are now the previous readings. */
	 p->decA[reqnum].prev =  p->decA[reqnum].tmp;

	/* Encoder speed (rpm). Note that diff.cnt is signed. */
	p->Cspeed_rpm_encoder = p->Fspeed_rpm_encoder * (float) p->decA[reqnum].diff.cnt / (float) p->decA[reqnum].diff.tim;

	/* Cable speed (meters/sec) = Encoder_rpm * Factor */
	p->Cspeed_cable = p->Cspeed_rpm_encoder * p->Fspeed_cable;

	// Cable out (meters) = Factor * net_encoder_counts
	p->Ccable_distance = p->Fcable_distance * (float)(p->cablezeroref +  p->decA[reqnum].tmp.cnt);

	return;
}