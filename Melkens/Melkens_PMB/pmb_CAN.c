/*
 * File:   pmb_motors.c
 * Author: Jaca
 *
 * Created on 16 lutego 2023, 00:37
 */


#include "xc.h"
#include "main.h"
#include "pmb_CAN.h"
#include "pmb_Functions.h"
#include "mcc_generated_files/pin_manager.h"
#include "pmb_MotorManager.h"
#include"IMUHandler/IMUHandler.h"
#include "DiagnosticsHandler.h"
#include <string.h>
#include <stdio.h>

/*
enum MotorParameters{
    OpType,
    Angle,
    Distance,
    Speed,
    EncoderSpan,
    LDir, 
    RDir
};
 */



typedef struct System_Time_RTC_t
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
}SystemTime_RTC;
SystemTime_RTC Route1, Route2, Route3, Route4;

CAN_MSG_OBJ CAN_Rx;

uint32_t calc_speed, calc_speed_right;
uint16_t RightMotor_PositionCount, LeftMotor_PositionCount, ThumbleMotor_PositionCount;
int16_t RightMotor_Current, LeftMotor_Current, ThumbleMotor_Current;

//CAN messages declaration

uint8_t CAN_Go_With_Speed[8] = {0x23,0x02,0x20,0x01,0x64,0x00,0x00,0x00};
uint8_t CAN_Go_Speed[8] = {0x23,0x00,0x20,0x01,0x00,0x00,0x00,0xBA};

void CAN_Polling(void)
{
    
    if(CAN1_ReceivedMessageCountGet() > 0) { 
        if(true == CAN1_Receive(&CAN_Rx)){

            /* Right wheel heartbeat */
            if(CAN_Rx.msgId==0x0700007E){
                Diagnostics_SetEvent(dDebug_RightInverterConnected);
            }

            /* Left wheel heartbeat */
            if(CAN_Rx.msgId==0x0700007F){
                Diagnostics_SetEvent(dDebug_LeftInverterConnected);
            }

            if(CAN_Rx.msgId==0x0580007E){
                if(CAN_Rx.data[1]==0x04&&CAN_Rx.data[2]==0x21){
                   RightMotor_PositionCount = (CAN_Rx.data[6] << 8) | CAN_Rx.data[7];
                   MotorManager_SetPositionCount(Motor_Right, RightMotor_PositionCount);
                   CalculateShaftTurn(Motor_Right);
                }
                else if(CAN_Rx.data[1]==0x00&&CAN_Rx.data[2]==0x21){
                    RightMotor_Current = (CAN_Rx.data[6] << 8) | CAN_Rx.data[7];
                    MotorManager_SetCurrent(Motor_Right, RightMotor_Current);
                }
            }
            /* Left motor encoder value */
            if(CAN_Rx.msgId==0x0580007F){
                if(CAN_Rx.data[1]==0x04&&CAN_Rx.data[2]==0x21){
                   LeftMotor_PositionCount = (CAN_Rx.data[6] << 8) | CAN_Rx.data[7];
                   MotorManager_SetPositionCount(Motor_Left, LeftMotor_PositionCount);
                   CalculateShaftTurn(Motor_Left);
                }
                else if(CAN_Rx.data[1]==0x00&&CAN_Rx.data[2]==0x21){
                    LeftMotor_Current = (CAN_Rx.data[6] << 8) | CAN_Rx.data[7];
                    MotorManager_SetCurrent(Motor_Left, LeftMotor_Current);                    
                }
            }
            if(CAN_Rx.msgId==0x0580007D){
                if(CAN_Rx.data[1]==0x00&&CAN_Rx.data[2]==0x21){
                ThumbleMotor_Current = (CAN_Rx.data[6] << 8) | CAN_Rx.data[7];
                MotorManager_SetCurrent(Motor_Thumble, ThumbleMotor_Current);
                IMUHandler_SetThumbleCurrent(ThumbleMotor_Current);
                }               
            }
           
            
        }
    }
}
