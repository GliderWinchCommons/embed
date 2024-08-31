/******************************************************************************
* File Name          : crc-32_nib.c
* Date First Issued  : 08-29-2022
* Board              : 
* Description        : CRC-32 computation with no nibble table lookup
*******************************************************************************/
/* See--
https://community.st.com/s/global-search/CRC-32
*/
#include "crc-32_nib.h" 

// Nibble lookup table for 0x04C11DB7 polynomial
static const  uint32_t CrcTable[16] = { 
0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };
 
uint32_t crc_32_nib_acc( uint32_t Crc,  uint32_t Data)
 {
     Crc = Crc ^ Data; // Apply all 32-bits
     
     // Process 32-bits, 4 at a time, or 8 rounds
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // 0x04C11DB7 Polynomial used in STM32
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     
     return(Crc);
 }
 /******************************************************************************
 * uint32_t crc_32_nib_calc(uint32_t* pdata, uint32_t len);
 * @brief   : Compute CRC-32
 * @param   : pdata = pointer to input array (words)
 * @param   : len = number of words
 * @param   : CRC-32
*******************************************************************************/
uint32_t crc_32_nib_calc(uint32_t* pdata, uint32_t len)
{
    uint32_t crc = ~0L;
    do
    {
        crc = crc_32_nib_acc(crc, *pdata++);
        len -= 1;
    } while (len > 0);
    return crc;
}
 