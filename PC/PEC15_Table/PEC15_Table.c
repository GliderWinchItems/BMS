

/************************************
Copyright 2012 Analog Devices, Inc. (ADI)
Permission to freely use, copy, modify, and distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies: THIS SOFTWARE IS PROVIDED “AS IS” AND ADI
DISCLAIMS ALL WARRANTIES
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL ADI BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
OR ANY DAMAGES WHATSOEVER RESULTING FROM ANY
USE OF SAME, INCLUDING ANY LOSS OF USE OR DATA
OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLI-
GENCE OR OTHER TORTUOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
***************************************/
#include <stdint.h>
#include <stdio.h>

/*
gcc PEC15_Table.c -o PEC15_Table && ./PEC15_Table
*/

uint16_t pec15Table[256];
uint32_t CRC15_POLY = 0x4599;
void init_PEC15_Table(void)
{
	uint16_t remainder;
	for (int i = 0; i < 256; i++)
	{
		remainder = i << 7;
		for (int bit = 8; bit > 0; --bit)
		{
			if (remainder & 0x4000)
			{
				remainder = ((remainder << 1));
				remainder = (remainder^CRC15_POLY);
			}
			else
			{
				remainder = ((remainder << 1));
			}
		}
		pec15Table[i] = remainder&0xFFFF;
	}
}
int main(void)
{
	init_PEC15_Table();

		printf("\n");
		for (int k = 0; k < 16; k++)
			printf("%7d",k);
		printf("\r//");
	printf("\nconst int16_t pec15Table[256] = {\n ");

	for (int i = 0; i < 16; i++)
	{	
		for (int j = 0; j < 16; j++)
		{
			printf("0x%04X,",pec15Table[(i*16)+j]);
		}
		printf(" /* %2d */\n ",i);
	}
	printf(("};\n"));
}
