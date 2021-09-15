/******************************************************************************
* File Name          : adcextendsum.c
* Date First Issued  : 07/16/2019
* Description        : Sum sums from adcfastsum16.c for long term smoothing and display
*******************************************************************************/

#include "adcextendsum.h"
#include "adcparams.h"

/* *************************************************************************
 * void adcextendsum(struct ADCFUNCTION* p);
 *	@brief	: Sum the 1/2 DMA sums for greater averaging
 * @param	: pcf = pointer to stuct array for ADCs
 * *************************************************************************/
void adcextendsum(struct ADCFUNCTION* p)
{
	struct ADCCHANNEL* pchan = &p->chan[0];

	(pchan+ 0)->xsum[0] += (pchan+ 0)->sum;
	(pchan+ 1)->xsum[0] += (pchan+ 1)->sum;
	(pchan+ 2)->xsum[0] += (pchan+ 2)->sum;
	(pchan+ 3)->xsum[0] += (pchan+ 3)->sum;
	(pchan+ 4)->xsum[0] += (pchan+ 4)->sum;
	(pchan+ 5)->xsum[0] += (pchan+ 5)->sum;
	(pchan+ 6)->xsum[0] += (pchan+ 6)->sum;
	(pchan+ 7)->xsum[0] += (pchan+ 7)->sum;
	(pchan+ 8)->xsum[0] += (pchan+ 8)->sum;
	(pchan+ 9)->xsum[0] += (pchan+ 9)->sum;
	(pchan+10)->xsum[0] += (pchan+10)->sum;
	(pchan+11)->xsum[0] += (pchan+11)->sum;
	(pchan+12)->xsum[0] += (pchan+12)->sum;
	(pchan+13)->xsum[0] += (pchan+13)->sum;
	(pchan+14)->xsum[0] += (pchan+14)->sum;
	(pchan+15)->xsum[0] += (pchan+15)->sum;
	(pchan+16)->xsum[0] += (pchan+16)->sum;
	(pchan+17)->xsum[0] += (pchan+17)->sum;

	p->ctr += 1;
	if (p->ctr >= ADCEXTENDSUMCT)
	{
		p->ctr = 0;
		p->idx_xsum ^= 0x1;
		
		(pchan+ 0)->xsum[0] = 0;
		(pchan+ 1)->xsum[0] = 0;
		(pchan+ 2)->xsum[0] = 0;
		(pchan+ 3)->xsum[0] = 0;
		(pchan+ 4)->xsum[0] = 0;
		(pchan+ 5)->xsum[0] = 0;
		(pchan+ 6)->xsum[0] = 0;
		(pchan+ 7)->xsum[0] = 0;
		(pchan+ 8)->xsum[0] = 0;
		(pchan+ 9)->xsum[0] = 0;
		(pchan+10)->xsum[0] = 0;
		(pchan+11)->xsum[0] = 0;
		(pchan+12)->xsum[0] = 0;
		(pchan+13)->xsum[0] = 0;
		(pchan+14)->xsum[0] = 0;
		(pchan+15)->xsum[0] = 0;
		(pchan+16)->xsum[0] = 0;
		(pchan+17)->xsum[0] = 0;
	}
	return;
}
