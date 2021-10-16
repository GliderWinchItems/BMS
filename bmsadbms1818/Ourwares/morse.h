/******************************************************************************
* File Name          : morse.h
* Date First Issued  : 02/13/2019, 09/05/2021
* Description        : Morse code
*******************************************************************************/

#ifndef __MORSE
#define __MORSE

/* ************************************************************************* */
 void morse_string(char* p, uint32_t pin);
/*  @brief	: Send a character string as Morse code
 * @param	: p = pointer to string
 * @param	: pin = port pin number(s), e.g. GPIO_PIN_0
 * *************************************************************************/
  void morse_number(uint32_t nx, uint32_t pin);
 /*	@brief	: Send a character string as Morse code
 * @param	: nx = number to send
 * @param	: pin = port pin number(s), e.g. GPIO_PIN_0
 * *************************************************************************/
void morse_trap(uint32_t x);
/*	@brief	: Disable interrupts, Send 'x' and endless loop
 * @param	: x = trap number to flash
 * *************************************************************************/
 void morse_hex(uint32_t n, uint32_t pin);
 /*	@brief	: Send a  hex number, skip leading zeroes
 * @param	: nx = number to send
 * @param	: pin = pin number, e.g. GPIO_PIN_0
 * *************************************************************************/

#endif

