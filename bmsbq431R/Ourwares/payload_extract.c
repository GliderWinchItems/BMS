/******************************************************************************
* File Name          : payload_extract.c
* Date First Issued  : 02/23/2019
* Description        : Extract payload from CAN msg
*******************************************************************************/

#include "payload_extract.h"

/* Definitions of payload type generated from database. */
#include "../../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* NOTE:
If the CAN msg does not have a DLC big enough to accommodate the payload
format designated the mailbox pre8 and union are not updated, nor is the
update counter incremented.  The DTW in the CAN msg will have been updated
when the CAN msg was received.  

Therefore, checking for a stale CAN reading by checking only the DTW could 
result in stale readings not being detected if the DLC reading is wrong.  This
situation would be some sort of software error whereby "someone" is sends a
CAN msg with a bogus CAN id, or payload that is incorrect.

The CAN msg embedded in the mailbox is always copied from the circular buffer
receiving incoming CAN msgs.

The following are not implemented, however the payload will be loaded into
the eight byte union--
F34F	3/4 float
HF    1/2 float
LAT_LON_HT
*/

/* ************************************************************************* 
 * void payload_extract(struct MAILBOXCAN* pmbx);
 *	@brief	: Lookup CAN ID and load mailbox with extract payload reading(s)
 * @param	: pmbx  = pointer to mailbox
 * *************************************************************************/
void payload_extract(struct MAILBOXCAN* pmbx)
{
	switch (pmbx->paytype)
	{
	case U8:
	case U8_VAR:
		if (pmbx->ncan.can.dlc >= 1)
		{
			pmbx->mbx.pre8[0] = pmbx->ncan.can.cd.uc[0];
		}
		break;		
	case FF:
	case U32:
	case S32:
		if (pmbx->ncan.can.dlc >= 4)
		{ // Place 1st four bytes of payload in union
			pmbx->mbx.u.i32[0] = pmbx->ncan.can.cd.ui[0];
			pmbx->ctr +=1 ;
		}
		break;	
	case xFF:
		if (pmbx->ncan.can.dlc >= 5)
		{ // Place [1]-[4] of payload in union 
			pmbx->mbx.u.i8[0] = pmbx->ncan.can.cd.uc[1];
			pmbx->mbx.u.i8[1] = pmbx->ncan.can.cd.uc[2];
			pmbx->mbx.u.i8[2] = pmbx->ncan.can.cd.uc[3];
			pmbx->mbx.u.i8[3] = pmbx->ncan.can.cd.uc[4];
			pmbx->ctr +=1 ;
		}
		break;	
	case xxFF:
	case xxU32:
	case xxS32:
		if (pmbx->ncan.can.dlc >= 6)
		{ // Place [2]-[5] of payload in union 
			pmbx->mbx.u.i8[0] = pmbx->ncan.can.cd.uc[2];
			pmbx->mbx.u.i8[1] = pmbx->ncan.can.cd.uc[3];
			pmbx->mbx.u.i8[2] = pmbx->ncan.can.cd.uc[4];
			pmbx->mbx.u.i8[3] = pmbx->ncan.can.cd.uc[5];
			pmbx->ctr +=1 ;
		}
		break;
	case U8_FF:
	case U8_U32:
	case U8_S32:
	case UNIXTIME:
		if (pmbx->ncan.can.dlc >= 5)
		{ 
			pmbx->mbx.pre8[0] = pmbx->ncan.can.cd.uc[0];
			// Place [2]-[5] of payload in union 
			pmbx->mbx.u.i8[0] = pmbx->ncan.can.cd.uc[1];
			pmbx->mbx.u.i8[1] = pmbx->ncan.can.cd.uc[2];
			pmbx->mbx.u.i8[2] = pmbx->ncan.can.cd.uc[3];
			pmbx->mbx.u.i8[3] = pmbx->ncan.can.cd.uc[4];
			pmbx->ctr +=1 ;		
		}
		break;	
	case U8_U8_FF:
	case U8_U8_U32:
	case U8_U8_S32:
		if (pmbx->ncan.can.dlc >= 6)
		{ 
			pmbx->mbx.pre8[0] = pmbx->ncan.can.cd.uc[0];
			pmbx->mbx.pre8[1] = pmbx->ncan.can.cd.uc[1];
			// Place [2]-[5] of payload in union 
			pmbx->mbx.u.i8[0] = pmbx->ncan.can.cd.uc[2];
			pmbx->mbx.u.i8[1] = pmbx->ncan.can.cd.uc[3];
			pmbx->mbx.u.i8[2] = pmbx->ncan.can.cd.uc[4];
			pmbx->mbx.u.i8[3] = pmbx->ncan.can.cd.uc[5];
			pmbx->ctr +=1 ;		
		}
		break;
	case U8_U8_U8_U32:
		if (pmbx->ncan.can.dlc >= 7)
		{ 
			pmbx->mbx.pre8[0] = pmbx->ncan.can.cd.uc[0];
			pmbx->mbx.pre8[1] = pmbx->ncan.can.cd.uc[1];
			pmbx->mbx.pre8[2] = pmbx->ncan.can.cd.uc[2];
			// Place [2]-[5] of payload in union 
			pmbx->mbx.u.i8[0] = pmbx->ncan.can.cd.uc[3];
			pmbx->mbx.u.i8[1] = pmbx->ncan.can.cd.uc[4];
			pmbx->mbx.u.i8[2] = pmbx->ncan.can.cd.uc[5];
			pmbx->mbx.u.i8[3] = pmbx->ncan.can.cd.uc[6];
			pmbx->ctr +=1 ;		
		}
		break;
	case FF_FF:		// Two four byte readings
	case U32_U32:
	case S32_S32:
		if (pmbx->ncan.can.dlc >= 8)
		{ // Place [0]-[7] of payload in union 
			pmbx->mbx.u.i64 = pmbx->ncan.can.cd.ull;
			pmbx->ctr +=1 ;
		}
		break;	

	// Payload type not implemented
	case UNDEF:
	default: 
		{ // Place [0]-[7] of payload in union 
			pmbx->mbx.u.i64 = pmbx->ncan.can.cd.ull;
			pmbx->ctr +=1 ;
		}
		break;	
	}

	return;
}
