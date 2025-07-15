/*
 * File:   pmb_MotorManager.c
 * Author: Jaca
 *
 * Created on 20 lipca 2023, 21:14
 */
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "xc.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/can_types.h"

#include "Tools/Timer.h"
#include "TimeManager/TimeManager.h"
#include "IMUHandler/IMUHandler.h"
#include "AnalogHandler/AnalogHandler.h"

#include "DriveIndicator.h"
#include "pmb_CAN.h"
#include "pmb_Functions.h"
#include "pmb_MotorManager.h"
#include "pmb_Keyboard.h"
#include "pmb_Display.h"
#include "pmb_Settings.h"
#include "pmb_System.h"
#include "DiagnosticsHandler.h"
#include "RoutesDataTypes.h"

#include "BatteryManager/BatteryManager.h"

#define dTimer_5ms   5U
#define dTimer_20ms  20U
#define dTimer_250ms  250U
#define dTimer_750ms  750U

#define INCREASE_SPEED_VALUE  100U

typedef enum DriveType_t{
    Drive_Forward = 0,
    Drive_Backward,
    Drive_RightTurn,
    Drive_LeftTurn,
    Drive_ThumbleForward,
    Drive_ThumbleBackward,
    Drive_Route,
    Drive_Stop,       
    Drive_Lift_UP,
    Drive_Lift_DOWN,
    Drive_Belt1_ON,
    Drive_Belt2_ON
}DriveType;

typedef struct Motor_Parameters_t
{ 
    MotorName Name;
    bool Enable;
    uint8_t Direction;
    uint8_t StepDirection;
    uint16_t Speed;
    uint16_t StepSpeed;
    bool Higher_Speed_Flag;
    bool UpdateSpeedRequest;
    uint32_t ID; 
    uint16_t Position_Count;//
    uint16_t Position_CountPrev;
    int32_t  PositionAcc;
    int32_t  Rotation_Count; 
    int32_t Rotation_Count_Positive;    //Needed for absolute distance readings on the display
    uint32_t Road_Measured;
    uint32_t Road_Saved;
    int16_t Current;
}MotorParameters;

MotorParameters Motor[Motor_NumOf];

DriveType DriveState;
DriveType DriveState_Previous;

StateMachineStates CurrentState;

Timer EncoderInquiryTimer;
Timer CurrentInquiryTimer;
Timer MotorEnableTimer;
Timer RotationCountResetTimer;

CAN_MSG_OBJ CAN_Motor_Left;
CAN_MSG_OBJ CAN_Motor_Right;
CAN_MSG_OBJ CAN_Motor_Thumble;
CAN_MSG_OBJ CAN_Motor_Lift;
CAN_MSG_OBJ CAN_Motor_Belt1;
CAN_MSG_OBJ CAN_Motor_Belt2;
CAN_MSG_OBJ CAN_Tx;

uint8_t CAN_Motor_EN[8] = {0x23,0x0D,0x20,0x01,0x00,0x00,0x00,0x00};
uint8_t CAN_Motor_DN[8] = {0x23,0x0C,0x20,0x01,0x00,0x00,0x00,0x00};
uint8_t CAN_Motor[8] = {0x23,0x00,0x20,0x01,0x00,0x00,0x00,0x00};
uint8_t CAN_MotorStop[8] = {0x23,0x00,0x20,0x01,0x00,0x00,0x00,0x00};
uint8_t CAN_Motor_Position[8] = {0x40,0x04,0x21,0x01,0x00,0x00,0x00,0x00};
uint8_t CAN_Motor_Current[8] = {0x40,0x00,0x21,0x01,0x00,0x00,0x00,0x00};

bool RotationCountResetRequest;
bool EnableSendRequest;
bool EncoderToSend;
uint8_t CurrentToSend = 0;
bool currentSafetyState = false, previousSafetyState = false;

uint16_t Correction_Cnt;

//Variables used for displaying dev data on the display
uint16_t RWheelSetSpeed = DEFAULT_SPEED, LWheelSetSpeed = DEFAULT_SPEED, AugSetSpeed = DEFAULT_SPEED_THUMBLE;
uint16_t LastRotL = 0, LastRotR = 0;
float CurrentAngle2 = 0, StepAngle = 0, PrevStepAngle = 0;
int IntStepAngle = 0;

//moover lift end switches variables
bool upperSwitchLastStatus = 0;
bool lowerSwitchLastStatus = 0;

//Functions
void MotorManager_HandleKeyboardEvent(KeyboardButton Event);
void MotorManager_HandleDisplayEvent(DisplayButton Event);
void MotorManager_SendEncoderInquiry(void);
void MotorManager_SendEnableMessage(void);
void MotorManager_StopMotor(MotorName Mot);
void MotorManager_HandleDrive(void);
void MotorManager_ToggleDrive(DriveType Event);
void MotorManager_StartMotor(MotorName Mot, uint8_t Direction);
void MotorManager_SetDrive(DriveType Drive);
void MotorManager_ToggleHigherSpeed(MotorName Mot);
void MotorManager_HandleRemoteEvent(RemoteButton Event);
void MotorManager_SendCurrentInquiry(void);
void MotorManager_ClearEventDuringError(DisplayButton *DisplayEvent, RemoteButton *RemoteEvent, KeyboardEvent *Keyboard);

//Motor Functions

void MotorManager_Initialise(void){
    //Definition of inverters ID
    Motor[Motor_Belt1].ID = 0x0600007A;
    Motor[Motor_Belt2].ID = 0x0600007B;
    Motor[Motor_Lift].ID = 0x0600007C;
    Motor[Motor_Thumble].ID = 0x0600007D;
    Motor[Motor_Right].ID = 0x0600007E;
    Motor[Motor_Left].ID = 0x0600007F;

    Motor[Motor_Right].Name = Motor_Right;
    Motor[Motor_Left].Name = Motor_Left;
    Motor[Motor_Thumble].Name = Motor_Thumble;
    Motor[Motor_Lift].Name = Motor_Lift;
    Motor[Motor_Belt1].Name = Motor_Belt1;
    Motor[Motor_Belt2].Name = Motor_Belt2;

    CAN_Tx.field.formatType = CAN_2_0_FORMAT;
    CAN_Tx.field.brs = CAN_NON_BRS_MODE;
    CAN_Tx.field.frameType = CAN_FRAME_DATA;
    CAN_Tx.field.idType = CAN_FRAME_EXT;
    CAN_Tx.field.dlc = DLC_8;   
        
    CAN_Motor_Right.msgId = Motor[Motor_Right].ID;
    CAN_Motor_Right.field.formatType = CAN_2_0_FORMAT;
    CAN_Motor_Right.field.brs = CAN_NON_BRS_MODE;
    CAN_Motor_Right.field.frameType = CAN_FRAME_DATA;
    CAN_Motor_Right.field.idType = CAN_FRAME_EXT;
    CAN_Motor_Right.field.dlc = DLC_8;   
    
    CAN_Motor_Left.msgId = Motor[Motor_Left].ID;
    CAN_Motor_Left.field.formatType = CAN_2_0_FORMAT;
    CAN_Motor_Left.field.brs = CAN_NON_BRS_MODE;
    CAN_Motor_Left.field.frameType = CAN_FRAME_DATA;
    CAN_Motor_Left.field.idType = CAN_FRAME_EXT;
    CAN_Motor_Left.field.dlc = DLC_8;  
        
    CAN_Motor_Thumble.msgId = Motor[Motor_Thumble].ID;
    CAN_Motor_Thumble.field.formatType = CAN_2_0_FORMAT;
    CAN_Motor_Thumble.field.brs = CAN_NON_BRS_MODE;
    CAN_Motor_Thumble.field.frameType = CAN_FRAME_DATA;
    CAN_Motor_Thumble.field.idType = CAN_FRAME_EXT;
    CAN_Motor_Thumble.field.dlc = DLC_8;  
    
    CAN_Motor_Lift.msgId = Motor[Motor_Lift].ID;
    CAN_Motor_Lift.field.formatType = CAN_2_0_FORMAT;
    CAN_Motor_Lift.field.brs = CAN_NON_BRS_MODE;
    CAN_Motor_Lift.field.frameType = CAN_FRAME_DATA;
    CAN_Motor_Lift.field.idType = CAN_FRAME_EXT;
    CAN_Motor_Lift.field.dlc = DLC_8;  
    
    CAN_Motor_Belt1.msgId = Motor[Motor_Belt1].ID;
    CAN_Motor_Belt1.field.formatType = CAN_2_0_FORMAT;
    CAN_Motor_Belt1.field.brs = CAN_NON_BRS_MODE;
    CAN_Motor_Belt1.field.frameType = CAN_FRAME_DATA;
    CAN_Motor_Belt1.field.idType = CAN_FRAME_EXT;
    CAN_Motor_Belt1.field.dlc = DLC_8;  
    
    CAN_Motor_Belt2.msgId = Motor[Motor_Belt2].ID;
    CAN_Motor_Belt2.field.formatType = CAN_2_0_FORMAT;
    CAN_Motor_Belt2.field.brs = CAN_NON_BRS_MODE;
    CAN_Motor_Belt2.field.frameType = CAN_FRAME_DATA;
    CAN_Motor_Belt2.field.idType = CAN_FRAME_EXT;
    CAN_Motor_Belt2.field.dlc = DLC_8;  
    
    
    Motor[Motor_Left].Enable = false;
    Motor[Motor_Right].Enable = false;
    Motor[Motor_Thumble].Enable = false;
    Motor[Motor_Lift].Enable = false;
    Motor[Motor_Belt1].Enable = false;
    Motor[Motor_Belt2].Enable = false;
    
    //Definition of inverters default speed
    Motor[Motor_Left].Speed = DEFAULT_SPEED; // ~23 RPM -> 14,5m/min
    Motor[Motor_Right].Speed = DEFAULT_SPEED; // ~23 RPM -> 14,5m/min
    Motor[Motor_Thumble].Speed = DEFAULT_SPEED_THUMBLE; // 40 RPM
    Motor[Motor_Thumble].Direction = dRIGHT;
    Motor[Motor_Lift].Speed = DEFAULT_SPEED_LIFT;
    Motor[Motor_Belt1].Speed = DEFAULT_SPEED_BELT;
    Motor[Motor_Belt2].Speed = DEFAULT_SPEED_BELT;
    
    //Clear Rotation Counter
    Motor[Motor_Right].Rotation_Count = 0;
    Motor[Motor_Left].Rotation_Count = 0;

    //TimerSetCounter(&EncoderInquiryTimer, dTimer_20ms);
    //TimerSetCounter(&MotorEnableTimer, dTimer_250ms*2);
    
    upperSwitchLastStatus = DBG2_GetValue();
    lowerSwitchLastStatus = DBG1_GetValue();

    CurrentState = State_Init;
    DriveState = Drive_Stop;
    DriveState_Previous = Drive_Stop;
    TimerSetCounter(&EncoderInquiryTimer, dTimer_5ms);
    EnableSendRequest = false;
}

void MotorManager_Perform1ms( void ){
    if( MotorManager_IsAnyMotorEnabled() ){
        LED1_SetHigh();
        Timer_Tick(&EncoderInquiryTimer);
        Timer_Tick(&CurrentInquiryTimer);
    }
    else{
        LED1_SetLow();
    }
    if( EnableSendRequest ){
        Timer_Tick(&MotorEnableTimer);
    }
}

void MotorManager_Perform100ms( void ){
    //lift motor end stop handler
    if(DBG2_GetValue() != upperSwitchLastStatus)//upper switch status changed
    {
        if(DBG2_GetValue()==0)//switch got closed - stop lift motor
          MotorManager_StopMotor(Motor_Lift); 
       
            
        upperSwitchLastStatus = DBG2_GetValue();
    }
    
    if(DBG1_GetValue() != lowerSwitchLastStatus)//lower switch status changed
    {
        if(DBG1_GetValue()==0)//switch got closed - stop lift motor
          MotorManager_StopMotor(Motor_Lift); 
       
            
        lowerSwitchLastStatus = DBG2_GetValue();
    }
}

void MotorManager_PerformAfterMainLoop(void){
        /* Handle getting data from encoders each 5ms */
    if(Timer_IsExpired(&EncoderInquiryTimer) ){
        /* 5ms passed, get data from encoders */
        MotorManager_SendEncoderInquiry();
        TimerSetCounter(&EncoderInquiryTimer, dTimer_5ms);
    }
    if(Timer_IsExpired(&RotationCountResetTimer)){
        if( MotorManager_IsRotationCountResetRequest() ){
                MotorManager_ResetRotationCount(Motor_Left);
                MotorManager_ResetRotationCount(Motor_Right);
                MotorManager_ResetRotationCountResetRequest();
        }
    }
    if(Timer_IsExpired(&CurrentInquiryTimer) ){
        MotorManager_SendCurrentInquiry();
        TimerSetCounter(&CurrentInquiryTimer, dTimer_20ms);
    }

    if( EnableSendRequest){
        if(Timer_IsExpired(&MotorEnableTimer)){
            EnableSendRequest = false;
            MotorManager_SendEnableMessage();
        }
    }
}

void MotorManager_StateMachine( void ){
    KeyboardEvent Keyboard;
    Keyboard.Button = Keyboard_Released;
    DisplayButton  Display_Button = Display_Released;
    RemoteButton  Remote_Button = Remote_Released;
    BatteryLevel Battery = BatteryManager_GetBatteryLevel();

    if( Battery != BatteryLevel_Critical ){
        Display_Button =   Display_GetEvent();
        Remote_Button = IMUHandler_GetRemoteMessage();
        Keyboard =  Keyboard_GetEvent();
        
        MotorManager_ClearEventDuringError(&Display_Button, &Remote_Button, &Keyboard);

        /* If battery level is overvoltage, let user control moover */
        if( Battery == BatteryLevel_Overvoltage ) {
            Battery = BatteryLevel_Good;
        }      
    }  
    /* Main state machine */
    switch(CurrentState){
        case State_Init:
            CurrentState = State_WaitForEvent;
            DriveState = Drive_Stop;
            DriveState_Previous = Drive_Stop;
            break;
        case State_Stop:
            MotorManager_StopMotor(Motor_Left);
            MotorManager_StopMotor(Motor_Right);
            MotorManager_StopMotor(Motor_Thumble);      
            MotorManager_StopMotor(Motor_Lift);
            MotorManager_SetDefaultSpeed();
            CurrentState = State_WaitForEvent;
            break;
        case State_WaitForEvent:
            if( Keyboard.Button != Keyboard_Released){
                /* Handle event from keyboard */
                MotorManager_HandleKeyboardEvent(Keyboard.Button);
            }
            else if(Display_Button != Display_Released){
                /* Handle event from display */
                MotorManager_HandleDisplayEvent(Display_Button);
            }
            else if(Remote_Button != Remote_Released){
                /* Handle event from display */
                MotorManager_HandleRemoteEvent(Remote_Button);
            }
/* FIRST SOLUTION - REENABLE POWER RAIL AS SOON AS SAFETY ACTIVATED */             
            //previousSafetyState = currentSafetyState;
/* ================================================================*/            
            break;
        case State_Track:
            /* During this state, RouteManager is handling driving */
            /* If emergancy stop event appear */
            /* If manual event appear */
            if(Keyboard.Button == Keyboard_LEFT){
                /* Increase/Decrease speed of right wheel to correct route */
                MotorManager_ToggleHigherSpeed(Motor_Left);                
            }
            if( Keyboard.Button == Keyboard_RIGHT ){
                /* Increase/Decrease speed of left wheel to correct route */
                MotorManager_ToggleHigherSpeed(Motor_Right);
            }
           
            break;
        default:
            break;                
    }
    MotorManager_HandleDrive();
}

void MotorManager_ClearEventDuringError(DisplayButton *DisplayEvent, RemoteButton *RemoteEvent, KeyboardEvent *Keyboard){
    /* Take actions, when one of wheels inverter is disconnected */
    if( !Diagnostics_IsInvertersReady()){
        if( *DisplayEvent == Display_UP 
        || *DisplayEvent == Display_DOWN
        || *DisplayEvent == Display_LEFT
        || *DisplayEvent == Display_RIGHT){
            DriveIndicator_SetIndication(500, 0);
            *DisplayEvent = Display_Released;
        }

        if( *RemoteEvent == Remote_UP 
            || *RemoteEvent == Remote_DOWN
            || *RemoteEvent == Remote_LEFT
            || *RemoteEvent == Remote_RIGHT){
                DriveIndicator_SetIndication(500, 0);
                *RemoteEvent = Remote_Released;
            }

        if( Keyboard->Button == Keyboard_UP 
            || Keyboard->Button == Keyboard_DOWN
            || Keyboard->Button == Keyboard_LEFT
            || Keyboard->Button == Keyboard_RIGHT){
                DriveIndicator_SetIndication(500, 0);
                Keyboard->Button = Keyboard_Released;
            }
    } 
}

void MotorManager_SendData(CAN_MSG_OBJ *MotorStructure){
    if(CAN_TX_FIFO_AVAILABLE == (CAN1_TransmitFIFOStatusGet(CAN1_TX_TXQ) & CAN_TX_FIFO_AVAILABLE)){    
        CAN1_Transmit(CAN1_TX_TXQ, MotorStructure); 
    }
}

void MotorManager_SendEnableMessage(void){
    if( Motor[Motor_Right].Enable ){
        CAN_Motor_Right.data = &CAN_Motor_EN[0];
        MotorManager_SendData(&CAN_Motor_Right);
    }
    if( Motor[Motor_Left].Enable ){
        CAN_Motor_Left.data = &CAN_Motor_EN[0];
        MotorManager_SendData(&CAN_Motor_Left);
    }
    if( Motor[Motor_Thumble].Enable ){
        CAN_Motor_Thumble.data = &CAN_Motor_EN[0];
        MotorManager_SendData(&CAN_Motor_Thumble);
    }
    if( Motor[Motor_Lift].Enable ){
        CAN_Motor_Lift.data = &CAN_Motor_EN[0];
        MotorManager_SendData(&CAN_Motor_Lift);
    }
    if( Motor[Motor_Belt1].Enable ){
        CAN_Motor_Belt1.data = &CAN_Motor_EN[0];
        MotorManager_SendData(&CAN_Motor_Belt1);
    }
    if( Motor[Motor_Belt2].Enable ){
        CAN_Motor_Belt2.data = &CAN_Motor_EN[0];
        MotorManager_SendData(&CAN_Motor_Belt2);
    }
}

void MotorManager_TriggerEnableMessageSend(uint16_t Timeout){
    /* Set 500ms counter before sending ENABLE message */
    Timer_SetCounter(&MotorEnableTimer, Timeout);
    EnableSendRequest = true;

}

void MotorManager_SendEncoderInquiry(void){
    if( Motor[Motor_Right].Enable && EncoderToSend){
        CAN_Motor_Right.data = &CAN_Motor_Position[0];
        MotorManager_SendData(&CAN_Motor_Right);
    }
    if( Motor[Motor_Left].Enable && !EncoderToSend){
        CAN_Motor_Left.data = &CAN_Motor_Position[0];
        MotorManager_SendData(&CAN_Motor_Left);
    }
    if( Motor[Motor_Thumble].Enable && !EncoderToSend){//czy nie trzeba tego wywalic?
        CAN_Motor_Thumble.data = &CAN_Motor_Position[0];
        MotorManager_SendData(&CAN_Motor_Thumble);
    }
//    if( Motor[Motor_Lift].Enable && !EncoderToSend){//czy nie trzeba tego wywalic?
//        CAN_Motor_Lift.data = &CAN_Motor_Position[0];
//        MotorManager_SendData(&CAN_Motor_Lift);
//    }
//    if( Motor[Motor_Belt1].Enable && !EncoderToSend){//czy nie trzeba tego wywalic?
//        CAN_Motor_Lift.data = &CAN_Motor_Position[0];
//        MotorManager_SendData(&CAN_Motor_Belt1);
//    }
//    if( Motor[Motor_Belt2].Enable && !EncoderToSend){//czy nie trzeba tego wywalic?
//        CAN_Motor_Lift.data = &CAN_Motor_Position[0];
//        MotorManager_SendData(&CAN_Motor_Belt2);
//    }
    if(EncoderToSend == 0)
        EncoderToSend = 1;
    else
        EncoderToSend = 0;
}
void MotorManager_SendCurrentInquiry(void){
    switch(CurrentToSend){
        case 0:
            CurrentToSend = CurrentToSend + 1;
            if( Motor[Motor_Right].Enable){
            
            CAN_Motor_Right.data = &CAN_Motor_Current[0];
                MotorManager_SendData(&CAN_Motor_Right);
            }            
            break;
        case 1:
            CurrentToSend = CurrentToSend + 1;
            if( Motor[Motor_Left].Enable){
                CAN_Motor_Left.data = &CAN_Motor_Current[0];
                MotorManager_SendData(&CAN_Motor_Left);
            }      
            break;
        case 2:
            CurrentToSend = CurrentToSend + 1;
            if( Motor[Motor_Thumble].Enable){
                CAN_Motor_Thumble.data = &CAN_Motor_Current[0];
                MotorManager_SendData(&CAN_Motor_Thumble);
            }      
            break;
        case 3:
            CurrentToSend = CurrentToSend + 1;//lift motor current  
            if( Motor[Motor_Lift].Enable){
                CAN_Motor_Lift.data = &CAN_Motor_Current[0];
                MotorManager_SendData(&CAN_Motor_Lift);
            }
            break;
        case 4:
            CurrentToSend = CurrentToSend + 1;//belt1 motor current  
            if( Motor[Motor_Belt1].Enable){
                CAN_Motor_Belt1.data = &CAN_Motor_Current[0];
                MotorManager_SendData(&CAN_Motor_Belt1);
            }
            break;
        case 5:
            CurrentToSend = CurrentToSend + 1;//Belt2 motor current  
            if( Motor[Motor_Belt2].Enable){
                CAN_Motor_Belt2.data = &CAN_Motor_Current[0];
                MotorManager_SendData(&CAN_Motor_Belt2);
            }
            break;
        case 6:
            CurrentToSend = 0;
            break;
        default:
            break;
    }
}


void MotorManager_HandleKeyboardEvent(KeyboardButton Event)
{
    switch(Event)
    {
        case Keyboard_UP:
            MotorManager_SetSpeed(Motor_Left, 700);
            MotorManager_SetSpeed(Motor_Right, 700);
            MotorManager_ToggleDrive(Drive_Forward);
            break;
        case Keyboard_DOWN:
            MotorManager_SetSpeed(Motor_Left, 700);
            MotorManager_SetSpeed(Motor_Right, 700);
            MotorManager_ToggleDrive(Drive_Backward);
            break;
        case Keyboard_RIGHT:
            MotorManager_SetSpeed(Motor_Left, 200);
            MotorManager_SetSpeed(Motor_Right, 200);
            MotorManager_ToggleDrive(Drive_LeftTurn);
            break;
        case Keyboard_LEFT:
            MotorManager_SetSpeed(Motor_Left, 200);
            MotorManager_SetSpeed(Motor_Right, 200);
            MotorManager_ToggleDrive(Drive_RightTurn);
            break;
        default:
            break;
    }
    MotorManager_HandleDrive();
}

void MotorManager_HandleDisplayEvent(DisplayButton Event)
{
    switch(Event){
        case Display_UP:
            MotorManager_ToggleDrive(Drive_Forward);
            break;
        case Display_DOWN:
            MotorManager_ToggleDrive(Drive_Backward);
            break;
        case Display_RIGHT:
            MotorManager_ToggleDrive(Drive_RightTurn);
            break;
        case Display_LEFT:
            MotorManager_ToggleDrive(Drive_LeftTurn);
            break;
        case Display_SLIDER_WHEELS:
            //MotorManager_SetUpdateSpeedRequest(Motor_Left);
            //MotorManager_SetUpdateSpeedRequest(Motor_Right);
            if( Motor[Motor_Left].Enable ){
                MotorManager_StartMotorKeepDirection(Motor_Left);
            }
            if(Motor[Motor_Right].Enable){
                MotorManager_StartMotorKeepDirection(Motor_Right);
            }
           
            break;
        case Display_SLIDER_THUMBLE:
            if( Motor[Motor_Thumble].Enable ){
                MotorManager_StartMotorKeepDirection(Motor_Thumble);
            }
            break;
        case Display_EMERGANCY_STOP://hihi
            DriveState_Previous = Drive_Stop;
            DriveState = Drive_Stop;
            Diagnostics_SetEvent(dDebug_StopEmergancy);
            MotorManager_SetStateMachineState(State_Stop);
            break;
        case Display_BARREL_FORWARD:
            MotorManager_ToggleDrive(Drive_ThumbleForward);
            break;
        case Display_BARREL_REVERSE:
            MotorManager_ToggleDrive(Drive_ThumbleBackward);
            break;
        case Display_BARREL_STOP:
            MotorManager_StopMotor(Motor_Thumble);
            break;
        case Display_Lift_UP:
            MotorManager_ToggleDrive(Drive_Lift_UP);
            break;
        case Display_Lift_DOWN:
            MotorManager_ToggleDrive(Drive_Lift_DOWN);
            break;
        case Display_Lift_STOP:
            MotorManager_StopMotor(Motor_Lift);
            break;    
        case Display_UPPER_BELT_ON:
            MotorManager_ToggleDrive(Drive_Belt1_ON);
            break;
        case Display_UPPER_BELT_OFF:
            MotorManager_StopMotor(Motor_Belt1);
            break;
        case Display_LOWER_BELT_ON:
            MotorManager_ToggleDrive(Drive_Belt2_ON);
            break;
        case Display_LOWER_BELT_OFF:
            MotorManager_StopMotor(Motor_Belt2);
            break;
        case Display_SLIDER_UPPER_BELT:
            if( Motor[Motor_Belt1].Enable ){
                MotorManager_StartMotorKeepDirection(Motor_Belt1);
            }
            break;
        case Display_SLIDER_LOWER_BELT:
            if( Motor[Motor_Belt2].Enable ){
                MotorManager_StartMotorKeepDirection(Motor_Belt2);
            }
            break;
        case Display_ENABLE_POWER:
            System_PowerRailRequestSequence(Sequence_PowerStageOn);
            break;
        case Display_DISABLE_POWER:
            System_PowerRailRequestSequence(Sequence_PowerStageOff);
            break; 
        case Display_ENABLE_CHARGER:
            System_PowerRailRequestSequence(Sequence_ChargerOn);
            break;
        case Display_DISABLE_CHARGER:
            System_PowerRailRequestSequence(Sequence_ChargerOff);
            break;
            
        default:
            break;
    }
    
    /* If pressed route selected button, change state to track*/
    if(Display_ROUTE_A <= Event && (Display_ROUTE_K + 1) > Event)
        CurrentState = State_Track;
    MotorManager_HandleDrive();
}

void MotorManager_HandleRemoteEvent(RemoteButton Event)
{
    uint16_t RemoteSpeed = (uint16_t)Remote_GetSpeed() * 5;       
    switch(Event)
    {
        case Remote_UP:
            MotorManager_SetSpeed(Motor_Left, 200);
            MotorManager_SetSpeed(Motor_Right, 200);
            MotorManager_ToggleDrive(Drive_Forward);
            break;
        case Remote_DOWN:
            MotorManager_SetSpeed(Motor_Left, 200);
            MotorManager_SetSpeed(Motor_Right, 200);
            MotorManager_ToggleDrive(Drive_Backward);
            break;
        case Remote_RIGHT:
            MotorManager_SetSpeed(Motor_Left, 50);
            MotorManager_SetSpeed(Motor_Right, 50);
            MotorManager_ToggleDrive(Drive_RightTurn);
            break;
        case Remote_LEFT:
            MotorManager_SetSpeed(Motor_Left, 50);
            MotorManager_SetSpeed(Motor_Right, 50);
            MotorManager_ToggleDrive(Drive_LeftTurn);
            break;
        case Remote_Lift_UP:
            MotorManager_ToggleDrive(Drive_Lift_UP);
            break;
        case Remote_Lift_DOWN:
            MotorManager_ToggleDrive(Drive_Lift_DOWN);
            break;
        case Remote_Belt1_ON:
            MotorManager_ToggleDrive(Drive_Belt1_ON);
            break;
        case Remote_Belt2_ON:
            MotorManager_ToggleDrive(Drive_Belt2_ON);
            break;
        case Remote_Stop:
            DriveState_Previous = Drive_Stop;
            DriveState = Drive_Stop;
            Diagnostics_SetEvent(dDebug_StopEmergancy);
            MotorManager_SetStateMachineState(State_Stop);
            break;    
        case Remote_PowerON:
            System_PowerRailRequestSequence(Sequence_PowerStageOn);
            break;
        case Remote_PowerOFF:
            System_PowerRailRequestSequence(Sequence_PowerStageOff);
            break;    
        case Remote_ChargeON:
            System_PowerRailRequestSequence(Sequence_ChargerOn);
            break;
        case Remote_ChargeOFF:
            System_PowerRailRequestSequence(Sequence_ChargerOff);
            break;
        case Remote_ThumbleStart:
                MotorManager_ToggleDrive(Drive_ThumbleBackward);
            break;    
        case Remote_ThumbleStop:
                DriveState_Previous = Drive_Stop;
                DriveState = Drive_Stop;
                //MotorManager_StopMotor(Motor_Thumble);
            break; 
        case Remote_Speed:
 
            if( Motor[Motor_Left].Enable ){
                MotorManager_SetSpeed(Motor_Left,RemoteSpeed);
                MotorManager_StartMotorKeepDirection(Motor_Left);
            }
            else{
                MotorManager_SetSpeed(Motor_Left,RemoteSpeed);
            }
            if(Motor[Motor_Right].Enable){
                MotorManager_SetSpeed(Motor_Right, RemoteSpeed); 
                MotorManager_StartMotorKeepDirection(Motor_Right);
            }
            else{
                MotorManager_SetSpeed(Motor_Right, RemoteSpeed); 
            }            
            break;
            
        default:
            break;
    }    /* If pressed route selected button, change state to track*/
    if(Remote_RouteA <= Event && (Remote_RouteK + 1) < Event)
        CurrentState = State_Track;
    MotorManager_HandleDrive();
}

void MotorManager_SetUpdateSpeedRequest(MotorName Mot){
    Motor[Mot].UpdateSpeedRequest = true;
}

void MotorManager_StartMotorKeepDirection(MotorName Mot){
    MotorManager_StartMotor(Mot, Motor[Mot].Direction);
}

void MotorManager_StartMotorOpositeDirection(MotorName Mot){
    
    if(Motor[Mot].Direction == L_REV)
    MotorManager_StartMotor(Mot, L_FOR);
    
    else if(Motor[Mot].Direction == L_FOR)
    MotorManager_StartMotor(Mot, L_REV);
    
    else if(Motor[Mot].Direction == R_REV)
    MotorManager_StartMotor(Mot, R_FOR);
    
    else if(Motor[Mot].Direction == R_FOR)
    MotorManager_StartMotor(Mot, R_REV);
}

void MotorManager_StartMotor(MotorName Mot, uint8_t Direction){
    uint16_t CalculatedSpeed;
    Motor[Mot].Direction = Direction;
    Motor[Mot].Enable = 1;
    uint8_t SpeedDataLow, SpeedDataHigh;
    /* Calculation of speed regarding forward/backward direction */
    
    uint32_t MotorSpeed = (uint32_t)Motor[Mot].Speed;
    if( dRIGHT == Direction ){
        CalculatedSpeed = 65535 - MotorSpeed * 100 / 15;
        SpeedDataHigh = 0xFF;
        SpeedDataLow = 0xFF; 
    }
    else if(dLEFT == Direction){
        CalculatedSpeed = MotorSpeed * 100 / 15; //proportionally 1500 RPM = 10000
        SpeedDataHigh = 0x00;
        SpeedDataLow = 0x00;
    }
    switch(Mot){
        case Motor_Left:        
            CAN_Motor_Left.data = &CAN_Motor[0];
            CAN_Motor_Left.data[4] = SpeedDataLow;
            CAN_Motor_Left.data[5] = SpeedDataHigh;
            CAN_Motor_Left.data[6] = CalculatedSpeed >> 8;
            CAN_Motor_Left.data[7] = CalculatedSpeed;
            MotorManager_SendData(&CAN_Motor_Left);
            break;
        case Motor_Right:
            CAN_Motor_Right.data = &CAN_Motor[0];
            CAN_Motor_Right.data[4] = SpeedDataLow;
            CAN_Motor_Right.data[5] = SpeedDataHigh;        
            CAN_Motor_Right.data[6] = CalculatedSpeed >> 8;
            CAN_Motor_Right.data[7] = CalculatedSpeed;
            MotorManager_SendData(&CAN_Motor_Right);
            break;
        case Motor_Thumble:
            CAN_Motor_Thumble.data = &CAN_Motor[0];
            CAN_Motor_Thumble.data[4] = SpeedDataLow;
            CAN_Motor_Thumble.data[5] = SpeedDataHigh;  
            CAN_Motor_Thumble.data[6] = (uint8_t)(CalculatedSpeed >> 8);
            CAN_Motor_Thumble.data[7] = CalculatedSpeed;
            MotorManager_SendData(&CAN_Motor_Thumble);
            break;
        case Motor_Lift:
            CAN_Motor_Lift.data = &CAN_Motor[0];
            CAN_Motor_Lift.data[4] = SpeedDataLow;
            CAN_Motor_Lift.data[5] = SpeedDataHigh;  
            CAN_Motor_Lift.data[6] = (uint8_t)(CalculatedSpeed >> 8);
            CAN_Motor_Lift.data[7] = CalculatedSpeed;
            MotorManager_SendData(&CAN_Motor_Lift);
            break;
         case Motor_Belt1:
            CAN_Motor_Belt1.data = &CAN_Motor[0];
            CAN_Motor_Belt1.data[4] = SpeedDataLow;
            CAN_Motor_Belt1.data[5] = SpeedDataHigh;  
            CAN_Motor_Belt1.data[6] = (uint8_t)(CalculatedSpeed >> 8);
            CAN_Motor_Belt1.data[7] = CalculatedSpeed;
            MotorManager_SendData(&CAN_Motor_Belt1);
            break;
         case Motor_Belt2:
            CAN_Motor_Belt2.data = &CAN_Motor[0];
            CAN_Motor_Belt2.data[4] = SpeedDataLow;
            CAN_Motor_Belt2.data[5] = SpeedDataHigh;  
            CAN_Motor_Belt2.data[6] = (uint8_t)(CalculatedSpeed >> 8);
            CAN_Motor_Belt2.data[7] = CalculatedSpeed;
            MotorManager_SendData(&CAN_Motor_Belt2);
            break;
        default:
            break;
    }
}

void MotorManager_StopMotor(MotorName Mot){
    Motor[Mot].Enable = 0;
    Motor[Mot].Current = 0;
    
    switch(Mot){
        case Motor_Left:
            CAN_Motor_Left.data = &CAN_MotorStop[0];
            MotorManager_SendData(&CAN_Motor_Left);
            LastRotL = MotorManager_GetRotationCountPositive(Motor_Left) * dDISTANCE_PER_MOTOR_ROTATION;
            MotorManager_ResetRotationCountPositive(Motor_Left);
            break;
        case Motor_Right:
            CAN_Motor_Right.data = &CAN_MotorStop[0];
            MotorManager_SendData(&CAN_Motor_Right);
            LastRotR = MotorManager_GetRotationCountPositive(Motor_Right) * dDISTANCE_PER_MOTOR_ROTATION;
            MotorManager_ResetRotationCountPositive(Motor_Right);
            break;
        case Motor_Thumble:
            CAN_Motor_Thumble.data = &CAN_MotorStop[0];
            MotorManager_SendData(&CAN_Motor_Thumble);
            break;
        case Motor_Lift:
            CAN_Motor_Lift.data = &CAN_MotorStop[0];
            MotorManager_SendData(&CAN_Motor_Lift);
        case Motor_Belt1:
            CAN_Motor_Belt1.data = &CAN_MotorStop[0];
            MotorManager_SendData(&CAN_Motor_Belt1);
        case Motor_Belt2:
            CAN_Motor_Belt2.data = &CAN_MotorStop[0];
            MotorManager_SendData(&CAN_Motor_Belt2);
       break;
        default:
            break;
    } 
}

MotorManager_StopAllMotors(void){
    MotorManager_StopMotor(Motor_Left);
    MotorManager_StopMotor(Motor_Right);
    MotorManager_StopMotor(Motor_Thumble);
    MotorManager_StopMotor(Motor_Lift);
    MotorManager_StopMotor(Motor_Belt1);
    MotorManager_StopMotor(Motor_Belt2);
}

void MotorManager_HandleDrive(void){
    if( DriveState != DriveState_Previous ){
        MotorManager_SetDrive(DriveState);
        DriveState_Previous = DriveState;
    }
}

void MotorManager_SetDrive(DriveType Drive){
    switch (Drive){
        case Drive_Forward:
            MotorManager_TriggerEnableMessageSend(0);
            MotorManager_StartMotor(Motor_Left,  dLEFT);
            MotorManager_StartMotor(Motor_Right, dRIGHT);
            break;
        case Drive_Backward:
            MotorManager_TriggerEnableMessageSend(0);
            MotorManager_StartMotor(Motor_Left,  dRIGHT);
            MotorManager_StartMotor(Motor_Right, dLEFT);
            break;
        case Drive_RightTurn:
            MotorManager_TriggerEnableMessageSend(0);
            MotorManager_StartMotor(Motor_Right,  dLEFT);
            MotorManager_StartMotor(Motor_Left,  dLEFT);
            break;
        case Drive_LeftTurn:
            MotorManager_TriggerEnableMessageSend(0);
            MotorManager_StartMotor(Motor_Left, dRIGHT);
            MotorManager_StartMotor(Motor_Right,  dRIGHT);
            break;
        case Drive_ThumbleForward:
            MotorManager_TriggerEnableMessageSend(100);
            MotorManager_StartMotor(Motor_Thumble, dLEFT);
            break;
        case Drive_ThumbleBackward:
            MotorManager_TriggerEnableMessageSend(100);
            MotorManager_StartMotor(Motor_Thumble, dRIGHT);
            break;
        case Drive_Lift_UP:
            MotorManager_TriggerEnableMessageSend(100);
            MotorManager_StartMotor(Motor_Lift, dRIGHT);
            break;
        case Drive_Lift_DOWN:
            MotorManager_TriggerEnableMessageSend(100);
            MotorManager_StartMotor(Motor_Lift, dLEFT);
            break;
        case Drive_Belt1_ON:
            MotorManager_TriggerEnableMessageSend(100);
            MotorManager_StartMotor(Motor_Belt1, dRIGHT);
            break;
        case Drive_Belt2_ON:
            MotorManager_TriggerEnableMessageSend(100);
            MotorManager_StartMotor(Motor_Belt2, dLEFT);
            break;
        case Drive_Stop://add if(motor enabled) then stop???
            MotorManager_StopMotor(Motor_Left);
            MotorManager_StopMotor(Motor_Right);
            MotorManager_StopMotor(Motor_Thumble);
            MotorManager_StopMotor(Motor_Lift);
            MotorManager_StopMotor(Motor_Belt1);
            MotorManager_StopMotor(Motor_Belt2);
            break;
        case Drive_Route:
            break;
    }
}

void MotorManager_ToggleDrive(DriveType Event){
    /* Forward type event */
    if( Drive_Forward == Event ){
        if(  Drive_Forward != DriveState)
            DriveState = Drive_Forward;
        else{
                //MotorManager_SetSpeed(Motor_Left, MotorManager_GetSpeed(Motor_Left) + 70);
                //MotorManager_SetSpeed(Motor_Right, MotorManager_GetSpeed(Motor_Right) + 70);
            DriveState = Drive_Stop;
        }
    }
    /* Backward type event */
    if( Drive_Backward == Event ){
        if(  Drive_Backward != DriveState)
            DriveState = Drive_Backward;
        else
            DriveState = Drive_Stop;
    }
    /* Left turn type event */
    if( Drive_LeftTurn == Event ){
        if(  Drive_LeftTurn != DriveState)
            DriveState = Drive_LeftTurn;
        else
            DriveState = Drive_Stop;
    }
    /* Right turn type event */
    if( Drive_RightTurn == Event ){
        if(  Drive_RightTurn != DriveState)
            DriveState = Drive_RightTurn;
        else
            DriveState = Drive_Stop;
    }
    /* Thumble Forward type event */
    if( Drive_ThumbleForward == Event ){
        if(  Drive_ThumbleForward != DriveState)
            DriveState = Drive_ThumbleForward;
        else
            DriveState = Drive_Stop;
    }
    /* Thumble Backward type event */
    if( Drive_ThumbleBackward == Event ){
        if(  Drive_ThumbleBackward != DriveState)
            DriveState = Drive_ThumbleBackward;
        else
            DriveState = Drive_Stop;
    }
    /* Lift UP type event */
    if( Drive_Lift_UP == Event ){
        if(  Drive_Lift_UP != DriveState && DBG2_GetValue() == 1)//check if limit switch in not pressed
            DriveState = Drive_Lift_UP;
        else
            DriveState = Drive_Stop;
    }
    /* Lift DOWN type event */
    if( Drive_Lift_DOWN == Event ){
        if(  Drive_Lift_DOWN != DriveState && DBG1_GetValue() == 1)//check if limit switch in not pressed
            DriveState = Drive_Lift_DOWN;
        else
            DriveState = Drive_Stop;
    }
    if( Drive_Belt1_ON == Event ){
        if(  Drive_Belt1_ON != DriveState)
            DriveState = Drive_Belt1_ON;
        else
            DriveState = Drive_Stop;
    }
    if( Drive_Belt2_ON == Event ){
        if(  Drive_Belt2_ON != DriveState)
            DriveState = Drive_Belt2_ON;
        else
            DriveState = Drive_Stop;
    }
}

uint8_t CalculateShaftTurn( MotorName Name ){
    uint8_t Ret = 0;
    int16_t diff;
    uint16_t Current;
    uint16_t Previous;
    int16_t  Turn;
    //for(MotorName Name; Name < Motor_NumOf; Name++){
        Current = Motor[Name].Position_Count;//informacja z invertera
        Previous = Motor[Name].Position_CountPrev;//osatnia odczytana warto??
                /* Calculation for each motor */
        diff = Current - Previous;
        if (diff > ENCODER_MAX_VALUE / 2){
            diff -= ENCODER_MAX_VALUE;
        }
        else if(diff < -ENCODER_MAX_VALUE / 2){
            
            diff += ENCODER_MAX_VALUE;
        }
        
        //percentRotation = abs(diff) / (ENCODER_MAX_VALUE * 100);
        if (diff > 0){
            Turn = diff;
            Ret = dRIGHTSPIN;
        }     
        else if (diff < 0){
            Turn = diff;
            Ret = dLEFTSPIN;
        }
        else{
            Ret = 0;
        }

        Motor[Name].PositionAcc = Motor[Name].PositionAcc + diff;

        if( Motor[Name].PositionAcc > 10000 ){
            Motor[Name].PositionAcc = 0;
            Motor[Name].Rotation_Count ++;
            Motor[Name].Rotation_Count_Positive ++;
        }
        else if(Motor[Name].PositionAcc < -10000){
            Motor[Name].PositionAcc = 0;
            Motor[Name].Rotation_Count --;
            Motor[Name].Rotation_Count_Positive ++;
        }
        
        Motor[Name].Position_CountPrev = Current;
        return Ret;
}

void MotorManager_ToggleHigherSpeed(MotorName Mot){
    if( !Motor[Mot].Higher_Speed_Flag ){
        MotorManager_SetSpeed(Mot, Motor[Mot].Speed + INCREASE_SPEED_VALUE);
        MotorManager_StartMotorKeepDirection(Mot);
        Motor[Mot].Higher_Speed_Flag = true;
    }else{
        MotorManager_SetSpeed(Mot, Motor[Mot].Speed - INCREASE_SPEED_VALUE);
        Motor[Mot].Higher_Speed_Flag = false;
        MotorManager_StartMotorKeepDirection(Mot);
    }
}

void MotorManager_SetStateMachineState(StateMachineStates State){
    CurrentState = State;
}

void MotorManager_SetRotationCountResetRequest(void){
    MotorManager_ResetRotationCountPositive(Motor_Left);
    MotorManager_ResetRotationCountPositive(Motor_Right);
    TimerSetCounter(&RotationCountResetTimer, dTimer_750ms);
    RotationCountResetRequest = true;
}

void MotorManager_ResetRotationCountResetRequest(void){
    RotationCountResetRequest = false;
}


bool MotorManager_IsRotationCountResetRequest(void){
    return RotationCountResetRequest;
}

bool MotorManager_GetHigherSpeedFlag(MotorName Mot){
    return Motor[Mot].Higher_Speed_Flag;
}

void MotorManager_ResetHigherSpeedFlag(void){
    Motor[Motor_Left].Higher_Speed_Flag = 0;
    Motor[Motor_Right].Higher_Speed_Flag = 0;
}

void MotorManager_SaveRoad(void){
    Motor[Motor_Right].Road_Saved = Motor[Motor_Right].Road_Saved + Motor[Motor_Right].Road_Measured;
    Motor[Motor_Right].Road_Measured = 0;
    Motor[Motor_Right].Rotation_Count = 0;
    Motor[Motor_Left].Road_Saved = Motor[Motor_Left].Road_Saved + Motor[Motor_Left].Road_Measured;
    Motor[Motor_Left].Road_Measured = 0;
    Motor[Motor_Left].Rotation_Count = 0;  
}

void MotorManager_SetMotorState(uint8_t Mot, uint8_t State){
    if( Motor_Right ==  Mot){
        Motor[Motor_Right].Enable = State;
    }
    else if ( Motor_Left ==  Mot){
        Motor[Motor_Left].Enable = State;
    }
    else if ( Motor_Thumble ==  Mot){
        Motor[Motor_Thumble].Enable = State;
    }
}

void MotorManager_SetDirection(MotorName Mot, uint8_t Direction){
    if( Motor_Right ==  Mot){
        Motor[Motor_Right].Direction = Direction;
    }
    else if ( Motor_Left ==  Mot){
        Motor[Motor_Left].Direction = Direction;
    }
    else if ( Motor_Thumble ==  Mot){
        Motor[Motor_Thumble].Direction = Direction;
    }
}

void MotorManager_SetSpeed(uint8_t Mot, uint16_t Speed){
    if(Speed == 0){
        Motor[Mot].Enable = 0;
    }
    else{
        Motor[Mot].Enable = 1;
    }
    Motor[Mot].Speed = Speed;
    
    switch(Mot)
    {
        case Motor_Left:
            LWheelSetSpeed = Speed;
        break;
        case Motor_Right:
            RWheelSetSpeed = Speed;
        break;
        case Motor_Thumble:
            AugSetSpeed = Speed;
        break;
        default:
        break;
    }
}

void MotorManager_SetStepSpeed(uint8_t Mot, uint16_t Speed){
    Motor[Mot].StepSpeed = Speed;
}

void MotorManager_SetStepDirection(uint8_t Mot, uint8_t dir){
    Motor[Mot].StepDirection = dir;
}

uint8_t MotorManager_GetStepDirection(uint8_t Mot){
    return Motor[Mot].StepDirection;
}

uint16_t MotorManager_GetStepSpeed(uint8_t Mot){
    return Motor[Mot].StepSpeed;
}

uint16_t MotorManager_GetSpeed(uint8_t Mot){
    return Motor[Mot].Speed;
}

int32_t MotorManager_GetRotationCount(MotorName Mot){
    return Motor[Mot].Rotation_Count;
}

// Needed for dev data display
int32_t MotorManager_GetRotationCountPositive(MotorName Mot){
    return Motor[Mot].Rotation_Count_Positive;
}

void MotorManager_SetRotationCount(MotorName Mot, int32_t RotationCount){
    Motor[Mot].Rotation_Count = RotationCount;
}

void MotorManager_SetPositionCount(MotorName Mot, uint16_t Count){
    Motor[Mot].Position_Count = Count;
}

uint16_t MotorManager_GetPositionCount(MotorName Mot){
    return Motor[Mot].Position_Count;
}

void MotorManager_SetCurrent(MotorName Mot, int16_t Current){
    Motor[Mot].Current = Current;
}

uint16_t MotorManager_GetCurrent(MotorName Mot){
    return Motor[Mot].Current;
}

bool MotorManager_IsAnyMotorEnabled()
{
    bool RetFlag = false;
    if( Motor[Motor_Right].Enable == true ||
        Motor[Motor_Left].Enable == true  || 
        Motor[Motor_Thumble].Enable == true ||
        Motor[Motor_Lift].Enable == true ||
        Motor[Motor_Belt1].Enable == true ||
        Motor[Motor_Belt2].Enable == true     
      )
        
    {
       RetFlag = true;
    }

    return RetFlag;
}

bool MotorManager_IsMotorEnabled(MotorName Mot){
    return Motor[Mot].Enable;
}

 void MotorManager_SetDefaultSpeed(void){
    Motor[Motor_Left].Speed = DEFAULT_SPEED;
    Motor[Motor_Right].Speed = DEFAULT_SPEED;
    Motor[Motor_Thumble].Speed = DEFAULT_SPEED_THUMBLE;
    Motor[Motor_Lift].Speed = DEFAULT_SPEED_LIFT;
    Motor[Motor_Belt1].Speed = DEFAULT_SPEED_BELT;
    Motor[Motor_Belt2].Speed = DEFAULT_SPEED_BELT;
 }

 void MotorManager_ResetRotationCount(MotorName Mot){
         Motor[Mot].Rotation_Count = 0;
 }
 
 // Needed for dev data display
  void MotorManager_ResetRotationCountPositive(MotorName Mot){
         Motor[Mot].Rotation_Count_Positive = 0;
 }