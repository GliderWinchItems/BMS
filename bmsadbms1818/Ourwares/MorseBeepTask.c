/******************************************************************************
* File Name          : MorseBeepTask.c
* Date First Issued  : 11/12/2019
* Description        : Morse code via beeper
*******************************************************************************/
#include <stdint.h>
#include "DTW_counter.h"
#include "stm32f4xx_hal.h"
#include "MorseBeepTask.h"
#include "SerialTaskSend.h"

#define TICPERSEC (186000000)
#define TIC_DIT (TICPERSEC/6)
#define TIC_DAH (TIC_DIT*4)
#define TIC_IDIT (TIC_DIT*1.3)
#define TIC_ICHAR (TIC_IDIT*4)
#define TIC_IWORD (TIC_ICHAR *3)
#define TIC_PAUSE (TICPERSEC*1) // Pause between code sequence

osThreadId MorseBeepTaskHandle = NULL;
osMessageQId MorseBeepTaskQHandle;

/* Queue */
#define QUEUESIZE 16	// Total size of bcb's tasks can queue up

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

const struct BEEPQ qdit = {
TIC_DIT,
TIC_IDIT,
1};
const struct BEEPQ qdah = {
TIC_DAH,
TIC_IDIT,
1};
const struct BEEPQ qchar = {
0,
(TIC_ICHAR-TIC_DIT),
1};
const struct BEEPQ qword = {
0,
(TIC_IWORD-TIC_ICHAR),
1};


/* *************************************************************************
 * static void delay(uint32_t ticks, uint8_t on);
 * @brief	: Delay based on DTW counter with LEDs ON
 * @param	: ticks = DTW count to delay
 * @param	: on = GPIO_PIN_SET or GPIO_PIN_RESET
 * *************************************************************************/
static void delay(uint32_t ticks, uint8_t on)
{
	uint32_t tx = DTWTIME + ticks;
	while ((int32_t)(tx - DTWTIME) > 0)
	{
			HAL_GPIO_WritePin(GPIOD, LEDALL, on); 
	}
	return;
}
/* *************************************************************************
 * static void queuedot(char c);
 *	@brief	: c = character to send as Morse cdoe
 * *************************************************************************/

/* *************************************************************************
 * static void morsebeepgenerate(char c);
 *	@brief	: c = character to send as Morse cdoe
 * *************************************************************************/
static void morsebeepgenerate(char c)
{
	const struct MORSE_ELEMENT* ptbl = mrse;
	int i;
	uint8_t ct;
	uint8_t dd;
	
	for (i = 0; i < (82 - 34); i++)
	{ // Dumb lookup of char in table
		if (ptbl->c == c)
		{ // Here, we found the char.
				ct = ptbl->ct;
				dd = ptbl->dd;	
				while (ct > 0) // Queue Beeper dots and dashes
				{
					if ((dd & 0x80) == 0)
					{ // Here, dot
						xQueueSendToBack(BeepTaskQHandle, &qdit, portMAX_DELAY);
					}
					else
					{ // Here, dash
						xQueueSendToBack(BeepTaskQHandle, &qdah, portMAX_DELAY);
					}
					dd = dd << 1;
					ct -= 1;
				}
				// Off time between characters
				xQueueSendToBack(BeepTaskQHandle, &qchar, portMAX_DELAY);
				return;
		}
		ptbl++;
	}	
	return;
}
/* *************************************************************************
 * void morsebeepstring(char* p);
 *	@brief	: Send a character string as Morse code
 * @param	: p = pointer to string
 * *************************************************************************/
void morsebeepstring(char* p)
{
	char prevspace = 0;
	char prevchar = 0;

	while(*p != 0)
	{
		if (*p != ' ')
		{ // Not an interword space
			if (((prevchar >= '0') || (prevchar <= '9')) &&
				(*p == '.'))
			{ // Substitute 'R' for '.' when preceded by number
				*p = 'R';
			}
			if ((*p >= 'a') && (*p <= 'z'))
			{
				*p = *p & ~(0x20);
			}
			morsebeepgenerate(*p);
			prevchar = *p;
		}
		else
		{ // Here, space between words
			// Off time between characters
			xQueueSendToBack(BeepTaskQHandle, &qword, portMAX_DELAY);
		}
		p++;
	}
	return;		
}
/* *************************************************************************
 * void morsebeepnumber(uint32_t n);
 *	@brief	: Send a fixed pt number as Morse code
 * @param	: nx = number to send
 * *************************************************************************/
void morsebeepnumber(uint32_t nx)
{
	char c[11];
	int i = 0;
	do
	{
		c[i] = (nx % 10) + '0';
		nx = nx/10;
		i += 1;
	} while (nx != 0);

	while (--i >= 0)
	{
		morsebeepgenerate(c[i]);

	}
	delay(TIC_IWORD,GPIO_PIN_RESET);
	return;
}
/* *************************************************************************
 * void morsebeephex(uint32_t n);
 *	@brief	: Send a  hex number, skip leading zeroes
 * @param	: nx = number to send
 * *************************************************************************/
static const char h[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void morsebeephex(uint32_t nx)
{
	uint8_t c;
	uint8_t sw = 0;
	int8_t i;

	morsebeepgenerate('X');	// Hex prefix (but skip '0' in front of X)

	uint32_t mask = 0xf0000000;

	if (nx == 0)
	{
		morsebeepgenerate('0');
		return;
	}
	
	for (i = 0; i < 8; i++)
	{
		if (((nx & mask) != 0) || (sw != 0))
		{
			sw = 1;
			c = h[(nx >> 28)];
			morsebeepgenerate(c);
		}
		nx = nx << 4;
	}  
	delay(TIC_IWORD,GPIO_PIN_RESET);
	return;	
}
/* *************************************************************************
 * void StartMorseBeepTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartMorseBeepTask(void* argument)
{
	BaseType_t Qret;	// queue receive return
	struct SERIALSENDTASKBCB*  pssb; // Receives ptr to buff control block

  /* Infinite loop */
  for(;;)
  {
		do
		{
		/* Wait indefinitely for someone to load something into the queue */
		/* Skip over empty returns, and NULL pointers that would cause trouble */
			Qret = xQueueReceive(SerialTaskSendQHandle,&pssb,portMAX_DELAY);
			if (Qret == pdPASS) // Break loop if not empty
				break;
		} while ((pssb->tskhandle == NULL));

	 	if (!((pssb->pbuf == NULL) || (pssb->size == 0)))
		{ // Here it looks OK.
			morsebeepstring(pbcb->pbuf);
		}
  			/* Release buffer just sent so it can be reused. */
		xSemaphoreGive(pssb->semaphore);
	}
}
/* *************************************************************************
 * osThreadId xMorseBeepTaskCreate(uint32_t taskpriority, uint32_t beepqsize);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: MorseBeepTaskHandle
 * *************************************************************************/
osThreadId xMorseBeepTaskCreate(uint32_t taskpriority, uint32_t beepqsize)
{
	BaseType_t ret = xTaskCreate(&StartMorseBeepTask, "MorseBeepTask",\
     128, NULL, taskpriority, &MorseBeepTaskHandle);
	if (ret != pdPASS) return NULL;

	MorseBeepTaskQHandle = xQueueCreate(QUEUESIZE, sizeof(struct SERIALSENDTASKBCB*) );
	if (MorseBeepTaskQHandle == NULL) return NULL;

	return MorseBeepTaskHandle;
}

