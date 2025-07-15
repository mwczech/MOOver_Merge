
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */



#include <xc.h> // include processor files - each processor file is guarded.  

#define dLEFT       1
#define dRIGHT      2

#define dRIGHTSPIN  1
#define dLEFTSPIN   2

#define dENABLED    1
#define dDISABLED   0
#define dTRACK      2

#define dRouteA     5
#define dRouteB     6
#define dRouteC     7
#define dRouteD     8


typedef enum Motor_t{
    Motor_Left = 0,
    Motor_Right,
    Motor_Thumble,
    Motor_Lift,
    Motor_Belt1,
    Motor_Belt2,
    Motor_NumOf
}MotorName;

typedef enum StateMachineStates_t{
    State_Init = 0,
    State_Stop,
    State_WaitForEvent,
    State_Track
    
}StateMachineStates;


/* main.c functions */
void MotorManager_Initialise(void);
void MotorManager_Perform1ms( void );
void MotorManager_Perform100ms( void );
void MotorManager_PerformAfterMainLoop(void);
void MotorManager_StateMachine( void );
void MotorManager_TriggerEnableMessageSend(uint16_t Timeout);
/*  */
void MotorManager_SetStateMachineState(StateMachineStates State);

/* Odometry Handling */
void CalculateOdometry_Data(void);
void MotorManager_SaveRoad(void);
void MotorManager_SetPositionCount(MotorName Mot, uint16_t Count);
void MotorManager_SetCurrent(MotorName Mot, int16_t Current);
uint16_t MotorManager_GetCurrent(MotorName Mot);
void CalculateDistance(MotorName Motor);
 void MotorManager_ResetRotationCount(MotorName Mot);

/* Motors settings */
void MotorManager_SetDirection(MotorName Mot, uint8_t Direction);
bool MotorManager_IsMotorEnabled(MotorName Mot);
bool MotorManager_IsAnyMotorEnabled();
void MotorManager_SetMotorState(uint8_t Motor, uint8_t State);
void MotorManager_SetDefaultSpeed(void);

void MotorManager_ResetHigherSpeedFlag(void);
bool MotorManager_GetHigherSpeedFlag(MotorName Mot);

uint16_t MotorManager_GetSpeed(uint8_t Motor);
void MotorManager_SetSpeed(uint8_t Motor, uint16_t Speed);

uint16_t MotorManager_GetStepSpeed(uint8_t Mot);
void MotorManager_SetStepSpeed(uint8_t Mot, uint16_t Speed);

void MotorManager_StartMotor(MotorName Mot, uint8_t Direction
);
uint8_t MotorManager_GetStepDirection(uint8_t Mot);
void MotorManager_SetStepDirection(uint8_t Mot, uint8_t dir);

int32_t MotorManager_GetRotationCount(MotorName Mot);
int32_t MotorManager_GetRotationCountPositive(MotorName Mot);
void MotorManager_SetRotationCount(MotorName Mot, int32_t RotationCount);

uint16_t MotorManager_GetPositionCount(MotorName Mot);
/* Motors CAN message handling*/

/* Sending message with speed frame. Direction is taken from "Motors" structure */
void MotorManager_StartMotorKeepDirection(MotorName Mot);
void MotorManager_StartMotorOpositeDirection(MotorName Mot);


/* Stop selected motor */
void MotorManager_StopMotor(MotorName Mot);
void MotorManager_StopAllMotors(void);
uint8_t CalculateShaftTurn( MotorName Name );

bool MotorManager_IsRotationCountResetRequest(void);
void MotorManager_ResetRotationCountResetRequest(void);
void MotorManager_SetRotationCountResetRequest(void);

//dev variables for HMI
extern uint16_t RWheelSetSpeed, LWheelSetSpeed, AugSetSpeed, LastRotR, LastRotL;
extern float CurrentAngle2, StepAngle, PrevStepAngle;
extern int IntStepAngle;
//CurrentAngle2 because theres another variable called CurrentAngle and I don't want to mess anything up