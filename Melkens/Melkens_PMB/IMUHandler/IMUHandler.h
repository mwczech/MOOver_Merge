/* 
 * File:   IMUHandler.h
 * Author: pawelton
 *
 * Created on May 14, 2023, 6:08 PM
 */

#ifndef IMUHandler_H
#define	IMUHandler_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define LEFT_ENCODER 0
#define RIGHT_ENCODER 1
#define THUMBLE_ENCODER 2
    
typedef enum MagnetName_t{
  Magnet1 = 0, //First sensor, position -15
  Magnet2,
  Magnet3,
  Magnet4,
  Magnet5,
  Magnet6,
  Magnet7,
  Magnet8,
  Magnet9,
  Magnet10,
  Magnet11,
  Magnet12,
  Magnet13,
  Magnet14,
  Magnet15,
  Magnet16, //Zero position
  Magnet17,
  Magnet18,
  Magnet19,
  Magnet20,
  Magnet21,
  Magnet22,
  Magnet23,
  Magnet24,
  Magnet25,
  Magnet26,
  Magnet27,
  Magnet28,
  Magnet29,
  Magnet30,
  Magnet31, //Last sensor, position 15       
  Magnet_NumOf
}MagnetName;  

typedef enum ESP_Message_t
{
	ESP_Forward = 0x4F46,    //FO
	ESP_Right = 0x4952,      //RI
	ESP_Left = 0x454C,       //LE
	ESP_Reverse = 0x4552,    //RE
	ESP_Stop = 0x5453,       //ST
	ESP_AugerStop = 0x3041,  //A0
	ESP_AugerStart = 0x3141, //A1
	ESP_RouteA = 0x4154,     //TA
	ESP_RouteB = 0x4254, 	   //TB
	ESP_RouteC = 0x4354, 	   //TC
	ESP_RouteD = 0x4454, 	   //TD
	ESP_RouteE = 0x4554,     //TE
	ESP_RouteF = 0x4654, 	   //TF
	ESP_RouteG = 0x4754, 	   //TG
	ESP_RouteH = 0x4854, 	   //TH
	ESP_RouteI = 0x4954,     //TI
	ESP_RouteJ = 0x4A54, 	   //TJ
	ESP_RouteK = 0x4B54, 	   //TK
	ESP_RoutePlay = 0x4C50,  //PL
	ESP_RoutePause = 0x4150, //PA            
	ESP_PowerON = 0x3150,  	 //P1
	ESP_PowerOFF = 0x3050,	 //P0
  ESP_ChargeON = 0x4857,   //WH
	ESP_ChargeOFF = 0x4C57,	 //WL
	ESP_SafetyON = 0x3153, 	 //S1
	ESP_SafetyOFF = 0x3053,	 //S0
	ESP_Program = 0x5250,	 //PR
	ESP_NoAction = 0x3030,   //00
	ESP_LineFeed = 0x0A0D	 //\r\n
    

}ESP_Message;

typedef enum RemoteButton_t{
    Remote_UP = 0,
    Remote_DOWN,
    Remote_LEFT,
    Remote_RIGHT,
    Remote_RoutePlay,
    Remote_RoutePause,        
    Remote_Stop,
    Remote_ThumbleStop,
    Remote_ThumbleStart,
    Remote_Speed,
    Remote_RouteStep,
    Remote_PowerON,
    Remote_PowerOFF,
    Remote_SafetyON,
    Remote_SafetyOFF,
    Remote_ChargeON,
    Remote_ChargeOFF,         
    Remote_Lift_UP,
    Remote_Lift_DOWN,
    Remote_Belt1_ON,
    Remote_Belt2_ON,
    Remote_RouteA, /* Important! Route enums should be ALWAYS before Remote_Released enum */
    Remote_RouteB,
    Remote_RouteC,
    Remote_RouteD,
    Remote_RouteE,
    Remote_RouteF,
    Remote_RouteG,
    Remote_RouteH,
    Remote_RouteI,
    Remote_RouteJ,
    Remote_RouteK,
    Remote_Released
}RemoteButton;

typedef enum MagnetPosition_t{
    Magnet1st = 0,
    Magnet2nd,
    Magnet3rd
}MagnetPosition;

typedef union
{
  struct
  {
   uint32_t  bit0 : 1;
   uint32_t  bit1 : 1;
   uint32_t  bit2 : 1;
   uint32_t  bit3 : 1;
   uint32_t  bit4 : 1;
   uint32_t  bit5 : 1;
   uint32_t  bit6 : 1;
   uint32_t  bit7 : 1;
   uint32_t  bit8 : 1;
   uint32_t  bit9 : 1;
   uint32_t  bit10 : 1;
   uint32_t  bit11 : 1;
   uint32_t  bit12 : 1;
   uint32_t  bit13 : 1;   
   uint32_t  bit14 : 1;
   uint32_t  bit15 : 1;
   uint32_t  bit16 : 1;
   uint32_t  bit17 : 1;
   uint32_t  bit18 : 1;
   uint32_t  bit19 : 1;
   uint32_t  bit20 : 1;
   uint32_t  bit21 : 1;
   uint32_t  bit22 : 1;
   uint32_t  bit23 : 1;
   uint32_t  bit24 : 1;
   uint32_t  bit25 : 1;
   uint32_t  bit26 : 1;
   uint32_t  bit27 : 1;
   uint32_t  bit28 : 1;
   uint32_t  bit29 : 1;
   uint32_t  bit30 : 1;
   uint32_t  bit31 : 1;
  }u;
  uint32_t status;
}MagnetsStatus;

void IMUHandler_Perform100ms( void );
    
void IMUHandler_ReadEncoderValues(uint16_t RightEncoder, uint16_t LeftEncoder );
int16_t IMUHandler_GetEncoderValue(uint8_t Wheel);
void IMUHandler_Perform1ms( void );
void IMUHandler_Perform100ms(void);
void IMUHandler_SetMessageReceived(void);
void IMUHandler_Init( void );
bool IMUHandler_IsInitialized( void );
int16_t IMUHandler_Get1msRotationTics(bool Wheel);
uint16_t IMUHandler_GetReceiveBufferAddress( void );
uint16_t IMUHandler_GetTransmitBufferAddress( void );
void IMUHandler_ProcessReceivedData(void);
int16_t GetRoll(void);

int16_t GetPitch(void);

int16_t GetYaw(void);

int16_t GetAhrsX(void);

int16_t GetAhrsY(void);

MagnetsStatus GetMagnets(void);
void Remote_ClearEvent(void);
float IMUHandler_GetAngle(void);
float IMUHandler_CalculateAngle(float prevDegree, float currentDegree);
RemoteButton IMUHandler_GetRemoteMessage();
uint8_t Remote_GetSpeed();
uint8_t Remote_GetRouteStep();
float IMUHandler_GetMagnetMagnetPositionInCM(MagnetPosition Magnet);
uint8_t IMUHandler_HowManyMagnetsDetected(void);
void IMUHandler_SetCurrentRouteStep(uint8_t Step);
bool IMUHandler_IsRouteSelectButton(void);

void IMUHandler_SetThumbleCurrent(int16_t Current );
void IMUHandler_SetOverallCurrent(int16_t Current );


#ifdef	__cplusplus
}
#endif

#endif	/* IMUHandler_H */

