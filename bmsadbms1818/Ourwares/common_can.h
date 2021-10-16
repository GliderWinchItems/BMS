/******************************************************************************
* File Name          : common_can.h
* Date First Issued  : 12/27/2012;01/11/2019
* Board              : STM32CubeMX/FreeRTOS 
* Description        : includes "common" for CAN
*******************************************************************************/
/*
01/11/2019 Edited version from svn_common/trunk

10/16/2013 Copy of svn_sensor/common_all/trunk/common_can.h (svn_sensor rev 259)
*/


#ifndef __MX_COMMON_CAN
#define __MX_COMMON_CAN

#include <stdint.h>

#include "common_misc.h"

#ifndef NULL 
#define NULL	0
#endif

#define CAN_IDE	0x4	// IDE bit in CAN msg
#define CAN_RTR 0x2	// RTR bit in CAN msg
#define CAN_EXTENDED_MASK 0x001FFFF8	// Bits in extended address mask

/* Buffering incoming CAN messages */
union CANDATA	// Unionize for easier cooperation amongst types
{
   unsigned long long ull;
   signed long long   sll;
   double             dbl;
	u32 	   ui[2];
	u16 	   us[4];
	u8 	   uc[8];
	u8	   u8[8];
	s32        si[2];
	s16        ss[4];
	s8         sc[8];
	float	    f[2];
};
struct CANRCVBUF		// Combine CAN msg ID and data fields
{ //                               offset  name:     verbose desciption
	u32 id;			// 0x00 CAN_TIxR: mailbox receive register ID p 662
	u32 dlc;		// 0x04 CAN_TDTxR: time & length p 660
	union CANDATA cd;	// 0x08,0x0C CAN_TDLxR,CAN_TDLxR: Data payload (low, high)
};
struct CANRCVTIMBUF		// CAN data plus timer ticks
{
	union LL_L_S	U;	// Linux time, offset, in 1/64th ticks
	struct CANRCVBUF R;	// CAN data
};
struct CANRCVSTAMPEDBUF
{
	union LL_L_S	U;	// Linux time, offset, in 1/64th ticks
	u32 id;			// 0x00 CAN_TIxR: mailbox receive register ID p 662
	u32 dlc;		// 0x04 CAN_TDTxR: time & length p 660
	union CANDATA cd;	// 0x08,0x0C CAN_TDLxR,CAN_TDLxR: Data payload (low, high)	
};

struct PP
{
	char 	*p;
	u32	ct;
};

#define GPSSAVESIZE	80	// Max size of saved GPS line (less than 256)
struct GPSPACKETHDR
{
	union LL_L_S	U;	// Linux time, offset, in 1/64th ticks
	u32 		id;	// Fake msg ID for non-CAN messages, such as GPS
	u8 c[GPSSAVESIZE];	// 1st byte is count; remaining bytes are data
};

union CANPC
{
	struct CANRCVBUF	can;		// Binary msg
	u8 c[sizeof(struct CANRCVBUF)+2];	// Allow for chksum w longest msg
};

struct CANPCWRAP	// Used with gateway (obsolete except for old code)
{
	union CANPC can;
	u32	chk;
	u8	*p;
	u8	prev;
	u8	c1;
	s16 	ct;
};

/* The following are used in the PC program and USART1_PC_gateway.  (Easier to find them here.) */

#define PC_TO_GATEWAY_ID	(' ')	// Msg to/from PC is for the gateway unit
#define PC_TO_CAN_ID		0x0	// Msg to/from PC is for the CAN bus

#define CAN_PC_FRAMEBOUNDARY	'\n'	// Should be a value not common in the data
#define CAN_PC_ESCAPE		0X7D	// Should be a value not common in the data

#define CHECKSUM_INITIAL	0xa5a5	// Initial value for computing checksum

#define ASCIIMSGTERMINATOR	'\n'	// Separator for ASCII/HEX msgs

#define PCTOGATEWAYSIZE	48	// (Note keep align 4 to allow casts to ints)

/* Compressed msg  */
struct PCTOGATECOMPRESSED
{
	u8 	cm[PCTOGATEWAYSIZE/2];	// seq + id + dlc + payload bytes + jic spares
	u8*	p;			// Pointer into cm[]
	s16	ct;			// Byte count of compressed result (not including checksum)
	u8	seq;			// Message count
	u8	chk;			// Checksum
};

struct PCTOGATEWAY	// Used in PC<->gateway asc-binary conversion & checking
{
	char asc[PCTOGATEWAYSIZE];	// ASCII "line" is built here
//	u8	*p;			// Ptr into buffer
	char	*pasc;			// Ptr into buffer
	u32	chk;			// Checksum
	s16	ct;			// Byte ct (0 = msg not complete; + = byte ct)
	s16	ctasc;			// ASC char ct
	u8	prev;			// Used for binary byte stuffing
	u8	seq;			// Sequence number (CAN msg counter)
	u8	mode_link;		// PC<->gateway mode (binary, ascii, ...)
	struct PCTOGATECOMPRESSED cmprs; // Easy way to make call to 'send'
};

/* ------------ PC<->gateway link MODE selection -------------------------------------- */
#define MODE_LINK	2	// PC<->gateway mode: 0 = binary, 1 = ascii, 2 = ascii-gonzaga

/* Error counts for monitoring. */
struct CANWINCHPODCOMMONERRORS
{
	u32 can_txerr; 		// Count: total number of msgs returning a TERR flags (including retries)
	u32 can_tx_bombed;	// Count: number of times msgs failed due to too many TXERR
	u32 can_tx_alst0_err; 	// Count: arbitration failure total
	u32 can_tx_alst0_nart_err;// Count: arbitration failure when NART is on
	u32 can_msgovrflow;	// Count: Buffer overflow when adding a msg
	u32 can_spurious_int;	// Count: TSR had no RQCPx bits on (spurious interrupt)
	u32 can_no_flagged;	// Count: 
	u32 can_pfor_bk_one;	// Count: Instances that pfor was adjusted in TX interrupt
	u32 can_pxprv_fwd_one;	// Count: Instances that pxprv was adjusted in 'for' loop
	u32 can_rx0err;		// Count: FIFO 0 overrun
	u32 can_rx1err;		// Count: FIFO 1 overrun
	u32 can_cp1cp2;		// Count: (RQCP1 | RQCP2) unexpectedly ON
	u32 error_fifo1ctr;	// Count: 'systickphasing' unexpected 'default' in switch
	u32 nosyncmsgctr;	// Count: 'systickphasing',lost sync msgs 
	u32 txint_emptylist;	// Count: TX interrupt with pending list empty
	u32 disable_ints_ct;	// Count: while in disable ints looped
};

struct PARAMIDPTR {
	uint16_t id;
	void*	ptr;
};

#endif 

