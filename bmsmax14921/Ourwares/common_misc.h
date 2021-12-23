/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : common_misc.h
* Hackerees          : deh
* Date First Issued  : 12/27/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : includes "common" for all sensor programs
*******************************************************************************/

#ifndef __COMMON_MISC_SENSOR
#define __COMMON_MISC_SENSOR/* This struct is used to return a pointer and count from a "get packet" routine */

#include <stdint.h>

/* Type definitions for shorter and nicer code */
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


struct PKT_PTR
{
	char*		ptr;	// Pointer to packet
	unsigned int	ct;	// Byte count of packet
};

/* Different ways of dealing with an int */
union UI_I_C
{
	u32			u32;
	s32			s32;
	unsigned long		ul;
	signed long		sl;
	unsigned int		ui;
	signed int		n;
	unsigned short		us[2];
	signed short		s[2];
	unsigned char		uc[4];
	signed char		c[4];
};

/* This is the same UNION as used in svn_pod (see common.h) */
union LL_L_S	
{
	unsigned long long	  ull;		// 64 bit version unsigned
	signed	 long long         ll;		// 64 bit version signed
	unsigned int		ui[2];		// Two 32 bit'ers unsigned
	int			 n[2];		// Two 32 bit'ers signed
	unsigned long		ul[2];		// Two 32 bit'ers unsigned
	signed   long           sl[2];		// Two 32 bit'ers signed
	unsigned short		us[4];		// Four 16 bit unsigned
	signed   short		 s[4];		// Four 16 bit signed
	unsigned char		uc[8];		// Last but not least, 8b unsigned
	signed   char		 c[8];		// And just in case, 8b signed
};
/* Use this for passing two 32b "things" */
struct TWO32
{
	union UI_I_C	a;
	union UI_I_C	b;
};

#endif 

