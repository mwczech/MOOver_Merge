/*
 * File:   pmb_functions.c
 * Author: Jaca
 *
 * Created on January 31, 2023, 10:46 PM
 */

#include "xc.h"
#include "main.h"
#include "pmb_CAN.h"
#include "pmb_Display.h"
#include "pmb_Functions.h"
#include "pmb_MotorManager.h"
#include "pmb_System.h"
#include "mcc_generated_files/pin_manager.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define FIXED_POINT_BITS 8
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)

float CalculateDegreeFromPi(int32_t Degree){
    float Angle;
    float Angle2;
    int32_t ScaledInput;
    
    ScaledInput = Degree * FIXED_POINT_SCALE / 3141;
    Angle = ScaledInput * 180 / FIXED_POINT_SCALE;
    Angle2 = (float)Degree / 3141.0f * 180.0f;
    /* Normalize angle (change position 0 angle, from -180/180 to -1/1 */
     if(Angle2 < 0){
         /*TODO: pawelton remove adding 1000 (sign indication) */
         Angle2 = Angle2 + 180.0f;
         return -Angle2;

     }
     else{
        Angle2 = 180.0f - Angle2;
        return Angle2;
     }
}


void PMB_Initialize(void)
{
    System_Init();
    //DisableAllPowerStages(); // Disable Power Stages
    MotorManager_Initialise();
}

char DecToHex(int decNum)
{
    // char array to store hexadecimal number
    char hexaDeciNum[50];
    // counter for hexadecimal number array
    int i = 0;
    while (decNum != 0)
    {
        /* temporary variable to
        store right most digit*/
        int temp = 0;
        // Get the right most digit
        temp = decNum % 16;
        // check if temp < 10
        if (temp < 10)
        {
            hexaDeciNum[i] = temp + 48;
            i++;
        }
        else
        {
            hexaDeciNum[i] = temp + 55;
            i++;
        }
        decNum = decNum / 16; // get the quotient
    }
    printf("0x"); //print hex symbol
    // printing hexadecimal number array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        return hexaDeciNum[j];
    }
    return 0;
}

uint8_t HexCharToInt(uint8_t HexChar)
{
 if ((HexChar >= '0') && (HexChar <= '9'))
  HexChar -= 0x30;
 else
 {
  if ((HexChar >= 'A') && (HexChar <= 'F'))
   HexChar -= 0x37;
  else
   HexChar = 0; //jesli przeslana dana nie jest z zakresu cyfr i liter hexadecymalnych
 }
 return HexChar;
}

uint8_t HexIntToChar(uint8_t HexInt)
{
 HexInt &= 0x0F;
 if ((HexInt >= 0) && (HexInt <= 9))
  HexInt += 0x30;
 else
  HexInt += 0x37;
 return HexInt;
}

uint8_t NumberOfDigits(uint16_t Int)
{
    uint8_t count = 0;
    while(Int != 0)
    {
        Int /=10;
        count++;
    }
    return count;
}

/* TO REMOVE
//ANALOG CNT TO VOLTAGE CALCULATION
uint32_t CntToVoltage (uint16_t *ADCcnt)
{
    uint32_t Voltage;
    uint16_t cnt;
    
    cnt = *ADCcnt;
    
    Voltage = cnt / 10;
    Voltage = Voltage * 806 / 100;
       
    //resulting a value in mV
    return Voltage;
}

uint32_t CntToRealVoltage (uint16_t *ADCcnt)
{
    uint32_t Voltage;
    uint16_t cnt;
    
    cnt = *ADCcnt;
    Voltage = cnt / 10;
    Voltage = Voltage * 806 / 100;
    //RealVoltage = 
    
    //resulting a value in mV
    return Voltage;
}
 */



