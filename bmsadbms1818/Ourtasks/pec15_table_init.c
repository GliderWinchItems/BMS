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
int16 pec15Table[256];
int16 CRC15_POLY = 0x4599;
void init_PEC15_Table()
{
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
