/******************************************************************************
* File Name          : pec15_nibble.c
* Date First Issued  : 06/11/2022
* Description        : ADBMS1818 PEC computation: four bit table lookup
*******************************************************************************/
/*
Number of cycles in 'main' for execution with size = 6: 155. 
*/

const uint16_t crctable[] = {
  0x0000,0xC599,0xCEAB,0x0B32,0xD8CF,0x1D56,0x1664,0xD3FD,0xF407,0x319E,0x3AAC,0xFF35,0x2CC8,0xE951,0xE263,0x27FA };

uint16_t pec15_nibble(uint8_t *data, int size)
{     
  uint16_t crc = 16; // PEC seed
  while(size--)
  {
    crc = crc ^ (*data++ << (15-8)); // Align upper bits
  
    crc = (crc << 4) ^ crctable[(crc >> (15-4)) & 0xF]; // Process byte 4-bits at a time
    crc = (crc << 4) ^ crctable[(crc >> (15-4)) & 0xF];
  }
     
      return (crc * 2);
}
