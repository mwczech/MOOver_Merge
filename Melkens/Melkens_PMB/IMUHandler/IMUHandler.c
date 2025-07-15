#include <stdint.h>
#include <stdio.h>
#include "string.h"
#include "IMUHandler.h"
#include "../Tools/Timer.h"
#include "../pmb_Settings.h"
#include "../pmb_Functions.h"
#include "../DmaController/DmaController.h"
#include "../mcc_generated_files/uart3.h"
#include "../mcc_generated_files/pin_manager.h"
#include "../pmb_System.h"
#include "../AnalogHandler/AnalogHandler.h"
#include "../DiagnosticsHandler.h"
#include "../pmb_MotorManager.h"
#include "../Melkens_Lib/Types/MessageTypes.h"
#include "../Melkens_Lib/CRC16/CRC16.h"
#include "../RoutesDataTypes.h"

#define ENCODER_ONE_WHEEL_LEN 2
#define ENCODER_LEFT_BEGIN 4
#define ENCODER_RIGHT_BEGIN ENCODER_LEFT_BEGIN + ENCODER_ONE_WHEEL_LEN + 1

#define PI 3.14159265359

//static char GetEncoderDataMessage[8] = "GET_ENCO";

Pmb2ImuFrame_t Pmb2ImuFrame;
Imu2PmbFrame_t Imu2PmbFrame;

bool IsInitialized = false;
bool IsDataSend = false;
bool IsRequestReceived = false;

volatile static int16_t Roll,Pitch,Yaw,Ahrs_X,Ahrs_Y;

typedef struct IMU_t{
    int16_t Roll;
    int16_t Pitch;
    int16_t Yaw;
    int16_t Ahrs_X;
    int16_t Ahrs_Y;
    MagnetsStatus MagnetBar;
  
}IMU;


typedef struct Encoder
{
	int16_t Left;
	int16_t Right;
	int16_t Left1msTicks;
	int16_t Right1msTicks;

	int16_t Diff;

}Encoder_t;

typedef struct Current
{
    uint16_t OveralCurrent;
    uint16_t ThumbleCurrent;
}Current_t;

Timer IMU_ReceiveTimeout;

int16_t PreviousEncoderLeftTicks;
int16_t PreviousEncoderRightTicks;
uint8_t MagnetsCounters[8];
bool    MagnetsStateAfterDebounce[8];

Encoder_t Encoder;
Current_t CurrentData;

IMU IMUData;
MagnetsStatus PreviousMagnets;
ESP_Message CurrentMessage; //todo: [PM] move it to IMU
RemoteButton ButtonEvent;
uint16_t RouteStepCnt;

//Dummy
int a=0;

void IMUHandler_MessageReceivedHandler(void);
void TranslateESPMessage(void);
void IMUHandler_EmergencyStop(void);

// void IMUHandler_SetEncoderValue(int16_t Value, uint8_t Wheel)
// {
// 	if(LEFT_ENCODER == Wheel)
// 	{
// 		Encoder.Left = Value;
// 	}
// 	else
// 	{
// 		Encoder.Right = Value;
// 	}
// }

// int16_t IMUHandler_GetEncoderValue(uint8_t Wheel)
// {
// 	if(LEFT_ENCODER == Wheel)
// 	{
// 		return Encoder.Left;
// 	}
// 	else if(RIGHT_ENCODER == Wheel)
// 	{
// 		return Encoder.Right;
// 	}
// 	else
// 	{
// 		return 0;
// 	}
// }

void IMUHandler_Process( void )
{
    /* Request received, process request and send message to master */
    //_LATB15 = 1;
    DMA_ChannelEnable(DMA_CHANNEL_0);
    DMACNT0 = 0x10;
    DMACH0bits.CHREQ = 1;
    //_LATB15 = 0;

}

bool IMUHandler_IsInitialized( void )
{
	return IsInitialized;
}

void IMUHandler_Init( void )
{
    /* Set transmit buffer address as source for DMA0 (sending data from) */
    DmaController_SetSourceAddress((uint16_t)&Pmb2ImuFrame,DMA_CHANNEL_0);
    /* Set UART3 TX address as destination for DMA0 (sending data to) */
    /* TODO: pawelton Change fixed address to UART3TX buff */
    DmaController_SetDestinationAddress(0x0F10 ,DMA_CHANNEL_0);
    
    /* Set UART3 RX buffer address as source for DMA1 (sending data from) */
    /* TODO: pawelton Change fixed address to UART3TX buff */
    DmaController_SetSourceAddress(0x0F0C, DMA_CHANNEL_1);
    /* Set receive buffer address as destination for DMA1 (sending data to) */
    DmaController_SetDestinationAddress((uint16_t)&Imu2PmbFrame ,DMA_CHANNEL_1);
    
	//UartHandler_SendMessage(Uart_1, &GetEncoderDataMessage[0], sizeof(GetEncoderDataMessage));
    sprintf(&Pmb2ImuFrame, "ENC:LL;RR;A5");
    DMA_Initialize();
    DMA_TransferCountSet(DMA_CHANNEL_0, sizeof(Pmb2ImuFrame_t));
    DMA_TransferCountSet(DMA_CHANNEL_1, sizeof(Imu2PmbFrame_t));
    U3MODEbits.URXEN = 1;
    U3MODEbits.UTXEN = 1;

     TimerSetCounter(&IMU_ReceiveTimeout, 100);
    //TimerSetCounter(&IMU_ReceiveTimeout, dIMU_NO_COMM_TIMEOUT);    // DMA_ChannelEnable(DMA_CHANNEL_0);

   // 
   // DMACH0bits.CHREQ = 1;
    //DMACH1bits.CHREQ = 1;
}

void IMUHandler_SetOveralCurrent(int16_t OveralCurrent ){
    CurrentData.OveralCurrent = OveralCurrent;
}

void IMUHandler_SetThumbleCurrent(int16_t Current ){
    CurrentData.ThumbleCurrent = Current;
}

void IMUHandler_ReadEncoderValues(uint16_t RightEncoder, uint16_t LeftEncoder ){
    Encoder.Left = LeftEncoder;
    Encoder.Right = RightEncoder;
}

bool isTwoBitSet(uint32_t data, uint8_t position){
    // Create a mask with only the bit at the specified position set
    if( position + 1 == Magnet_NumOf ){
        /* There is no magnet at position + 1 */
        return false;
    }
    else{
        uint32_t mask = (uint32_t)1 << position | (uint32_t)1 << (position + 1);
        // Check if the bit at the specified position is set in the data using bitwise AND
        uint32_t after = (data & mask);
        if ( after == mask) {
            return true; // Bit is set
        } else {
            return false; // Bit is not set
        } 
    }
}

uint8_t MagnetDiscoveredTimerCounter;
bool MagnetDiscovered;
uint8_t MagnetDetectionsNum;

int8_t VirtualSensors[3];
float MagnetsPosition[3];

void ConvertDetectionToVirtualSensor(MagnetName DetectedMagnet, bool IsDoubleSensor, uint8_t DetectionIndex){
    int8_t VirtualSensor;
    VirtualSensor = ((int8_t)DetectedMagnet - dMIDDLE_MAGNET_INDEX) * 2;
    if(IsDoubleSensor){
        VirtualSensor += 1;
    }
    switch(DetectionIndex){
        case 0:
            VirtualSensors[0] = VirtualSensor;
            break;
        case 1:
            VirtualSensors[1] = VirtualSensor;            
            break;
        case 2:
            VirtualSensors[2] = VirtualSensor;
            break;
        default:
            break;
    }
}

float ConvertVirtualSensorToDistance(int8_t VirtualSensor){
    return dMAGNET_BAR_VIRTUAL_STEP * (float)VirtualSensor;
}

void IMUHandler_ProcessMagnetsBar(){
    //If new data differs from previous data
    if(IMUData.MagnetBar.status != PreviousMagnets.status){
        //Clear Virtual Sensor data
         VirtualSensors[0] = 127;
         VirtualSensors[1] = 127;
         VirtualSensors[2] = 127;
         //Reset detection number counter
        MagnetDetectionsNum = 0;
        //If no magnets detected
        if( IMUData.MagnetBar.status == 0){
            MagnetDiscovered = 0;
        }
        //If magnets detected (first magnet sensor event)
        else{
            MagnetDiscovered = true;
        }
    //If new data is the same as previous AND magnets are detected (second magnet sensor event)
    }
    
    /* Perform calculation of magnets location */
    if( MagnetDiscovered ){
        MagnetName Name = Magnet1;
        MagnetDetectionsNum = 0;
        MagnetsPosition[0] = dMAGNET_NO_DETECTION;
        MagnetsPosition[1] = dMAGNET_NO_DETECTION;
        MagnetsPosition[2] = dMAGNET_NO_DETECTION;
        //Loop through all sensors
        while (Name < Magnet_NumOf){
            //Check if sensor's bit is set
            if(isBitSet(IMUData.MagnetBar.status, Name)){
                //Magnet detected in bitfield, check if there is two magnets in a row
                if(isTwoBitSet(IMUData.MagnetBar.status, Name) ){
                   //Two adjacent magnets detected in bitfield
                   //Convert to Virtual Sensors with the double sensor flag "true"
                   ConvertDetectionToVirtualSensor(Name, true, MagnetDetectionsNum);
                   //Convert Virtual Sensor data to physical distance
                   MagnetsPosition[MagnetDetectionsNum] = ConvertVirtualSensorToDistance(VirtualSensors[MagnetDetectionsNum]);
                   //Increment sensor search by 2
                   Name += 2;

                   
                }
                //If only one sensor is active
                else{
                   //Convert without the double sensor flag
                   ConvertDetectionToVirtualSensor(Name, false, MagnetDetectionsNum);
                   //Convert Virtual to Distance
                   MagnetsPosition[MagnetDetectionsNum] = ConvertVirtualSensorToDistance(VirtualSensors[MagnetDetectionsNum]);
                   //Increment sensor index by 1
                    Name += 1;
                }
                //Increment the count of found magnets
                MagnetDetectionsNum += 1;
                
            }
            //If no magnet is detected
            else{
                //Increment loop index
                Name += 1;
            }
        }
        __asm("nop");
    }
    else{
        /* State does not change, magnets are not discovered - do nothing */
        MagnetDetectionsNum = 0;
        MagnetsPosition[0] = dMAGNET_NO_DETECTION;
        MagnetsPosition[1] = dMAGNET_NO_DETECTION;
        MagnetsPosition[2] = dMAGNET_NO_DETECTION;
        
    }
    //Update the previous status for next comparison
    PreviousMagnets.status = IMUData.MagnetBar.status;
}

void IMUHandler_Perform100ms( void )
{
//    MotorManager_SetSpeed(Motor_Right, );
//    MotorManager_StartMotorKeepDirection(Motor_Right);
}

void IMUHandler_Perform1ms( void )
{
    /* Get values from encoder and send them through UART if request received */    
    if(DMA_IsTransferComplete(DMA_CHANNEL_1))
    {
        /* Received data from IMU */
        //IMUHandler_MessageReceivedHandler();
        IMUHandler_ProcessReceivedData();
        // if(IMUData.MagnetBar.status != 0xA5A5A5A5){
        //     IMUHandler_ProcessMagnetsBar();
        //     Diagnostics_SetEvent(dDebug_MagnetsConnected);
        // }
        IMUHandler_MessageReceivedHandler();
        Diagnostics_SetEvent(dDebug_IMUConnected);
        TimerSetCounter(&IMU_ReceiveTimeout,100);
    }
    else{
        TimerTick(&IMU_ReceiveTimeout);
        if( TimerIsExpired(&IMU_ReceiveTimeout) ){
            IMUHandler_EmergencyStop();
            IMUHandler_MessageReceivedHandler();
            TimerSetCounter(&IMU_ReceiveTimeout,100);

            }
    }

    /* Send data with encoders */
    if(U3STAbits.OERR){
        U3MODEbits.UARTEN = 0;
        U3STAbits.OERR = 0;
        U3MODEbits.UARTEN = 1;
        DMA_ChannelEnable(DMA_CHANNEL_1);
    }
}

int16_t IMUHandler_Get1msRotationTics(bool Wheel)
{
	if(Wheel == LEFT_ENCODER)
	{
		return Encoder.Left1msTicks;
	}
	else
	{
		return Encoder.Right1msTicks;
	}
}
uint16_t Low;
uint16_t High;
uint16_t IvBat;
uint16_t MagnetBarStat;
extern uint16_t BateryVoltage;

void IMUHandler_MessageReceivedHandler( void )
{
    /* Check if received message is request for encoder values */
    //if(!strcmp(ReceiveBufferEncoder, "GET_ENCO"))
    if(1)
    {
        /* Process sending after getting encoder values */
        _LATC12 = 1;
       
        Pmb2ImuFrame.motorRightRotation = MotorManager_GetPositionCount(Motor_Right);
        Pmb2ImuFrame.motorLeftRotation = MotorManager_GetPositionCount(Motor_Left);
        Pmb2ImuFrame.batteryVoltage = BateryVoltage;
        Pmb2ImuFrame.adcCurrent = AnalogHandler_GetADCFiltered(IM_SENSE);
        Pmb2ImuFrame.thumbleCurrent = CurrentData.ThumbleCurrent;
        // todo: [PM] update values to transmit

        Pmb2ImuFrame.crc = CRC16((uint8_t*)&Pmb2ImuFrame, sizeof(Pmb2ImuFrame_t) - sizeof(Pmb2ImuFrame.crc));
                
        _LATC12 = 0;

        DMA_ChannelEnable(DMA_CHANNEL_0);
        DMACH0bits.CHREQ = 1;
        
        /* Reset flag to prepare next request reception */
        DMACH1bits.CHEN = 1;
    }
    DMA_ResetTransferStatus(DMA_CHANNEL_1);

}

void IMUHandler_ProcessReceivedData(void){
    
    if(Imu2PmbFrame.crc == CRC16((uint8_t*)&Imu2PmbFrame, sizeof(Imu2PmbFrame_t) - sizeof(Imu2PmbFrame.crc))){
        
//        MotorManager_SetDirection(Motor_Right, R_REV);
//        MotorManager_SetDirection(Motor_Left, L_REV);
        
        if(Imu2PmbFrame.motorRightSpeed>0)
        {
            MotorManager_SetDirection(Motor_Right, R_FOR);
            
            MotorManager_SetSpeed(Motor_Right, Imu2PmbFrame.motorRightSpeed);
            MotorManager_StartMotorKeepDirection(Motor_Right);
            
        }
        else
        {
            MotorManager_SetDirection(Motor_Right, R_REV);
            
            MotorManager_SetSpeed(Motor_Right, abs(Imu2PmbFrame.motorRightSpeed));
            MotorManager_StartMotorKeepDirection(Motor_Right);
            
        }
        
        if(Imu2PmbFrame.motorLeftSpeed>0)
        {
            MotorManager_SetDirection(Motor_Left, L_FOR);
            
            MotorManager_SetSpeed(Motor_Left, Imu2PmbFrame.motorLeftSpeed);
            MotorManager_StartMotorKeepDirection(Motor_Left);
        }
        else
        {
            MotorManager_SetDirection(Motor_Left, L_REV);
            
            MotorManager_SetSpeed(Motor_Left, abs(Imu2PmbFrame.motorLeftSpeed));
            MotorManager_StartMotorKeepDirection(Motor_Left);
        }
        
//        MotorManager_SetSpeed(Motor_Right, Imu2PmbFrame.motorRightSpeed);
//        MotorManager_StartMotorKeepDirection(Motor_Right);
//        MotorManager_SetSpeed(Motor_Left, Imu2PmbFrame.motorLeftSpeed);
//        MotorManager_StartMotorKeepDirection(Motor_Left);
        MotorManager_StartMotor(Motor_Thumble, dRIGHT);
        MotorManager_SetSpeed(Motor_Thumble, Imu2PmbFrame.motorThumbleSpeed);
        MotorManager_StartMotorKeepDirection(Motor_Thumble);
        
        MotorManager_SetSpeed(Motor_Lift, Imu2PmbFrame.motorLiftSpeed);
        MotorManager_SetSpeed(Motor_Belt1, Imu2PmbFrame.motorBelt1Speed);
        MotorManager_SetSpeed(Motor_Belt2, Imu2PmbFrame.motorBelt2Speed);
        
        MotorManager_TriggerEnableMessageSend(0);
        
        //todo: [PM] serve rest of the data
    }
    else{
        Pmb2ImuFrame.crcImu2PmbErrorCount++;
        IMUHandler_EmergencyStop();
    }
    LED3_Toggle();
}

uint8_t IMUHandler_HowManyMagnetsDetected(void){
    return MagnetDetectionsNum;
}

float IMUHandler_GetMagnetMagnetPositionInCM(MagnetPosition Magnet){
    return MagnetsPosition[Magnet];
}

int16_t GetRoll(void){
    return IMUData.Roll;
}

int16_t GetPitch(void){
    return IMUData.Pitch;
}

int16_t GetYaw(void){
    return IMUData.Yaw;
}

int16_t GetAhrsX(void){
    return IMUData.Ahrs_X;
}

int16_t GetAhrsY(void){
    return IMUData.Ahrs_Y;
}

MagnetsStatus GetMagnets(void){
    return IMUData.MagnetBar;
}

float IMUHandler_GetAngle(void){
    int32_t CurrentPitch;
    float RetVal;
    
    CurrentPitch = IMUData.Roll;
    RetVal = CalculateDegreeFromPi(CurrentPitch);
    return RetVal;
}



// Function to calculate the angle between two points on the circle
float IMUHandler_CalculateAngle(float prevDegree, float currentDegree) {
    float angle = currentDegree - prevDegree;

    // Normalize the angle to be between -180 and 180 degrees
    if (angle <= -180) {
        angle += 360;
    } else if (angle > 180) {
        angle -= 360;
    }

    return angle;
}
    uint8_t speed;
    uint8_t route_step;
    uint16_t isV;
void TranslateESPMessage(void){
    switch(CurrentMessage){
        case ESP_Forward:
            ButtonEvent = Remote_UP;
            break;
        case ESP_Reverse:
            ButtonEvent = Remote_DOWN;
            break;        
        case ESP_Right:
            ButtonEvent = Remote_RIGHT;
            break;
        case ESP_Left:
            ButtonEvent = Remote_LEFT;
            break;
        case ESP_Stop:
            ButtonEvent = Remote_Stop;
            break;
        case ESP_RouteA:
            ButtonEvent = Remote_RouteA;
            break; 
        case ESP_RouteB:
            ButtonEvent = Remote_RouteB;
            break; 
        case ESP_RouteC:
            ButtonEvent = Remote_RouteC;
            break; 
        case ESP_RouteD:
            ButtonEvent = Remote_RouteD;
            break;
        case ESP_RouteE:
            ButtonEvent = Remote_RouteE;
            break;
        case ESP_RouteF:
            ButtonEvent = Remote_RouteF;
            break;
        case ESP_RouteG:
            ButtonEvent = Remote_RouteG;
            break;
            case ESP_RouteH:
            ButtonEvent = Remote_RouteH;
            break;
            case ESP_RouteI:
            ButtonEvent = Remote_RouteI;
            break;
            case ESP_RouteJ:
            ButtonEvent = Remote_RouteJ;
            break;
            case ESP_RouteK:
            ButtonEvent = Remote_RouteK;
            break;
        case ESP_RoutePlay:
            ButtonEvent = Remote_RoutePlay;
            break;
        case ESP_RoutePause:
            ButtonEvent = Remote_RoutePause;
            break;
        case ESP_PowerON:
            ButtonEvent = Remote_PowerON;
            break;
        case ESP_PowerOFF:
            ButtonEvent = Remote_PowerOFF;
            break;
        case ESP_ChargeON:
            ButtonEvent = Remote_ChargeON;
            break;
        case ESP_ChargeOFF:
            ButtonEvent = Remote_ChargeOFF;
            break;
        case ESP_SafetyON:
            ButtonEvent = Remote_SafetyON;
            break;
        case ESP_SafetyOFF:
            ButtonEvent = Remote_SafetyOFF;
            break;
        case ESP_AugerStart:
            ButtonEvent = Remote_ThumbleStart;
            break;
        case ESP_AugerStop:
            ButtonEvent = Remote_ThumbleStop;
            break;
        default:
            if(CurrentMessage != 0x3030){
                isV = CurrentMessage >> 8;
                if (isV == 'V'){
                    speed = (uint8_t)CurrentMessage;
                    ButtonEvent = Remote_Speed;
                }
                else if (isV == 'X'){
                    route_step = (uint8_t)CurrentMessage;
                    ButtonEvent = Remote_RouteStep;
                }
                else
                {
                    __asm("nop");
                }
            }
            else if(CurrentMessage == 0x3030){
              ButtonEvent = Remote_Released; 
            }
            else{
                __asm("nop");
            }
            break;
    }   
    

}
bool IMUHandler_IsRouteSelectButton(void){
    bool RetFlag = false;
    if (ButtonEvent >= Remote_RouteA && ButtonEvent < Remote_Released)
        RetFlag = true;
    return RetFlag;
} 


uint8_t Remote_GetSpeed(){
    return speed ;
}
uint8_t Remote_GetRouteStep(){
    return route_step;
}

void Remote_ClearEvent(void){
    ButtonEvent = Remote_Released;
}

RemoteButton IMUHandler_GetRemoteMessage(){
    return ButtonEvent;
}

void IMUHandler_SetCurrentRouteStep(uint8_t Step){
    RouteStepCnt = (uint16_t)Step;
}

void IMUHandler_EmergencyStop(void){
    MotorManager_StopAllMotors();
    // todo: [PM] add indication
}