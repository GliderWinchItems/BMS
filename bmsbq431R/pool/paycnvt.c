/******************************************************************************
* File Name          : paycnvt.c
* Date First Issued  : 12/07/2019
* Description        : Conversions of various payload formats
*******************************************************************************/
#include "paycnvt.h"
#include "morse.h"

/* gen_db.h: has #define for payload name versus code */ 
#include "../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

/* Payload format definitions. 
12/08/2019 copy of PAYLOAD_TYPE_INSERT.sql (edited)
--
    NAME   CODE  DLC    DESCRIPTION12  ([] = payload array index)                   
('NONE',     0,  0, ' No payload bytes');
('FF',       1,  4, ' [0]-[3]: Full Float');			--
('FF_FF',    2,  8, ' [0]-[3]: Full Float[0]; [4]-[7]: Full Float[1]');	--
('U32',		 3,  4, ' [0]-[3]: uint32_t');				--
('U32_U32',	 4,  8, ' [0]-[3]: uint32_t[0]; [4]-[7]: uint32_t[1]');	--
('U8_U32',	 5,  5, ' [0]: uint8_t; [1]-[4]: uint32_t');		--
('S32',		 6,  4, ' [0]-[3]: int32_t');				--
('S32_S32',	 7,  8, ' [0]-[3]: int32_t[0]; [4]-[7]: int32_t[1]');	--
('U8_S32',	 8,  5, ' [0]: int8_t; [4]-[7]: int32_t');		--
('HF',       9,  2, ' [0]-[1]: Half-Float');			--
('F34F',     10,  3, ' [0]-[2]: 3/4-Float');				--
('xFF',      11,  5, ' [0]:[1]-[4]: Full-Float, first   byte  skipped');	--
('xxFF',     12,  6, ' [0]:[1]:[2]-[5]: Full-Float, first 2 bytes skipped');	--
('xxU32',    13,  6, ' [0]:[1]:[2]-[5]: uint32_t, first 2 bytes skipped');	--
('xxS32',    14,  6, ' [0]:[1]:[2]-[5]: int32_t, first 2 bytes skipped');	--
('U8_U8_U32',15,  6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1],uint32_t,');	--
('U8_U8_S32',16,  6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], int32_t,');	--
('U8_U8_FF', 17,  6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], Full Float,');--
('U16',      18,  2, ' [0]-[1]:uint16_t');			--
('S16',      19,  2, ' [0]-[1]: int16_t');			--
('LAT_LON_HT',20, 6, ' [0]:[1]:[2]-[5]: Fix type, bits fields, lat/lon/ht');	--
('U8_FF',    21,  5, ' [0]:[1]-[4]: uint8_t, Full Float');	--
('U8_HF',    22,  3, ' [0]:[1]-[2]: uint8_t, Half Float');	--
('U8',       23,  1, ' [0]: uint8_t');	--
('UNIXTIME', 24,  5, ' [0]: U8_U32 with U8 bit field stuff');	--
('U8_U8',    25,  2, ' [0]:[1]: uint8_t[0],uint8[1]');	--
('U8_U8_U8_U32',   26, 7,'[0]:[1]:[2]:[3]-[5]: uint8_t[0],uint8_t[0],uint8_t[1], int32_t,');	--
('I16_I16',	       27, 4,'[1]-[0]: uint16_t[0]; [3]-[2]: uint16_t[1]');
('I16_I16_X6',     28, 4,'[1]-[0]: uint16_t[0]; [3]-[2]: uint16_t[1]; X');
('U8_U8_U8',       29, 6,'[1]-[2]:[2] uint8_t');--
('I16_X6',         30, 7,'[1]-[0]: uint16_t,[6]: uint8_t');
('I16_I16_I16_I16',31, 8,'[1]-[0]:[3]-[2]:[5]-[4]:[7]-[6]:uint16_t');
('I16__I16',       32, 8,'[1]-[0]:uint16_t,[6]-[5]:uint16_t');
('I16_I16_I16_X7', 33, 8,'[1]-[0]:[3]-[2]:[5]-[4]:uint16_t,[6]:uint8_t');
('I16_I16_X_U8_U8',34, 8,'[1]-[0]:[3]-[2]:uint16_t,[5]:[6]:uint8_t');
('I16',            35, 2,'[1]-[0]:uint16_t');	--
('U8_VAR',         36, 2,'[0]-uint8_t: [1]-[n]: variable dependent on first byte');	--
('U8_S8_S8_S8_S8', 37, 5,'[0]:uint8_t:[1]:[2]:[3]:[4]:int8_t (signed)');	--

('LVL2B',	249,  6, ' [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Board code),[2]-[5]see table');	--
('LVL2R',	250,  6, ' [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Readings code),[2]-[5]see table');	--
('UNDEF',	255,  8, ' Undefined');			--
*/

/* ******************************************************************************** 
 * uintYXX payuYXX(struct CANRCVBUF* pcanx, int offset)
 * @brief	: Combine bytes to make an unsigned int: Y = U or I, XX = 32 or 16
 * @param	: pcanx = pointer to struct with payload
 * @param	: offset = number of bytes to offset for int
 * @return	: uint32_t, or uint16_t
 **********************************************************************************/
// Little endian
uint32_t payU32(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[0+offset] <<  0;
		x |= pcanx->cd.uc[1+offset] <<  8;
		x |= pcanx->cd.uc[2+offset] << 16;
		x |= pcanx->cd.uc[3+offset] << 24;
		return x;
}
uint16_t payU16(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[0+offset] <<  0;
		x |= pcanx->cd.uc[1+offset] <<  8;
		return x;
}
// Big endian
uint32_t payI32(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[3+offset] <<  0;
		x |= pcanx->cd.uc[2+offset] <<  8;
		x |= pcanx->cd.uc[1+offset] << 16;
		x |= pcanx->cd.uc[0+offset] << 24;
		return x;
}
uint16_t payI16(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[1+offset] <<  0;
		x |= pcanx->cd.uc[0+offset] <<  8;
		return x;
}
float payFF(struct CANRCVBUF* pcanx, int offset)
{
	union{uint32_t ui; float ff;} ui_ff;

		ui_ff.ui  = pcanx->cd.uc[0+offset] <<  0;
		ui_ff.ui |= pcanx->cd.uc[1+offset] <<  8;
		ui_ff.ui |= pcanx->cd.uc[2+offset] << 16;
		ui_ff.ui |= pcanx->cd.uc[3+offset] << 24;
		return ui_ff.ff;
}

/* ******************************************************************************** 
 * union UNION_PAY convertpayload(struct CANRCVBUF* pcanx, uint8_t paycode, uint8_t k);
 * @brief	: Extract payload bytes
 * @param	: pcanx = pointer to CAN msg
 * @param	: paycode = type of payload (e.g. I16__I16' = 32)
 * @param	: k = payload item index: (0 - 7)
 * @return	: UNION_PAY = holds extracted payload item
 **********************************************************************************/
union UNION_PAY convertpayload(struct CANRCVBUF* pcanx, uint8_t paycode, uint8_t k)
{
	union UNION_PAY ui_ff;	
	ui_ff.ff = 0;

	switch(paycode)
	{ 
	case I16:
	case I16_I16:
	case I16_I16_I16_I16:
		ui_ff.ui = payI16(pcanx,k*2);
		break;

	case I16_X6:
		if (k == 0) {ui_ff.ui = payI16(pcanx,0); break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[5]; break;}
		break;

	case I16_I16_X6:
		if (k != 2) {ui_ff.ui = payI16(pcanx,k*2); break;}
		if (k == 2) {ui_ff.ui = pcanx->cd.uc[5];   break;}
		break;

	case I16__I16:
		if (k == 1) k = 2;
	case I16_I16_I16_X6:
		if (k != 3) {ui_ff.ui = payI16(pcanx,k*2);break;}
		if (k == 3) {ui_ff.ui = pcanx->cd.uc[6];  break;}
		break;

	case I16_I16_X_U8_U8:
		if (k < 2)  {ui_ff.ui = payI16(pcanx,k*2);break;}
		if (k > 2)  {ui_ff.ui = pcanx->cd.uc[k];  break;}
		break;

	case NONE:
		break;

	case U8:
	case U8_U8:
	case U8_U8_U8:
		ui_ff.ui = pcanx->cd.uc[k];
		break;

	case U32:
	case U32_U32:
		ui_ff.ui = payU32(pcanx,k*4);
		break;

	case xxU32:
		k = 1;
	case UNIXTIME:
	case U8_U32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = payU32(pcanx,1); break;}
		break;

	case U8_U8_U32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1]; break;}
		if (k == 2) {ui_ff.ui = payU32(pcanx,2); break;}
		break;
		
	case U8_U8_S32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1]; break;}
		if (k == 2) 
		{
			ui_ff.ui = payU32(pcanx,2); 
		}
		break;

	case xxS32:
		k = 1;
	case S32:
	case S32_S32:
		ui_ff.ui = payU32(pcanx,k*4);
		break;

	case U8_S32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = payU32(pcanx,1);}
		break;

	case FF:
	case FF_FF:
		ui_ff.ff = payFF(pcanx,k*4);
		break;

	case xxFF:
		k = 1;
	case U8_FF:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ff = payFF(pcanx,1);}
		break;

	case U8_U8_FF:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0];    break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1];    break;}
		if (k == 2) {ui_ff.ff = payFF(pcanx,2); }
		break;		

	default: // Payload type requested not handled
		morse_trap(38); // Programming trap
		break;
	}

	return ui_ff;
}	

