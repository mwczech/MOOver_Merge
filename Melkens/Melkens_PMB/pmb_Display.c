/*
 * File:   pmb_Display.c
 * Author: Jaca
 *
 * Created on March 2, 2023, 8:00 PM
 */


#include "xc.h"
#include "pmb_CAN.h"
#include "pmb_Display.h"
#include "IMUHandler/IMUHandler.h"
#include "TimeManager/TimeManager.h"
#include "BatteryManager/BatteryManager.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/uart2.h"
#include "DiagnosticsHandler.h"
#include "mcc_generated_files/pin_manager.h"
#include <string.h>
#include <stdio.h>
#include "mcc_generated_files/can1.h"
#include "mcc_generated_files/can_module_features.h"
#include "pmb_Functions.h"
#include "pmb_MotorManager.h"
#include "pmb_RouteManager.h"
#include "pmb_System.h"
#include "IMUHandler/IMUHandler.h"
#include "AnalogHandler/AnalogHandler.h"
#include "Tools/Tools.h"
#include "pmb_Scheduler.h"
#include "RoutesDataTypes.h"
#include "pmb_Settings.h"
#include "BatteryManager/BatteryManager.h"

#define dACK_OFFSET 4
#define dPAYLOAD_OFFSET 10

typedef enum HMIDotPic_t{
    HMIDotPic_Green = 50,
    HMIDotPic_Blank = 51,
    HMIDotPic_Red = 52,

}HMIDotPic;

uint8_t up_flag, down_flag;

extern uint8_t CurrentRouteStep;
extern int32_t CalulatedAngle;
extern RouteData CurrentRoute; 
int32_t AngleS;
int16_t TestAngle;
float ImportAngle;
uint16_t AngleInt;
uint16_t AngleFraction;
uint8_t AngleSign;
volatile uint8_t RepeatSendRouteStep;
DiagnosticsEvent_t EventToDisplay;
bool ActiveMagnets[32];

const uint16_t DotPicPosition[32] = {9,43,75,108,141,173,205,239,272, 305, 338, 371, 404, 437, 469, 503, 536, 569, 602, 634, 667, 700, 733, 766, 798, 832, 865, 898, 930, 964, 996};

void floatToUint16(float num, uint16_t *integerPart, uint16_t *decimalPart, uint8_t *sign);
void UpdateSchedulerDisplay(void);

uint16_t BateryVoltage, RailCurrent;

typedef enum SendingSteps_t{
    Step_StatSwitch1 = 0,
    Step_StatSwitch2,
    Step_StatSwitch3,
    Step_StatSwitch4,
    Step_StatSwitch5,
    Step_StatSwitch6,
    Step_StatSwitch7,
    Step_StatSwitch8,
    Step_HSStatus,
    Step_LSStatus,
    Step_StatCharger,
    Step_CurrentSensorRough,
    Step_CurrentSensorA,
    Step_BatteryVoltageRough,
    Step_BatteryVoltageV,
    Step_LeftRot,
    Step_RightRot,
    Step_EnablePowerButton,
    Step_EnableChargerButton,
    Step_ImuAngle,
    Step_ImuAngleFB,
    Step_ImuAngleSign,  
    Step_RouteStep,
    Step_DebugEvent,
    Step_BatteryWarning,
    SendingSteps_NumOf        
}SendingSteps;

/* TODO: Move it to const or find better way to store this patterns */
uint8_t EMPTY[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
uint8_t SwStat_ASCII[] = "SwStatX=zzzzz";
uint8_t SupStat_ASCII[] = "SupStat=zzzzz";
uint8_t HsStat_ASCII[] = "HsStat=zzzzz";
uint8_t LsStat_ASCII[] = "LsStat=zzzzz";
uint8_t ChStat_ASCII[] = "ChStat=zzzzz";
uint8_t Ir_ASCII[] = "Ir=zzzz";
uint8_t Iv_ASCII[] = "Iv=zzzz";
uint8_t Ubatr_ASCII[] = "Ubatr=zzzzz";
uint8_t Ubatv_ASCII[] = "Ubatv=zzzzz";
uint8_t LeftRot_ASCII[] = "LeftRot=zzzzz";
uint8_t RightRot_ASCII[] = "RightRot=zzzz";
uint8_t PowerButStat_ASCII[] = "PowBut=zzzzz";
uint8_t ChargButStat_ASCII[] = "ChaBut=zzzzz";
uint8_t ImuAngle_ASCII[] = "ImuAng=zzzzz";
uint8_t ImuAngFB_ASCII[] = "ImuAFB=zzzzz";
uint8_t ImuSign_ASCII[] = "ImuSGN=zzzzz";
uint8_t RouteStep_ASCII[] = "RouteStep=zzzzz";
uint8_t DebugEvent_ASCII[] = "DBG=0zzzzzzzzzzz";
uint8_t PicFrame_ASCII[] = "pic x,y,IDzzzzzzz";
uint8_t BatteryWarning_ASCII[] = "page 8zzzzzzz";
uint8_t TimDays_ASCII[] = "TimxDays.val=xxxxxx";
uint8_t HourStart_ASCII[] = "HourStartx=xxxxxxxxx";
uint8_t MinuteStart_ASCII[] = "MinuteStartx=xxxxxxx";
uint8_t SWTimer_ASCII[] = "swx.val=xxxxxxxxxxxx";
uint8_t SelectedRoute_ASCII[] = "cbx.val=xxxxxxxxxxxx";
        
uint8_t TransmitBufferUART2[] = "xxxxxxxxxxxxxxxxxxxx";

uint8_t DebugStatus_ASCII[] = "DBG=xxxxxxx";
uint8_t LeftEnco_ASCII[] = "LEnco=xxxxxxx";
uint8_t RightEnco_ASCII[] = "REnco=xxxxxxx";


uint8_t HMI_Ack[] = {0x1A, 0xFF, 0xFF, 0xFF};
uint8_t HMI_FirstCharAck= 0x1A;


bool Transferred_bool;
uint16_t Transferred_uint16_t;
uint16_t RouteStepSelected = 0xFF;


uint8_t Cmd_End[3] = {0xFF, 0xFF, 0xFF};
uint8_t NumberMessageOffset;
volatile uint16_t MotorSpeed;
uint8_t readBuffer_UI[30];
uint8_t RemainingBytes = 0;
uint8_t readBuffer_UI_RemainingBytes[10];
SendingSteps CurrentSendDataStep;
DisplayButton DisplayGlobalEvent = Display_Released;
bool IsBrokenFrame = false;
uint8_t ByteNum = 0;
uint8_t CurrrentByte = 0;
uint16_t CurrentSliderValThumble;

uint16_t BatteryLevelCounter = 0;
uint16_t RotR = 0, RotL = 0;
uint8_t input, output;

void Read_Data_Display(void){
   /*READ DATA FROM KEYBOARD (UART1)*/
    

    
    if( IsBrokenFrame ){
     /* Get remaining bytes from broken frame and place it in proper place in array */

        UART2_ReadBuffer(&readBuffer_UI[CurrrentByte], RemainingBytes);
        
        ByteNum = CurrrentByte + RemainingBytes;
        CurrrentByte = 0;
        IsBrokenFrame = false;
    }
    else{
        /* No broken frame, all frames has been evaluated, start from 0 position */
        CurrrentByte = 0;
        ByteNum = 0;
        memcpy(&readBuffer_UI[ByteNum], &EMPTY[0], 30);
        ByteNum = UART2_ReadBuffer(&readBuffer_UI[0], 30);
    }
        
    while(ByteNum > 0){
        /* Go through entire buffer and check for ACK frames */
        if( !memcmp(&readBuffer_UI[CurrrentByte], &HMI_Ack[0], sizeof(HMI_Ack)) ){
            /* Found ACK, skip 4 bytes */
            ByteNum -= dACK_OFFSET;
            CurrrentByte += dACK_OFFSET;
        }
        else{
            /* Not ACK, data received */
            /* Check if not corrupted - length equal 10 */
            if ( ByteNum >= dPAYLOAD_OFFSET){


                switch (readBuffer_UI[CurrrentByte]){
                    case 'U':
                        if( readBuffer_UI[CurrrentByte + 1] == '1' ){
                            
                            MotorSpeed = /*GEAR_SHIFT_N * */ (readBuffer_UI[CurrrentByte + 3] << 8 | readBuffer_UI[CurrrentByte + 2]);
                            MotorManager_SetSpeed(Motor_Thumble , MotorSpeed);
                            CurrentSliderValThumble = MotorSpeed;
                            DisplayGlobalEvent = Display_SLIDER_THUMBLE;
                        }
                        else if( readBuffer_UI[CurrrentByte + 1] == '2' ){
                            MotorSpeed =/* GEAR_SHIFT_N * */(readBuffer_UI[CurrrentByte + 3] << 8 | readBuffer_UI[CurrrentByte + 2]);
                            
                            MotorManager_SetSpeed(Motor_Left, MotorSpeed);
                            MotorManager_SetSpeed(Motor_Right, MotorSpeed);
                            
                            DisplayGlobalEvent = Display_SLIDER_WHEELS;     
                        }
//                        else if( readBuffer_UI[CurrrentByte + 1] == '3'  ){
//                            MotorSpeed = GEAR_SHIFT_N * (readBuffer_UI[CurrrentByte + 3] << 8 | readBuffer_UI[CurrrentByte + 2]);
//                            MotorManager_SetSpeed(dRightMotor, MotorSpeed);
//                            DisplayGlobalEvent = Display_SLIDER_RIGHT;                       
//                        }
                        break;
                    case 'S':
                        if( readBuffer_UI[CurrrentByte + 1] == '1'  ){
                            DisplayGlobalEvent =  Display_UP;
                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '2'){
                            DisplayGlobalEvent  = Display_LEFT;
                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '3'){
                            DisplayGlobalEvent  = Display_RIGHT;
                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '4'){
                            DisplayGlobalEvent = Display_DOWN;
                        }
                        break;
                    case 'E':
                        if( readBuffer_UI[CurrrentByte + 1] == 'S' ){
                            DisplayGlobalEvent = Display_EMERGANCY_STOP;
                        }
                        if( readBuffer_UI[CurrrentByte + 1] == '0' ){
                            DisplayGlobalEvent = Display_PLAY;
                        }
                        if( readBuffer_UI[CurrrentByte + 1] == '1' ){
                            DisplayGlobalEvent = Display_PAUSE;
                            Diagnostics_SetEvent(dDebug_Pause);
                        }
                        break;
                    case 'B':
                        if( readBuffer_UI[CurrrentByte + 1] == '1'  ){
                            if(readBuffer_UI[CurrrentByte + 2] == 0x01){
                                DisplayGlobalEvent = Display_ENABLE_POWER;
                            }
                            else if( readBuffer_UI[CurrrentByte + 2] == '3' ){
                                DisplayGlobalEvent = Display_BARREL_FORWARD;
                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == '4'){
                                DisplayGlobalEvent = Display_BARREL_STOP;

                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == '5'){
                                DisplayGlobalEvent = Display_BARREL_REVERSE;

                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == '6'){
                                DisplayGlobalEvent = Display_ROUTE_A;

                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == '7'){
                                DisplayGlobalEvent = Display_ROUTE_B;

                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == '8'){
                                DisplayGlobalEvent = Display_ROUTE_C;

                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == '9'){
                                DisplayGlobalEvent = Display_ROUTE_D;

                            }

                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '2' ){
                            if(readBuffer_UI[CurrrentByte + 2] == '0'){
                                RouteStepSelected = readBuffer_UI[CurrrentByte + 4];                            
                                RouteManager_SetStepRequest(RouteStepSelected);
                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == 0x01){
                                DisplayGlobalEvent = Display_DISABLE_POWER; 
                            }
                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '3' ){
                            DisplayGlobalEvent = Display_ENABLE_CHARGER;
                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '4' ){
                            DisplayGlobalEvent = Display_DISABLE_CHARGER;
                        }
                        break;
                    case 'X':
                    #if COMPILE_SWITCH_MOONION    
                     
                        if( readBuffer_UI[CurrrentByte + 1] == '1'  ){
                            if(readBuffer_UI[CurrrentByte + 2] == '3'){
                                    DBG1_SetHigh();     //Motor Down
                                    DBG2_SetLow();
                            }
                            else if( readBuffer_UI[CurrrentByte + 2] == '2' ){
                                    DBG1_SetLow();      //Motor Up
                                    DBG2_SetHigh();
                            }   
                            else if( readBuffer_UI[CurrrentByte + 2] == '1' ){
                                    DBG1_SetLow();      //Motor stop
                                    DBG2_SetLow();
                            }   
                            if(readBuffer_UI[CurrrentByte + 2] == '5'){
                                    DisplayGlobalEvent = Display_UPPER_BELT_ON;   //upper belt on
                            }
                            else if( readBuffer_UI[CurrrentByte + 2] == '4' ){
                                    DisplayGlobalEvent = Display_UPPER_BELT_OFF;      //upper belt off
                            }   
                            if(readBuffer_UI[CurrrentByte + 2] == '7'){
                                    DisplayGlobalEvent = Display_LOWER_BELT_ON;     //lower belt on
                            }
                            else if( readBuffer_UI[CurrrentByte + 2] == '6' ){
                                    DisplayGlobalEvent = Display_LOWER_BELT_OFF;      //lower belt off
                            } 
                            else if( readBuffer_UI[CurrrentByte + 2] == '8' ){//upper belt slider
                                    MotorSpeed =(readBuffer_UI[CurrrentByte + 4] << 8 | readBuffer_UI[CurrrentByte + 3]);
                            
                                    MotorManager_SetSpeed(Motor_Belt1, MotorSpeed);

                                    DisplayGlobalEvent = Display_SLIDER_UPPER_BELT;          
                            }   
                            else if( readBuffer_UI[CurrrentByte + 2] == '9' ){//lover <3 belt slider
                                    MotorSpeed =(readBuffer_UI[CurrrentByte + 4] << 8 | readBuffer_UI[CurrrentByte + 3]);
                            
                                    MotorManager_SetSpeed(Motor_Belt2, MotorSpeed);

                                    DisplayGlobalEvent = Display_SLIDER_LOWER_BELT;          
                            }   
                        }
                        else if(readBuffer_UI[CurrrentByte + 1] == '2' ){
                            if(readBuffer_UI[CurrrentByte + 2] == '0'){
                               
                            }
                            else if(readBuffer_UI[CurrrentByte + 2] == 0x01){
                                
                            }
                        }
                    #else//if moover is selected drive lift motor
                        if( readBuffer_UI[CurrrentByte + 1] == '1'  ){
                            if(readBuffer_UI[CurrrentByte + 2] == '3'){
                                    DisplayGlobalEvent = Display_Lift_UP;
                            }
                            else if( readBuffer_UI[CurrrentByte + 2] == '2' ){
                                   DisplayGlobalEvent = Display_Lift_DOWN;
                            }   
                            else if( readBuffer_UI[CurrrentByte + 2] == '1' ){
                                    DisplayGlobalEvent = Display_Lift_STOP;
                            }   
                        }
                    #endif
                        break;  
                    case 'T':
                        Scheduler_SetCurrentTime(readBuffer_UI[2], readBuffer_UI[3],readBuffer_UI[4]);
                        break;
                    case 'F':
                        if( readBuffer_UI[1] == '9' ){
                            /* Received save to flash schedule button */
                            Scheduler_SaveToFlash();
                        }
                        break;
                    case 'Y':
                        if( readBuffer_UI[1] == 'E'
                        &&  readBuffer_UI[2] == 'N'      
                        &&  readBuffer_UI[3] == 'T'){
                            /* Scheduler site clicked on display */
                            UpdateSchedulerDisplay();
                        }
                        else 
                            if( readBuffer_UI[1] != 0x00 ){
                                Route_ID routeSelected;        
                                if( readBuffer_UI[2] == 0x00 ){
                                    /* If selected route value is 0 - clear timer */
                                    routeSelected = readBuffer_UI[1] - 1;
                                    Scheduler_DisableSchedule( routeSelected );
                                }
                                else{
                                    Time StartTime;
                                    TimerName selectedTimer;
                                    uint8_t days;
                                    days = readBuffer_UI[3];
                                    selectedTimer = readBuffer_UI[1] - 1;
                                    /* Display send routeA as value 1, therefore routeSelected should be icremented by 1 */
                                    routeSelected = readBuffer_UI[2];
                                    if(routeSelected == 0){
                                        /* Error - route has not been selected */
                                        routeSelected = Route_NumOf;
                                    }
                                    else{
                                        routeSelected = routeSelected - 1;
                                    }
                                    StartTime.Hour = readBuffer_UI[4];
                                    StartTime.Minute = readBuffer_UI[5];

                                    Scheduler_SetSchedule(StartTime,selectedTimer,routeSelected,days);
                                }

                            }else{
                                /* timer byte is zero - clear schedule request from display */
                                
                            }
                        output = 0;
                        input = readBuffer_UI[3];
                        output = reverseBits(input);
                        __asm("nop");
                        //Scheduler_SetSchedule()
                        break;
                    default:
                        __asm("nop");
                        break;
                        
                } 
                ByteNum -= dPAYLOAD_OFFSET;
                CurrrentByte += dPAYLOAD_OFFSET;            
            }
            
            else{
                    /* There might be broken frame with proper data - analyse it due to poor HMI robustness (only one frame when button pressed)*/
                    if(readBuffer_UI[CurrrentByte] >= 0x40 && readBuffer_UI[CurrrentByte] <= 0x55){
                        IsBrokenFrame = true;
                        RemainingBytes = dPAYLOAD_OFFSET - ByteNum;
                        CurrrentByte += dPAYLOAD_OFFSET - RemainingBytes;
                        break;
                    }
                    else if(!memcmp(&readBuffer_UI[CurrrentByte], &HMI_Ack[0], ByteNum)) {
                        /* Use case - broken ACK frame at the end of input */
                        IsBrokenFrame = true;
                        RemainingBytes = dACK_OFFSET - ByteNum;
                        CurrrentByte += dACK_OFFSET - RemainingBytes;

                        break;
                    }
                    
                    break;
                    
            }
        }
        
    }
    
}

void Display_SendMagnetPicRequest(uint16_t index, HMIDotPic pic){

            memcpy(&TransmitBufferUART2[0], &PicFrame_ASCII, 4);  /* 'pic ' */
            NumberMessageOffset = Tools_ITOAu16(DotPicPosition[index], &TransmitBufferUART2[4]); /* 'pic x' */
            TransmitBufferUART2[4+NumberMessageOffset] = (uint8_t)',';
            TransmitBufferUART2[5+NumberMessageOffset] = (uint8_t)'4';
            TransmitBufferUART2[6+NumberMessageOffset] = (uint8_t)'9';
            TransmitBufferUART2[7+NumberMessageOffset] = (uint8_t)'6';
            TransmitBufferUART2[8+NumberMessageOffset] = (uint8_t)',';  /* 'pic x,y,' */
            Tools_ITOAu16(pic, &TransmitBufferUART2[9+NumberMessageOffset]); /* 'pic x,y,ID' */

            memcpy(&TransmitBufferUART2[11+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 11+NumberMessageOffset+3);
}   

void Display_SendData(void){
    MagnetsStatus Magnets = GetMagnets();

    for(int i=0; i<32; i++){
        if(isBitSet(Magnets.status, i)){
            if( ActiveMagnets[i] ){
                /* Do nothing, active magnet has been sent already */
            }
            else{
                /* Magnet has not been sent yet */
                ActiveMagnets[i] = true;
                if(i == 15){
                    Display_SendMagnetPicRequest(i, HMIDotPic_Green);
                }
                else{
                    Display_SendMagnetPicRequest(i, HMIDotPic_Red);                    
                }

            }
        }
        else{
            /* Magnet not active, check if pic should be overwriten */
            if( ActiveMagnets[i] ){
                ActiveMagnets[i] = false;
                Display_SendMagnetPicRequest(i, HMIDotPic_Blank); 
            }
        }
    }

    ImportAngle = IMUHandler_GetAngle();
    
    //Get rotation counts
    RotR = MotorManager_GetRotationCountPositive(Motor_Right) * dDISTANCE_PER_MOTOR_ROTATION;
    RotL = MotorManager_GetRotationCountPositive(Motor_Left) * dDISTANCE_PER_MOTOR_ROTATION;
    
    switch(CurrentSendDataStep){
        //1 Left Wheel Set Speed
        case Step_StatSwitch1:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '1';
            NumberMessageOffset = Tools_ITOAu16(LWheelSetSpeed, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        //2 Right Wheel Set Speed
        case Step_StatSwitch2:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '2';
            NumberMessageOffset = Tools_ITOAu16(RWheelSetSpeed, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);   
        break;
        //3 Auger Set Speed
        case Step_StatSwitch3:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '3';
            NumberMessageOffset = Tools_ITOAu16(AugSetSpeed, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        //4 Current step distance driven left wheel (Step DC)
        case Step_StatSwitch4:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '4';
            NumberMessageOffset = Tools_ITOAu16(RotL, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        //5 Current step distance driven right wheel (Step DC)
        case Step_StatSwitch5:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '5';
            NumberMessageOffset = Tools_ITOAu16(RotR, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        // L Distance driven in last step (saved until the next step finishes)
        case Step_StatSwitch6:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '6';
            NumberMessageOffset = Tools_ITOAu16(LastRotL, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        //R Distance driven in last step (saved until the next step finishes)
        case Step_StatSwitch7:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '7';
            NumberMessageOffset = Tools_ITOAu16(LastRotR, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        // Angle delta
        case Step_StatSwitch8:
            memcpy(&TransmitBufferUART2[0], &SwStat_ASCII, 8);
            TransmitBufferUART2[6] = '8';
            NumberMessageOffset = Tools_ITOAu16(IntStepAngle, &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
        break;
        //
        case Step_LSStatus:

        break;
        //
        case Step_StatCharger:

        break;
        // Current ADC direct reading
        case Step_CurrentSensorRough:
            memcpy(&TransmitBufferUART2[0], &Ir_ASCII, 3);  
            NumberMessageOffset = Tools_ITOAu16(AnalogHandler_GetADCRough(IM_SENSE), &TransmitBufferUART2[3]);
            memcpy(&TransmitBufferUART2[3+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 3+NumberMessageOffset+3);
        break;
        // Current current
        case Step_CurrentSensorA:
            CalculateAnalogRealValues();
            memcpy(&TransmitBufferUART2[0], &Iv_ASCII, 3);         
            NumberMessageOffset = Tools_ITOAu16(RailCurrent, &TransmitBufferUART2[3]);
            memcpy(&TransmitBufferUART2[3+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 3+NumberMessageOffset+3);       
        break;
        // Voltage ADC direct reading
        case Step_BatteryVoltageRough:
            memcpy(&TransmitBufferUART2[0], &Ubatr_ASCII, 6);
            NumberMessageOffset = Tools_ITOAu16(AnalogHandler_GetADCRough(BAT_STATUS), &TransmitBufferUART2[6]);
            memcpy(&TransmitBufferUART2[6 + NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 6+NumberMessageOffset+3);
        break;
        // Current Voltage
        case Step_BatteryVoltageV:
            CalculateAnalogRealValues();
            memcpy(&TransmitBufferUART2[0], &Ubatv_ASCII, 6);  
            NumberMessageOffset = Tools_ITOAu16(BateryVoltage, &TransmitBufferUART2[6]);
            memcpy(&TransmitBufferUART2[6 + NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 6+NumberMessageOffset+3);
        break;       
        case Step_LeftRot:
            memcpy(&TransmitBufferUART2[0], &LeftRot_ASCII, 8);  
            NumberMessageOffset = Tools_ITOAu16(MotorManager_GetRotationCount(Motor_Left), &TransmitBufferUART2[8]);
            memcpy(&TransmitBufferUART2[8+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 8+NumberMessageOffset+3);
            break;
        case Step_RightRot:
            memcpy(&TransmitBufferUART2[0], &RightRot_ASCII, 9);  
            NumberMessageOffset = Tools_ITOAu16(MotorManager_GetRotationCount(Motor_Right), &TransmitBufferUART2[9]);
            memcpy(&TransmitBufferUART2[9+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 9+NumberMessageOffset+3);
            break;     

        case Step_EnablePowerButton:
            Transferred_bool = System_GetPowerRailState();
            Transferred_uint16_t = (Transferred_bool) ? 1 : 0;
            memcpy(&TransmitBufferUART2[0], &PowerButStat_ASCII, 7);            
            NumberMessageOffset = Tools_ITOAu16(Transferred_uint16_t, &TransmitBufferUART2[7]);
            memcpy(&TransmitBufferUART2[7+NumberMessageOffset], &Cmd_End, 3);  
            UART2_WriteBuffer(&TransmitBufferUART2[0], 7+NumberMessageOffset+3); 
            break;

        case Step_EnableChargerButton:
            Transferred_bool = System_GetPowerRailState();
            Transferred_uint16_t = (Transferred_bool) ? 1 : 0;
            memcpy(&TransmitBufferUART2[0], &ChargButStat_ASCII, 7);            
            NumberMessageOffset = Tools_ITOAu16(Transferred_uint16_t, &TransmitBufferUART2[7]);
            memcpy(&TransmitBufferUART2[7+NumberMessageOffset], &Cmd_End, 3);  
            UART2_WriteBuffer(&TransmitBufferUART2[0], 7+NumberMessageOffset+3); 
            break;
            
        case Step_ImuAngle:
            ImportAngle = IMUHandler_GetAngle();
            floatToUint16(ImportAngle, &AngleInt, &AngleFraction, &AngleSign);
            memcpy(&TransmitBufferUART2[0], &ImuAngle_ASCII, 7);            
            NumberMessageOffset = Tools_ITOAu16(AngleInt, &TransmitBufferUART2[7]);
            memcpy(&TransmitBufferUART2[7+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 7+NumberMessageOffset+3);    
            break;
            
        case Step_ImuAngleFB:
            memcpy(&TransmitBufferUART2[0], &ImuAngFB_ASCII, 7);            
            NumberMessageOffset = Tools_ITOAu16(AngleFraction, &TransmitBufferUART2[7]);
            memcpy(&TransmitBufferUART2[7+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 7+NumberMessageOffset+3);    
            break;
            
        case Step_ImuAngleSign:
            memcpy(&TransmitBufferUART2[0], &ImuSign_ASCII, 7);            
            NumberMessageOffset = Tools_ITOAu16((uint16_t)AngleSign, &TransmitBufferUART2[7]);
            memcpy(&TransmitBufferUART2[7+NumberMessageOffset], &Cmd_End, 3);
            UART2_WriteBuffer(&TransmitBufferUART2[0], 7+NumberMessageOffset+3);    
            break;
            
        case Step_RouteStep:
            if(RouteManager_GetCurrentRouteStep() != dROUTE_IDLE){
                memcpy(&TransmitBufferUART2[0], &RouteStep_ASCII, 10);            
                NumberMessageOffset = Tools_ITOAu16(CurrentRouteStep, &TransmitBufferUART2[10]);
                memcpy(&TransmitBufferUART2[10+NumberMessageOffset], &Cmd_End, 3);
                UART2_WriteBuffer(&TransmitBufferUART2[0], 10+NumberMessageOffset+3);
                RepeatSendRouteStep = 5;   
            }
            else{
                if(RepeatSendRouteStep > 0){
                    RepeatSendRouteStep -= 1;
                    memcpy(&TransmitBufferUART2[0], &RouteStep_ASCII, 10);            
                    NumberMessageOffset = Tools_ITOAu16(CurrentRouteStep, &TransmitBufferUART2[10]);
                    memcpy(&TransmitBufferUART2[10+NumberMessageOffset], &Cmd_End, 3);
                    UART2_WriteBuffer(&TransmitBufferUART2[0], 10+NumberMessageOffset+3);
                }
            }
            break;
        case Step_DebugEvent:
            break;
        case Step_BatteryWarning:
            if( BatteryManager_GetBatteryLevel() == BatteryLevel_Low ){
                if( BatteryLevelCounter < 10000 ){
                    BatteryLevelCounter ++;
                }
                else{
                    BatteryLevelCounter = 0;
                    /* Set page 8 on display to indicate warning*/
                    memcpy(&TransmitBufferUART2[0], &BatteryWarning_ASCII, 6);            
                    memcpy(&TransmitBufferUART2[6], &Cmd_End, 3);
                    UART2_WriteBuffer(&TransmitBufferUART2[0], 9);
                }
            }
            else{
                BatteryLevelCounter = 0;
            }
            break;
        default:            
            break;
    }
    CurrentSendDataStep++;
    if(SendingSteps_NumOf==CurrentSendDataStep){
        CurrentSendDataStep = Step_StatSwitch1;
    }
}

void UpdateSchedulerDisplay(void){
    TimerName i;
    uint8_t days;
    uint16_t startHour, startMinute, selectedRoute;
    Scheduler CurrentSchedule;
    for(i=0; i<dTimer_NumOf; i++){
        CurrentSchedule = Scheduler_GetSchedule(i);
        /* Iterate 4 timers and send settings to display  */

        /* Prepare days variable */
        memcpy(&TransmitBufferUART2[0], &TimDays_ASCII, 13);
        /* Put timer number in 4th byte of TimxDays */
        TransmitBufferUART2[3] = (uint8_t)i + '0';
        days = CurrentSchedule.days;
        NumberMessageOffset = Tools_ITOAu16((uint16_t)days, &TransmitBufferUART2[13]);
        memcpy(&TransmitBufferUART2[13+NumberMessageOffset], &Cmd_End, 3);
        UART2_WriteBuffer(&TransmitBufferUART2[0], 13+NumberMessageOffset+3);  

        /* Prepare hour variable */
        memcpy(&TransmitBufferUART2[0], &HourStart_ASCII, 11);
        /* Put timer number in 10th byte of HourStartx */
        TransmitBufferUART2[9] = (uint8_t)i + 1 + '0';
        startHour = CurrentSchedule.startTime.Hour;
        NumberMessageOffset = Tools_ITOAu16((uint16_t)startHour, &TransmitBufferUART2[11]);
        memcpy(&TransmitBufferUART2[11+NumberMessageOffset], &Cmd_End, 3);
        UART2_WriteBuffer(&TransmitBufferUART2[0], 11+NumberMessageOffset+3);

         /* Prepare minute variable */
        memcpy(&TransmitBufferUART2[0], &MinuteStart_ASCII, 13);
        /* Put timer number in 12th byte of MinuteStartx */
        TransmitBufferUART2[11] = (uint8_t)i+1+ '0';
        startMinute = CurrentSchedule.startTime.Minute;
        NumberMessageOffset = Tools_ITOAu16((uint16_t)startMinute, &TransmitBufferUART2[13]);
        memcpy(&TransmitBufferUART2[13+NumberMessageOffset], &Cmd_End, 3);
        UART2_WriteBuffer(&TransmitBufferUART2[0], 13+NumberMessageOffset+3);

        /* Prepare selected route variable */
        memcpy(&TransmitBufferUART2[0], &SelectedRoute_ASCII, 8);
        /* Put timer number in 3rd byte of MinuteStartx */
        TransmitBufferUART2[2] = (uint8_t)i + '0';
        selectedRoute = CurrentSchedule.routeName;
        if( selectedRoute == Route_NumOf ){
            selectedRoute = 0;
        }
        else{
            selectedRoute = selectedRoute + 1;
        }
        TransmitBufferUART2[8] = selectedRoute + '0';
        memcpy(&TransmitBufferUART2[9], &Cmd_End, 3);
        UART2_WriteBuffer(&TransmitBufferUART2[0], 12);

        /* Send enable SW */
        memcpy(&TransmitBufferUART2[0], &SWTimer_ASCII, 8);
        TransmitBufferUART2[2] = (uint8_t)i+ '0';
        if( CurrentSchedule.enabled ){
            TransmitBufferUART2[8] = '1';
        }
        else{
            TransmitBufferUART2[8] = '0';
        }
        memcpy(&TransmitBufferUART2[9], &Cmd_End, 3);
        UART2_WriteBuffer(&TransmitBufferUART2[0], 12);
        

    }
}

void Display_ClearEvent(void){
   DisplayGlobalEvent = Display_Released;

}

DisplayButton Display_GetEvent(void){
   return DisplayGlobalEvent;
}

void CalculateAnalogRealValues(void)
{
    uint16_t RealVoltage, RealCurrent;
    RealVoltage = AnalogHandler_GetADCFiltered(BAT_STATUS);
    BateryVoltage = CalculateVoltage(RealVoltage);

    RealCurrent = AnalogHandler_GetADCFiltered(IM_SENSE);
    RailCurrent = CalculateCurrent(RealCurrent);
}

void floatToUint16(float num, uint16_t *integerPart, uint16_t *decimalPart, uint8_t *sign) {
    // Determine the sign
    if (num < 0) {
        *sign = 1; // Negative sign
        num = -num; // Make the number positive for further processing
    } else {
        *sign = 2; // Positive sign
    }

    // Split the float into integer and decimal parts
    float integerFloat = (float)((int)num);
    float decimalFloat = num - integerFloat;

    // Convert the integer part to uint16_t
    *integerPart = (uint16_t)integerFloat;

    // Convert the decimal part to uint16_t
    *decimalPart = (uint16_t)(decimalFloat * 10000); // Multiply by 10000 to retain four decimal places
}

