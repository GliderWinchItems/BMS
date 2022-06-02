/******************************************************************************
* File Name          : adcextendsum.c
* Date First Issued  : 07/16/2019
* Description        : Sum sums from adcfastsum16.c for long term smoothing and display
*******************************************************************************/

#include "adcextendsum.h"
#include "adcparams.h"

static uint32_t ctr;

/* *************************************************************************
 * void adcextendsum(struct ADCFUNCTION* p);
 *	@brief	: Sum the 1/2 DMA sums for greater averaging
 * @param	: pcf = pointer to stuct array for ADCs
 * *************************************************************************/
void adcextendsum(struct ADCFUNCTION* p)
{
	struct ADCCHANNEL* pchan = &p->chan[0];

	(pchan+0)->xsum[0] += (pchan+0)->sum;
	(pchan+1)->xsum[0] += (pchan+1)->sum;
	(pchan+2)->xsum[0] += (pchan+2)->sum;
	(pchan+3)->xsum[0] += (pchan+3)->sum;
	(pchan+4)->xsum[0] += (pchan+4)->sum;
	(pchan+5)->xsum[0] += (pchan+5)->sum;

	ctr += 1;
	if (ctr >= ADCEXTENDSUMCT)
	{
		ctr = 0;
		p->idx_xsum ^= 0x1;

		(pchan+0)->xsum[1] = (pchan+0)->xsum[0];
		(pchan+1)->xsum[1] = (pchan+1)->xsum[0];
		(pchan+2)->xsum[1] = (pchan+2)->xsum[0];
		(pchan+3)->xsum[1] = (pchan+3)->xsum[0];
		(pchan+4)->xsum[1] = (pchan+4)->xsum[0];
		(pchan+5)->xsum[1] = (pchan+5)->xsum[0];
		
		(pchan+0)->xsum[0] = 0;
		(pchan+1)->xsum[0] = 0;
		(pchan+2)->xsum[0] = 0;
		(pchan+3)->xsum[0] = 0;
		(pchan+4)->xsum[0] = 0;
		(pchan+5)->xsum[0] = 0;
	}
	return;
}
