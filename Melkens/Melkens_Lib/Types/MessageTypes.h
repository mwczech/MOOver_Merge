/*
 * MessageTypes.h
 *
 *  Created on: May 20, 2025
 *      Author: piomod
 */
#include <stdint.h>
#include <stdbool.h>

#ifndef MESSAGETYPES_H
#define MESSAGETYPES_H

#define PROTOCOL_VERSION 001

#pragma pack(push,1)
//---------------------------------------------------------
// IMU ---> PMB
typedef struct {
  int16_t motorRightSpeed;
  int16_t motorLeftSpeed;
  uint16_t motorThumbleSpeed;
  uint16_t motorLiftSpeed;
  uint16_t motorBelt1Speed;
  uint16_t motorBelt2Speed;
  ////////
  uint16_t crc;
} Imu2PmbFrame_t;

// IMU <--- PMB
typedef struct {
  uint32_t motorRightRotation;
  uint32_t motorLeftRotation;
  uint16_t batteryVoltage;
  uint16_t adcCurrent;
  uint16_t thumbleCurrent;
  uint16_t crcImu2PmbErrorCount;
  ////////
  uint16_t crc;
} Pmb2ImuFrame_t;

//---------------------------------------------------------
// IMU ---> ESP

typedef struct {
  uint32_t magnetBarStatus;
  uint16_t pmbConnection;
  uint16_t motorRightSpeed;
  uint16_t motorLeftSpeed;
  uint16_t batteryVoltage;
  uint16_t adcCurrent;
  uint16_t thumbleCurrent;
  uint16_t crcImu2PmbErrorCount;
  uint16_t crcPmb2ImuErrorCount;
  uint16_t crcEsp2ImuErrorCount;
  ////////
  uint16_t crc;
} Imu2EspFrame_t;

// IMU <--- ESP
typedef struct {
  int8_t moveX;
  int8_t moveY;
  uint16_t augerSpeed; //0-1500
  uint8_t rootNumber; //a=0, b=1, c=2, ...
  uint8_t rootAction; //stop=0, play=1, pause=2
  uint8_t power; //off=0, on=1
  uint8_t charging; //off=0, on=1
  ////////
  uint16_t crc;
} Esp2ImuFrame_t;

//---------------------------------------------------------

//---------------------------------------------------------
// IMU ---> PC Debug communication
typedef struct {
  uint16_t motorRightSpeed;
  uint16_t motorLeftSpeed;
  uint16_t Xpos1;
  uint16_t Ypos1;
  uint16_t Xpos2;
  uint16_t Ypos2;
  uint16_t angle;
  uint16_t motorBelt2Speed;
  ////////
  uint16_t crc;
} Imu2PCFrame_t;
//---------------------------------------------------------
#pragma pack(pop)

#endif /* MESSAGETYPES_H */
