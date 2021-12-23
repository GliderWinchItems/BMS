/******************************************************************************
* File Name          : canfilter_setup.c
* Date First Issued  : 01/10/2019
* Description        : CAN FreeRTOS/ST HAL: Hardware filtering. (32b only)
*******************************************************************************/
#include "canfilter_setup.h"

#include "CanTask.h"
#include "can_iface.h"

struct CANFILTERW
{
	CAN_FilterTypeDef filt;  // HAL filter struct
	uint8_t banknum;         // Filter bank number: next available (pair 32b words)
	uint8_t odd;             // Filter bank 32b reg pair, next : 0 = even, 1 = odd					
	uint8_t oto_sw;          // OTO setup switch for struct
};

static struct CANFILTERW canfilt1 = {0};
static struct CANFILTERW canfilt2 = {0};
static struct CANFILTERW canfilt3 = {0};

/* *************************************************************************
 * HAL_StatusTypeDef canfilter_setup_first(uint8_t cannum, CAN_HandleTypeDef *phcan, uint8_t slavebankdmarc);
 * @brief	: Sets Bank 0 to pass ==>all<== msgs to FIFO 0, 32b mask mode
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: slavebankdmarc = For CAN2, filter bank number demarcation, where CAN2 banks start
 * @return	: HAL_ERROR or HAL_OK
 * *************************************************************************/
/* NOTE: 'banknum' is left at zero, odd zero, so that the next addition will override
   the "accept all" configuration of this setup.

	Some of the values in the HAL struct do not need to be updated, e.g. 'FilterActivation' hence
   the call to  'first"' sets these up.
*/
HAL_StatusTypeDef canfilter_setup_first(uint8_t cannum, CAN_HandleTypeDef *phcan, uint8_t slavebankdmarc)
{
	struct CANFILTERW* p;
	HAL_StatusTypeDef ret;

	switch(cannum)
	{
	case 0:	p = &canfilt1; break; // CAN 1
	case 1: 	p = &canfilt2; break; // CAN 2
	case 2:	p = &canfilt3; break; // CAN 3
	default:		return HAL_ERROR;
	} // CAN1 & CAN3 start at zero
	if (cannum != 2)
	{ // Here, CAN 1 or CAN 3
		p->filt.FilterBank = 0;  // Filter bank number
	}
	else
	{ // Here, CAN2 slave filter bank is shifted
		p->filt.FilterBank = slavebankdmarc;  // CAN1-CAN2 bank demarcation
	}
	p->filt.FilterIdHigh         = 0;
	p->filt.FilterIdLow          = 0;
	p->filt.FilterMaskIdHigh     = 0;
	p->filt.FilterMaskIdLow      = 0;
	p->filt.FilterFIFOAssignment = 0;	// FIFO 0
	p->filt.FilterMode           = CAN_FILTERMODE_IDMASK;
	p->filt.FilterScale          = CAN_FILTERSCALE_32BIT;
	p->filt.FilterActivation     = ENABLE;
	p->filt.SlaveStartFilterBank = slavebankdmarc; // No meaning for CAN3
	ret = HAL_CAN_ConfigFilter(phcan, &p->filt); // Store in hardware
	p->oto_sw = 1;
	p->banknum = 0;
	p->odd     = 0;
	return ret;
}
/* *************************************************************************
 * HAL_StatusTypeDef canfilter_setup_add32b_mskmode(uint8_t cannum, \
	 CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint32_t msk,  \
    uint8_t  fifo  );
 * @brief	: Add a 32b  id and mask to a specified filter bank
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: id    = 32b CAN id
 * @param	: msk   = 32b mask (0's are don't cares)
 * @param	: fifo  = fifo: 0 or 1
 * @return	: HAL_ERROR or HAL_OK
 * *************************************************************************/
HAL_StatusTypeDef canfilter_setup_add32b_mskmode(uint8_t cannum, \
	 CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint32_t msk,  \
    uint8_t  fifo  )
{
	struct CANFILTERW* p;
	HAL_StatusTypeDef ret;

	switch(cannum)
	{
	case 1:	p = &canfilt1; break; // CAN 1
	case 2: 	p = &canfilt2; break; // CAN 2
	case 3:	p = &canfilt3; break; // CAN 3
	default:		return HAL_ERROR;
	}

	/* Make sure the first setup was made */
	if (p->oto_sw == 0)
	{
		canfilter_setup_first(cannum, phcan, 14);
	}

	/* Check for bozo programmer call. */
	if ((cannum == 2) && (p->banknum <= p->filt.SlaveStartFilterBank))
	{ // Here, bank number is in CAN1 area
		return HAL_ERROR;
	}

	/* 32b id & mask take a complete filter bank */
	if (p->odd != 0)
	{ // Here one register availalbe in current banknum
		p->banknum += 1;	// Advance to next bank
		p->odd = 0;
	}

	p->filt.FilterBank       = p->banknum;
	p->filt.FilterIdHigh     = (id  >> 16) & 0xffff;
	p->filt.FilterIdLow      = (id  >>  0) & 0xffff;
	p->filt.FilterMaskIdHigh = (msk >> 16) & 0xffff;
	p->filt.FilterMaskIdLow  = (msk >>  0) & 0xffff;
	p->filt.FilterFIFOAssignment = fifo & 0x1;
	p->filt.FilterMode           = CAN_FILTERMODE_IDMASK;
	ret = HAL_CAN_ConfigFilter(phcan, &p->filt); // Store in hardware

	p->banknum += 1;	// Advance to next bank
	return ret;
}

/* *************************************************************************
 * HAL_StatusTypeDef canfilter_setup_add32b_id(uint8_t cannum, CAN_HandleTypeDef *phcan, \
    uint32_t id,   \ 
    uint8_t  fifo );
 * @brief	: Add a 32b id, advance bank number & odd/even
 * @param	: cannum = CAN module number 1, 2, or 3
 * @param	: phcan = Pointer to HAL CAN handle (control block)
 * @param	: id    = 32b CAN id
 * @param	: fifo  = fifo: 0 or 1
 * @return	: HAL_ERROR or HAL_OK
 * *************************************************************************/
HAL_StatusTypeDef canfilter_setup_add32b_id(uint8_t cannum, CAN_HandleTypeDef *phcan, \
    uint32_t id,   \
    uint8_t  fifo )
{
	struct CANFILTERW* p;
	HAL_StatusTypeDef ret;

	if (phcan == NULL) return  HAL_ERROR;

	switch(cannum)
	{
	case 1:	p = &canfilt1; break; // CAN 1
	case 2: 	p = &canfilt2; break; // CAN 2
	case 3:	p = &canfilt3; break; // CAN 3
	default:		return HAL_ERROR;
	}

	/* Make sure the first setup was made */
	if (p->oto_sw == 0)
	{ // If not setup, use default for CAN2 bank demarcation
		ret = canfilter_setup_first(cannum, phcan, 14);
		if (ret == HAL_ERROR) return HAL_ERROR;
	}

	/* Check for bad CAN1,2 bank number */
	if ((cannum == 2) && (p->banknum <= p->filt.SlaveStartFilterBank))
	{ // Here, bank number is in CAN1 area, but CAN2 request
		return HAL_ERROR;
	}

	if (p->odd != 0)
	{ // Here, next available is in the odd position

		/* Advance one of ID (out of pair for one bank) */
		p->odd = 0;	// Reset to even
		p->banknum += 1; // Advance bank number
		if (p->banknum >= 28) return HAL_ERROR; // Oops check

		/* Setup First position of pair with ID */
		p->filt.FilterMaskIdHigh = (id >> 16) & 0xffff;
		p->filt.FilterMaskIdLow  = (id >>  0) & 0xffff;

	}
	else
	{ // Here, next position is in the first/even of pair. */
		/* Setup 2nd position of pair with ID */
		p->filt.FilterIdHigh = (id >> 16) & 0xffff;
		p->filt.FilterIdLow  = (id >>  0) & 0xffff;
		p->odd = 1;
	}
	p->filt.FilterBank = p->banknum;
	p->filt.FilterFIFOAssignment = fifo & 0x1;
	p->filt.FilterMode           = CAN_FILTERMODE_IDLIST;
	p->filt.FilterActivation     = ENABLE;
	ret = HAL_CAN_ConfigFilter(phcan, &p->filt); // Store in hardware

	return ret;
}

