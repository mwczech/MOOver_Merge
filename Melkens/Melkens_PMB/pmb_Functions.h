/* 
 * File:   pmb_Functions.h
 * Author: ciaptak
 *
 * Created on 15 pa?dziernika 2023, 16:33
 */

#ifndef PMB_FUNCTIONS_H
#define	PMB_FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

void EnableCharger(void);
void EnableCharger(void);
void DisableCharger(void);
void ResetCharger(void);
char DecToHex(int decNum);
uint8_t HexCharToInt(uint8_t HexChar);
float CalculateDegreeFromPi(int32_t Degree);
uint8_t HexIntToChar(uint8_t HexInt);
uint8_t NumberOfDigits(uint16_t Int);

#ifdef	__cplusplus
}
#endif

#endif	/* PMB_FUNCTIONS_H */

