/******************************************************************************
* File Name          : morse.c
* Date First Issued  : 02/13/2019
* Description        : Morse code
*******************************************************************************/
#include <stdint.h>
#include "DTW_counter.h"
#include "stm32l4xx_hal.h"
#include "main.h"

#define TICPERSEC (16000000)
#define TIC_DIT (TICPERSEC/6)
#define TIC_DAH (TIC_DIT*4)
#define TIC_IDIT (TIC_DIT*1.3)
#define TIC_ICHAR (TIC_IDIT*4)
#define TIC_IWORD (TIC_ICHAR *3)
#define TIC_PAUSE (TICPERSEC*1) // Pause between code sequence

#define LEDALL (GPIO_PIN_0|GPIO_PIN_1)

struct MORSE_ELEMENT
{
	char c;
	uint8_t dd;
	uint8_t ct;
};

const struct MORSE_ELEMENT mrse[] = {
{'A', 0b01000000, 2}, 
{'B', 0b10000000, 4}, 
{'C', 0b10100000, 4}, 
{'D', 0b10000000, 3}, 
{'E', 0b00000000, 1}, 
{'F', 0b00100000, 4}, 
{'G', 0b11000000, 3}, 
{'H', 0b00000000, 4},
{'I', 0b00000000, 2}, 
{'J', 0b01110000, 4}, 
{'K', 0b10100000, 3}, 
{'L', 0b01000000, 4}, 
{'M', 0b11000000, 2}, 
{'N', 0b10000000, 2}, 
{'O', 0b11100000, 3}, 
{'P', 0b01100000, 4}, 
{'Q', 0b11010000, 4}, 
{'R', 0b01000000, 3}, 
{'S', 0b00000000, 3}, 
{'T', 0b10000000, 1}, 
{'U', 0b00100000, 3}, 
{'V', 0b00010000, 4}, 
{'W', 0b01100000, 3}, 
{'X', 0b10010000, 4}, 
{'Y', 0b10110000, 4}, 
{'Z', 0b11000000, 4}, 
{'0', 0b11111000, 5}, 
{'1', 0b01111000, 5}, 
{'2', 0b00111000, 5}, 
{'3', 0b00011000, 5}, 
{'4', 0b00001000, 5}, 
{'5', 0b00000000, 5}, 
{'6', 0b10000000, 5}, 
{'7', 0b11000000, 5}, 
{'8', 0b11100000, 5}, 
{'9', 0b11110000, 5}, 
{'.', 0b01010100, 6}, 
{',', 0b11001100, 6}, 
{'?', 0b00110000, 6}, 
{'-', 0b10001000, 5}, 
{'@', 0b01101000, 6}, 
{'_', 0b00110100, 6}, 
{'+', 0b01010000, 5}, 
{';', 0b10101000, 6}, 
{'&', 0b01000000, 5},
{'/', 0b10010000, 5},
{'(', 0b10110000, 5},
{')', 0b10110100, 6},
};

/* *************************************************************************
 * static void delay(uint32_t ticks, uint32_t pin, uint8_t on);
 * @brief	: Delay based on DTW counter with LEDs ON
 * @param	: ticks = DTW count to delay
 * @param	: pin = pin number, e.g. GPIO_PIN_0
 * @param	: on = GPIO_PIN_RESET or GPIO_PIN_SET
 * *************************************************************************/
static void delay(uint32_t ticks, uint32_t pin, uint8_t on)
{
	uint32_t tx = DTWTIME + ticks;
	while ((int32_t)(tx - DTWTIME) > 0)
	{
			HAL_GPIO_WritePin(GPIOB, pin, on); 
	}
	return;
}

/* *************************************************************************
 * static void morse_generate(char c, uint32_t pin);
 *	@brief	: c = character to send as Morse cdoe
 * @param	: pin = pin number, e.g. GPIO_PIN_0
 * *************************************************************************/
static void morse_generate(char c, uint32_t pin)
{
	const struct MORSE_ELEMENT* ptbl = mrse;
	int i;
	uint8_t ct;
	uint8_t dd;
	
	for (i = 0; i < (74 - 16); i++)
	{
		if (ptbl->c == c)
		{
				ct = ptbl->ct;
				dd = ptbl->dd;	
				while (ct > 0)
				{
					if ((dd & 0x80) == 0)
					{ // Here, dot
						delay(TIC_DIT, pin, GPIO_PIN_RESET);
					}
					else
					{ // Here, dash
						delay(TIC_DAH, pin, GPIO_PIN_RESET);						
					}
					delay(TIC_IDIT, pin, GPIO_PIN_SET);
					dd = dd << 1;
					ct -= 1;
				}
				delay(TIC_ICHAR, pin, GPIO_PIN_SET);
				return;
		}
		ptbl++;
	}	
	return;
}
/* *************************************************************************
 * void morse_string(char* p, uint32_t pin);
 *	@brief	: Send a character string as Morse code
 * @param	: p = pointer to string
 * @param	: pin = pin number, e.g. GPIO_PIN_0
 * *************************************************************************/
void morse_string(char* p, uint32_t pin)
{
	while(*p != 0)
	{
		if (*p != ' ')
		{
			morse_generate(*p++, pin);
		}
		else
		{
			delay(TIC_IWORD, pin,GPIO_PIN_SET);
			p++;
		}
	}

	return;		
}
/* *************************************************************************
 * void morse_number(uint32_t nx, uint32_t pin);
 *	@brief	: Send a character string as Morse code
 * @param	: nx = number to send
 * @param	: pin = pin number, e.g. GPIO_PIN_0
 * *************************************************************************/
void morse_number(uint32_t nx, uint32_t pin)
{
	char c[16];
	int i = 0;
	do
	{
		c[i] = (nx % 10) + '0';
		nx = nx/10;
		i += 1;
	} while (nx != 0);

	while (--i >= 0)
	{
		morse_generate(c[i], pin);

	}
	delay(TIC_IWORD, pin, GPIO_PIN_SET);
	return;
}
/* *************************************************************************
 * void morse_trap(uint32_t x);
 *	@brief	: Disable interrupts, Send 'x' and endless loop
 * @param	: x = trap number to flash
 * *************************************************************************/
void morse_trap(uint32_t x)
{
	/* Disable global interrupts */
__asm__ volatile ("CPSID I");
	while(1==1)
	{
		morse_number(x, (GPIO_PIN_0|GPIO_PIN_1));
		delay(TIC_PAUSE, (GPIO_PIN_0|GPIO_PIN_1), GPIO_PIN_SET);	
	}
}
/* *************************************************************************
 * void morse_hex(uint32_t n, uint32_t pin);
 *	@brief	: Send a  hex number, skip leading zeroes
 * @param	: nx = number to send
 * @param	: pin = pin number, e.g. GPIO_PIN_0
 * *************************************************************************/
static const char h[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void morse_hex(uint32_t nx, uint32_t pin)
{
	uint8_t c;
	uint8_t sw = 0;
	int8_t i;

	morse_generate('X', pin);	// Hex prefix (but skip '0' in front of X)

	uint32_t mask = 0xf0000000;

	if (nx == 0)
	{
		morse_generate('0', pin);
		return;
	}
	
	for (i = 0; i < 8; i++)
	{
		if (((nx & mask) != 0) || (sw != 0))
		{
			sw = 1;
			c = h[(nx >> 28)];
			morse_generate(c, pin);
		}
		nx = nx << 4;
	}  
	delay(TIC_IWORD, pin, GPIO_PIN_SET);
	return;	
}

