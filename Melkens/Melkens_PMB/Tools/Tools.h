/* 
 * File:   Tools.h
 * Author: pawelton
 *
 * Created on June 18, 2023, 4:04 PM
 */

#ifndef TOOLS_H
#define	TOOLS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    
uint8_t Tools_ITOAu16(uint16_t Number, uint8_t *RetString);
bool isBitSet(uint32_t data, uint8_t position);
uint8_t reverseBits(uint8_t num);

#ifdef	__cplusplus
}
#endif

#endif	/* TOOLS_H */

