#include "Tools.h"
#include <string.h>
uint16_t dummy;
uint8_t Tools_ITOAu16(uint16_t Number, uint8_t *RetString)
{
    //dummy++;
    uint8_t Len = 0, Len2 = 0;
    char ResultString[5]; /* Maximum length of uint16_t value 65535 */
    uint16_t ProcessNumber;
    uint8_t Modulo;
    ProcessNumber = Number;
    for(int i=5; i>0; i--)
    {
        Len++;
        Modulo = ProcessNumber % 10;
        ResultString[i-1] =  Modulo + '0';
        ProcessNumber = ProcessNumber / 10;
        if(ProcessNumber == 0 )
        {
            Len2 = 5-i;
            break;
            
        }
    }
    
    memcpy(RetString, &ResultString[5-Len], Len);
    
    return Len;
    
}

bool isBitSet(uint32_t data, uint8_t position) {
    // Create a mask with only the bit at the specified position set
    uint32_t mask = (uint32_t)1 << position;
    
    // Check if the bit at the specified position is set in the data using bitwise AND
    if ((data & mask) != 0) {
        return true; // Bit is set
    } else {
        return false; // Bit is not set
    }
}

uint8_t reverseBits(uint8_t num) {
    num = ((num & 0xF0) >> 4) | ((num & 0x0F) << 4);
    num = ((num & 0xCC) >> 2) | ((num & 0x33) << 2);
    num = ((num & 0xAA) >> 1) | ((num & 0x55) << 1);
    return num;
}
