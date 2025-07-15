#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pmb_RouteManager.h"
#include "pmb_MotorManager.h"
#include "pmb_Scheduler.h"
#include "IMUHandler/IMUHandler.h"
#include "pmb_Functions.h"
#include "pmb_Display.h"
#include "pmb_Keyboard.h"
#include "Tools/Timer.h"
#include "pmb_Settings.h"
#include "DriveIndicator.h"
#include "RoutesDataTypes.h"
#include "AnalogHandler/AnalogHandler.h"
#include "DiagnosticsHandler.h"
#include "pmb_System.h"
#include "mcc_generated_files/pin_manager.h"
#include "BatteryManager/BatteryManager.h"

#define FULL_WHEEL_TURN 55

uint8_t Hourtmp, Mintmp;
uint8_t Hour1, Min1;
static uint8_t RouteRepetitionCount;
bool VelocityCorrection;
float CalulatedAngle;
int MagnetSearchWindowFlag;

bool stepRepeatFlag = 0;
uint8_t stepRepeatCount = 0;

enum RouteParameters
{
    OpType = 0,
    dX,
    dY,
    SpeedRight,
    SpeedLeft,
    LDir,
    RDir,
    ThumbleState
};

typedef enum RouteStates_t
{
    RouteState_Init = 0,
    RouteState_Idle,
    RouteState_WaitForStart,
    RouteState_BuzzerLampInication,
    RouteState_SetNextStep,
    RouteState_Drive,
} RouteStates;

typedef struct FeedingTableData_t
{
    uint8_t FirstTableEnter;
    uint8_t FirstTableExit;
    uint8_t SecondTableEnter;
    uint8_t SecondTableExit;
} FeedingTableData;

// Routes
//{MODE, dX, dY, VelocityR, VelocityL, DirectionR, DirectionL, ThumbleState}

/* Route settings */
uint8_t Route_Scheduler_Cnt;
bool ActivationByScheduler;
bool SchedulerAllowanceFlag;
OperType Operation_Type;
OperType Operation_TypeSaved;
uint8_t CurrentRouteStep;
RouteStates RouteState;
Route_ID RouteSelected;
/* End of route settings */

FeedingTableData FeedingTableParam;

uint16_t Route_Step, Route_StepMax;
uint32_t cor_dX2, cor_dY2 = 0;
uint32_t cor_dX, cor_dY = 0;
uint32_t TetaAngle, AlfaAngle;
int32_t RouteRotationCountLeft, RouteRotationCountRight;
uint16_t Diagonal;
uint16_t Offset;
uint8_t RequestedStepNumber;
uint8_t SendAdditionalStepsCount;

uint32_t Diagonal_O;
uint8_t Selected_Route = 0;
uint16_t SpeedTemp;
bool IsRoutePause;
bool CurrentStepDone;
bool ManualCorrectionDone;
bool MagnetsDiscoveredLatched;
bool MagnetsDiscovered;
float DesiredAngle;
float TurnAngle;
float CurrentAngle;
float AngleBeforePauseButton;
float MagnetCorrectionAngle = 0;
bool MagnetDetection;
bool SlowerSpeedFlag;
bool autoRoutePlay;
MagnetsStatus StatusM;
uint8_t AngleCorrectionMagnet;
float EncoderMultiplier;
int32_t RotationCountBeforePauseLeft;
int32_t RotationCountBeforePauseRight;

RouteData CurrentRoute;
extern uint16_t RouteStepSelected;

float previusMagnetDeltaDistance = 0;
float previusMagnetDetected = 0;
float previusTurnAngle = 0;
float stepDistanceOffset = 0;
float previusMagnetCorrectionAngle = 0;

//information if robot is going to accelerate or stop in current step
bool accelerating = false;
bool decelerate = false;
bool rampEnable = false;//enable acceleration and decelaration
bool changedDirection = false;
bool previusStepNormInTheSameDirection = false;

float EncoderFinishedPercent;

bool RouteManager_LoadNextStepData(void);
void RouteManager_AutomaticCorrection(float Angle);
bool RouteManager_IsTurnStepAchieved(OperType Operation);
bool RouteManager_Is90DegStepAchieved(OperType Operation);
bool RouteManager_IsNormStepAchieved(void);
bool RouteManager_IsNormNoMagnetStepAchieved(void);
bool RouteManager_ManualCorrection(void);
bool RouteManager_IsCurrentStepDone();
void RouteManager_FinishRoute(void);
bool RouteManager_IsRouteSelectButton(DisplayButton Event);
void RouteManager_PrepareRouteSettings(Route_ID  Root);
void RouteManager_SetMotors(void);
void RouteManager_AutomaticCorrectionForward(float Angle);
void RouteManager_AutomaticCorrectionReverse(float Angle);
void RouteManager_RoutePause(void);
void RouteManager_RoutePlay(void);
void RouteManager_ChargeSensorHandler(void);

bool isChargeMagnetDetected = false;

void RouterManager_ClearEventDuringError(DisplayButton *DisplayEvent, RemoteButton *RemoteEvent);

void RouteManager_Init(void)
{
    RouteState = RouteState_Init;
    CurrentStepDone = false;
    MagnetsDiscoveredLatched = false;
    Diagonal = 0;
    RequestedStepNumber = 255;
    RouteManager_SetStepRequest(0);
}

void RouteManager_Perform1ms(void)
{
    /* Calculation of Read_Measured to check, if new step should be applied */
    /* Previous implementation name RouteRoutineTimer */

    switch (Operation_Type){
    case NORM:
        /* Do not stop wheels untiil IMU/Encoder step data reached */
        if (RouteManager_IsNormStepAchieved()){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        
        break;
    case NORM_NOMAGNET:
        /* Do not stop wheels until IMU/Encoder step data reached */
        if (RouteManager_IsNormNoMagnetStepAchieved()){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
       
        break;
    case TU_R:
        if (RouteManager_IsTurnStepAchieved(TU_R)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case TU_L:
        if (RouteManager_IsTurnStepAchieved(TU_L)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case L_90:
        if (RouteManager_Is90DegStepAchieved(L_90)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case R_90:
        if (RouteManager_Is90DegStepAchieved(R_90)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case OpTypeNoOperation:
        break;
    default:
        break;
    }
}

void RouteManager_Perform100ms(void)
{
    /* Calculation of Read_Measured to check, if new step should be applied */
    /* Previous implementation name RouteRoutineTimer */
    CurrentAngle = IMUHandler_GetAngle();

    switch (Operation_Type){
    case NORM:
        /* Do not stop wheels untiil IMU/Encoder step data reached */
        if (RouteManager_IsNormStepAchieved()){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        else{
            /* Handle manual correction when keyboard/display buttons are pressed */
            if (RouteManager_ManualCorrection()){
                /* Reset StepBeginAngle to set new angle to follow during norm */
//                DesiredAngle = CurrentAngle;
            }
            else if(MotorManager_GetStepDirection(Motor_Left)==L_FOR && MotorManager_GetStepDirection(Motor_Right)==R_FOR){//if going forward correct angle
                /* Handler automatic correction - correct wheels speed if dCORRECTION_ANGLE_THRESHOLD is exceed */
                    RouteManager_AutomaticCorrectionForward(CurrentAngle);//if mopover is driving forward
            }
            else if(MotorManager_GetStepDirection(Motor_Left)==L_REV && MotorManager_GetStepDirection(Motor_Right)==R_REV){// if going back correct angle
                /* Handler automatic correction - correct wheels speed if dCORRECTION_ANGLE_THRESHOLD is exceed */
                    RouteManager_AutomaticCorrectionReverse(CurrentAngle);//if mopover is driving backward
            }
            
        }
        break;
    case NORM_NOMAGNET:
        /* Do not stop wheels until IMU/Encoder step data reached */
        if (RouteManager_IsNormNoMagnetStepAchieved()){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        else{
            /* Handle manual correction when keyboard/display buttons are pressed */
            if (RouteManager_ManualCorrection()){
                /* Reset StepBeginAngle to set new angle to follow during norm */
//                DesiredAngle = CurrentAngle;
            }
            else if(MotorManager_GetStepDirection(Motor_Left)==L_FOR && MotorManager_GetStepDirection(Motor_Right)==R_FOR){//if going forward correct angle
                /* Handler automatic correction - correct wheels speed if dCORRECTION_ANGLE_THRESHOLD is exceed */
                
                RouteManager_AutomaticCorrectionForward(CurrentAngle);
            }
            else if(MotorManager_GetStepDirection(Motor_Left)==L_REV && MotorManager_GetStepDirection(Motor_Right)==R_REV){// if going back correct angle
                /* Handler automatic correction - correct wheels speed if dCORRECTION_ANGLE_THRESHOLD is exceed */
                
                RouteManager_AutomaticCorrectionReverse(CurrentAngle);
            }
        }
        break;
    case TU_R:
        if (RouteManager_IsTurnStepAchieved(TU_R)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case TU_L:
        if (RouteManager_IsTurnStepAchieved(TU_L)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case L_90:
        if (RouteManager_Is90DegStepAchieved(L_90)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case R_90:
        if (RouteManager_Is90DegStepAchieved(R_90)){
            MotorManager_SaveRoad();
            CurrentStepDone = true;
        }
        break;
    case OpTypeNoOperation:
        break;
    default:
        break;
    }
    RouteManager_ChargeSensorHandler();
}

void RouteManager_StoreOperationType(void){
    /* Do not store operation type if already stopped */
    if( OpTypeNoOperation != Operation_Type)
    Operation_TypeSaved = Operation_Type;
}

void RouteManager_SetOperationType(OperType OpType){
    Operation_Type = OpType;
}

void RouteManager_RestoreOperationType(void){
    Operation_Type = Operation_TypeSaved;
}


void RouteManager_StateMachine(void){
    DisplayButton Display_Button = Display_Released;
    RemoteButton  Remote_Button = Remote_Released;
    BatteryLevel Battery = BatteryManager_GetBatteryLevel();

    if( Battery == BatteryLevel_Good ){
        Display_Button = Display_GetEvent();
        Remote_Button = IMUHandler_GetRemoteMessage();
   
    }
    else if( Battery == BatteryLevel_Low ){    
       Display_Button = Display_GetEvent();
       Remote_Button = IMUHandler_GetRemoteMessage();
       RouterManager_ClearEventDuringError(&Display_Button, &Remote_Button);

        /* In case of critical low voltage levels, to not perform any action untill device charged*/

    }

    switch (RouteState){
    case RouteState_Init:
        RouteManager_ResetRouteSettings();
        RouteState = RouteState_Idle;

        break;
    case RouteState_Idle:
        /* Place here if needed different type of event than DisplayTouch/Maybe automatic route from RTC */
        /* Check if route has been selected */
        /* To add new route, go to DisplayButton enumarator in pmb_Display.h */
        if( Display_EMERGANCY_STOP == Display_Button || Remote_Stop == IMUHandler_GetRemoteMessage()){
            RouteManager_SetStepRequest(0);
        }    
        
        if (Display_ENABLE_POWER == Display_Button)
            SchedulerAllowanceFlag = true; // let the scheduler work after another power enable
        
        if (RouteManager_IsRouteSelectButton(Display_Button) && Battery == BatteryLevel_Good){
            RouteSelected = Display_Button - Display_ROUTE_A; /* To make route index from 0 to n */
            RouteManager_PrepareRouteSettings(RouteSelected);
        
            RouteState = RouteState_WaitForStart;
        }
        if( IMUHandler_IsRouteSelectButton() && Battery == BatteryLevel_Good ){
            RouteSelected = IMUHandler_GetRemoteMessage() - Remote_RouteA; /* To make route index from 0 to n */
            RouteManager_PrepareRouteSettings(RouteSelected);
   
            RouteState = RouteState_WaitForStart;
        }
        RouteSelected = Scheduler_GetRouteFromScheduler();
        if( RouteSelected != Route_NumOf && Diagnostics_IsInvertersReady()){
            LED2_SetHigh();
            RouteManager_PrepareRouteSettings(RouteSelected);
            autoRoutePlay = true;
            RouteState = RouteState_WaitForStart;
            
        }

        if( Remote_RouteStep == IMUHandler_GetRemoteMessage()){
            RouteManager_SetStepRequest(Remote_GetRouteStep());
           // Remote_ResetRouteStep();//Missing function
        }
        break;

    case RouteState_WaitForStart:
        if (Display_PLAY == Display_Button || Remote_RoutePlay == IMUHandler_GetRemoteMessage() || autoRoutePlay){
#ifdef dDEBUG_DISABLE_BUZZER_ROUTE            
            DriveIndicator_SetIndication(0, 3000);
#else
            DriveIndicator_SetIndication(3000, 3000);
#endif
            autoRoutePlay = false;
            RouteState = RouteState_BuzzerLampInication;

        } 
         /* -------------------EMERGANCY STOP BUTTON HANDLING-----------------------------*/  
        if (Display_EMERGANCY_STOP == Display_Button || Remote_Stop == IMUHandler_GetRemoteMessage()){
            RouteManager_FinishRoute();
            RouteManager_SetStepRequest(0);
            DriveIndicator_SetDisable(dIndicationType_Both);
            ActivationByScheduler = false;
            SchedulerAllowanceFlag = false;
            MotorManager_SetStateMachineState(State_Stop);
        }
          
        break;    
    case RouteState_BuzzerLampInication:
        /* Start first step of route after buzzer and lamp indication */
        if (DriveIndicator_IsFinishedIndication()){
            RouteManager_LoadNextStepData();
            RouteManager_SetMotors();
            MotorManager_TriggerEnableMessageSend(500);
            MotorManager_SetRotationCountResetRequest();
            RouteState = RouteState_Drive;
            CurrentStepDone = false;
            DriveIndicator_SetDisable(dIndicationType_Both);
        }
        break;
    case RouteState_SetNextStep:
        /* Load next step data, if function return 0, route is ended and machine goes to Idle state */
        if (RouteManager_LoadNextStepData()){
            // Uncomment if DriveIndication is needed at every route step
            // DriveIndicator_SetEnable();
            RouteManager_SetMotors();
            MotorManager_SetRotationCountResetRequest();
            
            /* Next step data loaded, go to drive state */
            RouteState = RouteState_Drive;
        }
        else{
            /* All steps finished, go to idle */
            RouteManager_FinishRoute();
            MotorManager_SetStateMachineState(State_WaitForEvent);
        }
        break;
    case RouteState_Drive:
/* -------------------SAFETY SWITCH / PAUSE BUTTON HANDLING-----------------------*/
        if( AnalogHandler_IsSafetyActivated() || Display_PAUSE == Display_Button || Remote_RoutePause == IMUHandler_GetRemoteMessage())
        {
            if( AnalogHandler_IsSafetyActivated()  ){
                System_PowerRailRequestSequence(Sequence_PowerStageOn);
            }
            RouteManager_RoutePause();
        }

/* ------------------------------------------------------------------------------*/

/* -------------------EMERGANCY STOP BUTTON HANDLING-----------------------------*/  
        if (Display_EMERGANCY_STOP == Display_Button || Remote_Stop == IMUHandler_GetRemoteMessage())
        {
            LED2_SetLow();
            RouteManager_FinishRoute();
            RouteManager_SetStepRequest(0);
            DriveIndicator_SetDisable(dIndicationType_Both);
            ActivationByScheduler = false;
            SchedulerAllowanceFlag = false;
            MotorManager_SetStateMachineState(State_Stop);
        }
/* ------------------------------------------------------------------------------*/

/* -------------------PLAY BUTTON HANDLING---------------------------------------*/
        if (Display_PLAY == Display_Button || Remote_RoutePlay == IMUHandler_GetRemoteMessage() ){
            RouteManager_RoutePlay();
        }
/* ------------------------------------------------------------------------------*/

#ifdef dSKIP_ROUTE_STEP_EVENT_ENABLE
        if(Keyboard.Button == Keyboard_UP){
            CurrentRouteStep++;
            Operation_Type = OpTypeNoOperation;
            RouteState = RouteState_SetNextStep;
            CurrentStepDone = false;               
        }
#else
#endif

        /* If route button selected during route - stop route */
        if (RouteManager_IsRouteSelectButton(Display_Button)){
            RouteManager_FinishRoute();
            //DriveIndicator_SetEnable();
            MotorManager_SetStateMachineState(State_Init);
            RouteState = RouteState_Idle;
        }
        /* Set next step state if current step is done */
        if (RouteManager_IsCurrentStepDone()){
            if(!stepRepeatFlag)//if repeatStepFlag is not activated proceed to next step
                CurrentRouteStep++;
           // else
               // CurrentRouteStep--;
            Operation_Type = OpTypeNoOperation;
            RouteState = RouteState_SetNextStep;
            CurrentStepDone = false;
        }
        break;
    default:
        break;
    }
}

void RouterManager_ClearEventDuringError(DisplayButton *DisplayEvent, RemoteButton *RemoteEvent){
    if( !Diagnostics_IsInvertersReady() || !Diagnostics_IsIMUReady()){
        if( (*DisplayEvent >= Display_ROUTE_A && *DisplayEvent <= Display_ROUTE_K)
        || *DisplayEvent == Display_PLAY){
            DriveIndicator_SetIndication(500, 0);
            *DisplayEvent = Display_Released;
        }
    } 
    
    if( !Diagnostics_IsInvertersReady() || !Diagnostics_IsIMUReady()){
        if( (*RemoteEvent >= Remote_RouteA && *RemoteEvent <= Remote_RouteK)
        || *RemoteEvent == Remote_RoutePlay){
            DriveIndicator_SetIndication(500, 0);
            *RemoteEvent = Remote_Released;
        }
    }
}

void RouteManager_RoutePlay(void){
    if(IsRoutePause){
        RouteManager_SetOperationType(Operation_TypeSaved);

        MotorManager_TriggerEnableMessageSend(0);
        MotorManager_StartMotorKeepDirection(Motor_Left);
        MotorManager_StartMotorKeepDirection(Motor_Right);
        if (CurrentRoute.Step->ThumbleEnabled)
            MotorManager_StartMotorKeepDirection(Motor_Thumble);

        IsRoutePause = false;
    }
}

void RouteManager_RoutePause(void){
    RouteManager_StoreOperationType();
    RouteManager_SetOperationType(OpTypeNoOperation);

    MotorManager_StopMotor(Motor_Left);
    MotorManager_StopMotor(Motor_Right);
    MotorManager_StopMotor(Motor_Thumble);
#if COMPILE_SWITCH_MOONION 
    MotorManager_StopMotor(Motor_Belt1);
    MotorManager_StopMotor(Motor_Belt2);
#endif

    IsRoutePause = true;

}

bool RouteManager_IsNormStepAchieved(void)
{
    bool Ret = false;
    float EncoderFinishedPercent_Left;
    float EncoderFinishedPercent_Right;
    float MagnetsEnableMultiplier;
    float EndStepPercent;
    bool MagnetsDiscovered = false;
    
    StatusM = GetMagnets();
    
    if( true == MagnetsDiscoveredLatched ){
        /* Magnets state from previous step has not been released */
        /* Do not calculate magnets unltil device step out from previous magnets */
        if(StatusM.status == 0){
            MagnetsDiscoveredLatched = false;
        }
    }
    else{
        /* Magnets has been released after step starts - waiting for hitting magnet again */
        //TUTAJ DALEJ - SPRAWDZIC DLACZEGO NIE WCHODZI TUTAJ - GDZIES WCZESNIEJ NA PEWNO WCHODZI BO DISPLAY DZIALA
        if(IMUHandler_GetMagnetMagnetPositionInCM(Magnet1st) != dMAGNET_NO_DETECTION){
            MagnetsDiscovered = true;
            
        }

    }

    EncoderFinishedPercent_Left = ((float)MotorManager_GetRotationCount(Motor_Left) * dDISTANCE_PER_MOTOR_ROTATION) / (float)(cor_dX + stepDistanceOffset) ;
    EncoderFinishedPercent_Right = ((float)MotorManager_GetRotationCount(Motor_Right) * dDISTANCE_PER_MOTOR_ROTATION) / (float)(cor_dX + stepDistanceOffset);
    
    EncoderFinishedPercent = fabs(EncoderFinishedPercent_Left - EncoderFinishedPercent_Right)/2;

    //Set the magnet search window (step completion in %))

    if(cor_dX < 10)
        MagnetsEnableMultiplier = 0.20f;
    else if(cor_dX > 50)
        MagnetsEnableMultiplier = 0.80f;
    else{
        /* Dynamic change from 0.2 to 0.95 */
        MagnetsEnableMultiplier = 0.2f + (((float)cor_dX / 100.0f) * 0.75f);
    }

    //MagnetsEnableMultiplier = 0.50f;    //Lower window limit (%)
    EndStepPercent = 1.5f;  //Upper window limit (%)
    MagnetSearchWindowFlag = 0;  //Reset the flag for indicating the magnet search window
    if(!stepRepeatFlag)
    {
        if (EncoderFinishedPercent >= MagnetsEnableMultiplier){
        /* Achieved 95% from encoders, now waiting for magnets */
            MagnetSearchWindowFlag = 1;
            if(EncoderFinishedPercent >= EndStepPercent){
                /* If exceeded 150% of distance, stop step */
                MagnetSearchWindowFlag = 2;
                Ret = true;
            }
            else if(MagnetsDiscovered){
                /* Magnets detected between 95% and 150% of step */
                Ret = true;

                if(stepRepeatCount != 0)
                    stepRepeatCount = 0;//if moover is not returning to previus position while magnet is found, step repeat count is cleared - moover is back in track!!!
            }
        }
    }
    else//while returning to previous position search for magnets
    {
        MagnetSearchWindowFlag = 1;
        EndStepPercent += 0.2f;//increase magnet search window while returning to previous position
        
        if(EncoderFinishedPercent >= EndStepPercent){//if moover can not find magnet while returning to previous position
            /* If exceeded 150% of distance, stop step */
            MagnetSearchWindowFlag = 2;
            Ret = true;
        }
        else if(MagnetsDiscovered)
        {
            Ret = true;
            stepRepeatFlag = false;//moover found previous position and can retry previously failed step
        }
    }
    
    int thumbleCurrent = abs((int16_t)MotorManager_GetCurrent(Motor_Thumble));
    if(thumbleCurrent > 45 && !stepRepeatFlag)//if overcurrent on auger accured retry step
    {
         MagnetSearchWindowFlag = 2;
         Ret = true;
    }
       
    
    //Magnet Search Window Indication
    if (MagnetSearchWindowFlag == 1)
    {
#ifdef dDEBUG_ENABLE_BUZZER_ROUTE
        DriveIndicator_SetIndication(0, 500);
#endif
    }
    if (MagnetSearchWindowFlag == 2)
    {  
        if(stepRepeatCount>=10 || stepRepeatFlag)//if moover can not find next step after many tries finish route - moover is lost :(
                                               //or moover can not find magnet while tying to go back to previous position - moover is lost, finish route
        {
            MagnetSearchWindowFlag = 0;
            RouteManager_FinishRoute();
            ActivationByScheduler = false;
            SchedulerAllowanceFlag = false;
            MotorManager_SetStateMachineState(State_Stop);
            DriveIndicator_SetIndication(1000, 1000);
        }
        else//if moover can not find next step go to previous position and retry
        {
            stepRepeatCount++;
            stepRepeatFlag = true;
            CurrentRoute.Step--;
            if(CurrentRouteStep > 0)
            {
                CurrentRouteStep--;
            }
        }
    }
    return Ret;
}

bool RouteManager_IsNormNoMagnetStepAchieved(void)
{
    float EncoderFinishedPercent_Left;
    float EncoderFinishedPercent_Right;
    
    EncoderFinishedPercent_Left = (float)MotorManager_GetRotationCount(Motor_Left) * dDISTANCE_PER_MOTOR_ROTATION / (float)cor_dX ;
    EncoderFinishedPercent_Right = (float)MotorManager_GetRotationCount(Motor_Right) * dDISTANCE_PER_MOTOR_ROTATION / (float)cor_dX;
    
    EncoderFinishedPercent = fabs(EncoderFinishedPercent_Left - EncoderFinishedPercent_Right)/2;
    
    if (EncoderFinishedPercent >= 1.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool RouteManager_IsTurnStepAchieved(OperType Operation)
{
    float DiagonalFinishedPercent;
    float IMUFinishedPercent;

    if (Operation == TU_L){
        DiagonalFinishedPercent = (float)MotorManager_GetRotationCount(Motor_Left) / (float)Diagonal;
    }
    else if (Operation == TU_R){
        DiagonalFinishedPercent = (float)MotorManager_GetRotationCount(Motor_Right) / (float)Diagonal;
    }

    IMUFinishedPercent = -(fabs(IMUHandler_CalculateAngle(DesiredAngle, CurrentAngle) / TurnAngle) - 1.0f);

    if ((IMUFinishedPercent * dIMU_JUDGEMENT_FACTOR + fabs(DiagonalFinishedPercent) * dECODER_JUDGEMENT_FACTOR) >= 0.97f){
        return true;
    }
    else{
        return false;
    }
}

bool RouteManager_Is90DegStepAchieved(OperType Operation)
{
    float EncoderFinishedPercent_Left;
    float EncoderFinishedPercent_Right;
    float IMUFinishedPercent;

    float IMUJudgementResult;
    float LeftJudgementResult;
    float RightJudgementResult;
    
    EncoderFinishedPercent_Left = (float)MotorManager_GetRotationCount(Motor_Left) / (float)FULL_WHEEL_TURN;
    EncoderFinishedPercent_Right = (float)MotorManager_GetRotationCount(Motor_Right) / (float)FULL_WHEEL_TURN;
    
    IMUFinishedPercent = fabs(-(fabs(IMUHandler_CalculateAngle(DesiredAngle + MagnetCorrectionAngle, CurrentAngle) / TurnAngle) - 1.0f));//in some rare cases resoult of this func was negative therefore absolute value is needed
   
    IMUJudgementResult = IMUFinishedPercent * dIMU_JUDGEMENT_FACTOR;
    LeftJudgementResult = fabs(EncoderFinishedPercent_Left * dECODER_JUDGEMENT_FACTOR);
    RightJudgementResult = fabs(EncoderFinishedPercent_Right * dECODER_JUDGEMENT_FACTOR);
    
    if(IMUFinishedPercent>0.5 && false == SlowerSpeedFlag)
    {
        //SlowerSpeedFlag = true;
        MotorManager_SetSpeed(Motor_Right, MotorManager_GetStepSpeed(Motor_Right)*(1-((IMUFinishedPercent-0.5)*1.7)));
        MotorManager_StartMotorKeepDirection(Motor_Right);
        
        MotorManager_SetSpeed(Motor_Left, MotorManager_GetStepSpeed(Motor_Left)*(1-((IMUFinishedPercent-0.5)*1.7)));
        MotorManager_StartMotorKeepDirection(Motor_Left);
    }
    else
    {
        MotorManager_SetSpeed(Motor_Right, MotorManager_GetStepSpeed(Motor_Right)*((IMUFinishedPercent * 1.6)+0.2));
        MotorManager_StartMotorKeepDirection(Motor_Right);
        
        MotorManager_SetSpeed(Motor_Left, MotorManager_GetStepSpeed(Motor_Left)*((IMUFinishedPercent * 1.6)+0.2));
        MotorManager_StartMotorKeepDirection(Motor_Left);
    }
    
    if (fabs(IMUHandler_CalculateAngle(DesiredAngle + MagnetCorrectionAngle, CurrentAngle))<1)
    {
        SlowerSpeedFlag = false;
        return true;
//        MotorManager_SetSpeed(Motor_Right, 0);
//        MotorManager_SetSpeed(Motor_Left, 0);
    }
    else{
        return false;
    }
}

bool RouteManager_ManualCorrection(void)
{
    bool RetFlag = false;
    if (MotorManager_GetHigherSpeedFlag(Motor_Left) || MotorManager_GetHigherSpeedFlag(Motor_Right)){
        RetFlag = true;
    }
    return RetFlag;
}

bool RouteManager_IsRouteOngoing(void)
{
    if (RouteState > RouteState_Idle)
        return true;
    else
        return false;
}

void RouteManager_SetMotors(void){
    if (MotorManager_IsMotorEnabled(Motor_Left))
        MotorManager_StartMotorKeepDirection(Motor_Left);
    else
        MotorManager_StopMotor(Motor_Left);

    if (MotorManager_IsMotorEnabled(Motor_Right))
        MotorManager_StartMotorKeepDirection(Motor_Right);
    else
        MotorManager_StopMotor(Motor_Right);

    if (MotorManager_IsMotorEnabled(Motor_Thumble))
        MotorManager_StartMotorKeepDirection(Motor_Thumble);
        //MotorManager_StopMotor(Motor_Thumble);
}

void RouteManager_SetStepRequest(uint8_t Step){
    SendAdditionalStepsCount = 4;
    RequestedStepNumber = Step;
}

void RouteManager_PrepareRouteSettings(Route_ID  Route)
{
    if(RequestedStepNumber != 255){
        Route_SetRoutePointer(&CurrentRoute, RouteSelected, RequestedStepNumber);
        CurrentRouteStep = RequestedStepNumber;
        /*Reset request*/
    }
    else{
        Route_SetRoutePointer(&CurrentRoute, RouteSelected, CurrentRouteStep);
    }
    RequestedStepNumber = 255;
    //CurrentRoute.PreviousStep = 
    RouteRepetitionCount = CurrentRoute.RepeatCount;
    MotorManager_ResetRotationCount(Motor_Left);
    MotorManager_ResetRotationCount(Motor_Right);
    DesiredAngle = IMUHandler_GetAngle();
    previusMagnetDeltaDistance = 0;
    previusTurnAngle = 0;
}

/* Stop motors, set default speed and change route state to IDLE */
void RouteManager_FinishRoute(void)
{  
    int i=1;
                CurrentAngle2 = IMUHandler_GetAngle();
            StepAngle = CurrentAngle2 - PrevStepAngle;
            
            if (StepAngle > 180)
            {
                StepAngle = StepAngle - 360;
            }
            else if (StepAngle < -180)
            {
                StepAngle = StepAngle + 360;
            }
            
            PrevStepAngle = CurrentAngle2;
            IntStepAngle = (int)(fabs(StepAngle)*10);
    RouteManager_ResetRouteSettings();
    MotorManager_StopMotor(Motor_Right);
    MotorManager_StopMotor(Motor_Left);
    MotorManager_StopMotor(Motor_Thumble);
    MotorManager_SetDefaultSpeed();
    RouteState = RouteState_Idle;
    stepRepeatFlag = false;
    stepRepeatCount = 0;//reset values while finishing route
}

bool RouteManager_IsRouteSelectButton(DisplayButton Event)
{
    bool RetFlag = false;
    if (Event >= Display_ROUTE_A && Event < Display_Released)
        RetFlag = true;
    return RetFlag;
}

uint8_t RouteManager_GetCurrentRouteStep(void){
    uint8_t RetVal = 255;
    if( RouteState_Idle != RouteState ){
        RetVal =  CurrentRouteStep;
    }
    return RetVal;
}

bool RouteManager_SwitchToNextStep(){
    bool Ret;
    Ret = true;
    if (CurrentRoute.StepCount ==  CurrentRouteStep){
        if (RouteRepetitionCount > 0){
            CurrentRouteStep = 0;
            Route_SetRoutePointer(&CurrentRoute, RouteSelected, 0);
            RouteRepetitionCount = RouteRepetitionCount - 1;
            if(RouteRepetitionCount == 0){
                Ret = false;
                
            }
        }
        else{
            Ret = false;
        }
     }
    return Ret;
}
float MagnetCM;
double MagnetCMDouble;
double RouteStepDxDouble;
bool RouteManager_LoadNextStepData(void)
{
    bool IsEndOfRoute = 1;
    MotorManager_ResetHigherSpeedFlag();
   
    if( RouteManager_SwitchToNextStep()){
        //Save the distance driven after finishing a route step (for dev display purposes)
        LastRotL = MotorManager_GetRotationCountPositive(Motor_Left) * dDISTANCE_PER_MOTOR_ROTATION;
        LastRotR = MotorManager_GetRotationCountPositive(Motor_Right) * dDISTANCE_PER_MOTOR_ROTATION;


        
        //reset variables
        accelerating = false;
        decelerate = false;
        changedDirection = false;
        previusStepNormInTheSameDirection = false;
        
        int nextStepDistance = 0;
        
        if(CurrentRouteStep>0)
        {//check if robot should accelerate at sthe start of this step
            uint8_t currentDirRight = CurrentRoute.Step->DirectionRight;//check current step motor directions
            uint8_t currentDirLeft = CurrentRoute.Step->DirectionLeft;
            
            CurrentRoute.Step--;//check previous step
            
            if(CurrentRoute.Step->OperationType != NORM)
                if(rampEnable)
                    accelerating = true;
            
            if(CurrentRoute.Step->OperationType == NORM && CurrentRoute.Step->DirectionRight != currentDirRight && CurrentRoute.Step->DirectionLeft != currentDirLeft)
            {
                if(rampEnable)
                    accelerating = true;
                
                changedDirection = true;
            }
            else if(CurrentRoute.Step->OperationType == NORM && CurrentRoute.Step->DirectionRight == currentDirRight && CurrentRoute.Step->DirectionLeft == currentDirLeft)
            {
                previusStepNormInTheSameDirection = true;
            }
            CurrentRoute.Step++;
        }
        else if(rampEnable)
            accelerating = true;// if robot is on first step accelerate
        
        if (CurrentRouteStep == CurrentRoute.StepCount-1)//if robot is on last step decelerate on the end
        {
            if(rampEnable)
                decelerate = true;
        }
        else
        {//check if robot should decelerate at the end of this step
            CurrentRoute.Step++;//check next step
            
            if(CurrentRoute.Step->OperationType != NORM && CurrentRoute.Step->OperationType != NORM_NOMAGNET)
            {
                if(rampEnable)
                    decelerate = true;
            }
            else if(CurrentRoute.Step->MagnetCorrection != dMAGNET_NO_CORRECTION)
                    nextStepDistance = CurrentRoute.Step->dX;    
            
            CurrentRoute.Step--;
        }
        
        
        MagnetCM = IMUHandler_GetMagnetMagnetPositionInCM(Magnet1st);
        stepDistanceOffset = 0;
        //if robot is going in straight line and correction is on nad is not returning to previous position
        if(CurrentRoute.Step->MagnetCorrection != dMAGNET_NO_CORRECTION && CurrentRoute.Step->OperationType == NORM && !stepRepeatFlag){//angle correction only in norm step
            
            if((MagnetCM == dMAGNET_NO_DETECTION) && (previusTurnAngle != 0))//if magnet was not detected and last step was turning
            {
                MagnetCM = (cos(previusTurnAngle*0.01745329251)*previusMagnetDetected);//calculate magnet position based on last detected magnet and turn angle
                //calculate step distance offset
                stepDistanceOffset = (sin(previusTurnAngle*0.0174)*previusMagnetDetected);
            }
            else if (MagnetCM == dMAGNET_NO_DETECTION)//if magnet is not detected cancel magnet correction
            {
                MagnetCM = CurrentRoute.Step->MagnetCorrection;
            }
            
             if(CurrentRoute.Step->DirectionLeft == L_REV && CurrentRoute.Step->DirectionRight == R_REV)//if going in reverse
             {
                MagnetCMDouble = -(double)(MagnetCM - CurrentRoute.Step->MagnetCorrection);//adjusting for expected magnet
                stepDistanceOffset = -stepDistanceOffset;
             }
             else
             {
                MagnetCMDouble = (double)(MagnetCM - CurrentRoute.Step->MagnetCorrection);//adjusting for expected magnet
             }
            
            RouteStepDxDouble = (double)(CurrentRoute.Step->dX + stepDistanceOffset);
                        
            if(MagnetCMDouble < 0){
                MagnetCorrectionAngle = atan(-MagnetCMDouble/RouteStepDxDouble);
                MagnetCorrectionAngle = -MagnetCorrectionAngle * 180.0d / 3.1416d;
            }
            else if (MagnetCMDouble > 0){
                MagnetCorrectionAngle = atan(MagnetCMDouble/RouteStepDxDouble);
                MagnetCorrectionAngle = MagnetCorrectionAngle * 180.0d / 3.1416d;
            }
            else{
                MagnetCorrectionAngle = 0;
            }
            
        }
        else if((nextStepDistance != 0) && (CurrentRoute.Step->Angle != 0) && (MagnetCM != dMAGNET_NO_DETECTION))//if robot is turning and magnet is detected and correction in the next step is on
        {
            float magnet = MagnetCM;
            
            MagnetCM = (cos(CurrentRoute.Step->Angle*0.01745329251)*magnet);//calculate virtual magnet for the next step
            stepDistanceOffset = (sin(CurrentRoute.Step->Angle*0.01745329251)*magnet);
            
            CurrentRoute.Step++;
            
            if(CurrentRoute.Step->DirectionLeft == L_REV && CurrentRoute.Step->DirectionRight == R_REV)
            {
                MagnetCMDouble = -(double)(MagnetCM - CurrentRoute.Step->MagnetCorrection);//get next step expected magnet
                stepDistanceOffset = -stepDistanceOffset;
            }
            else
            {
                MagnetCMDouble = (double)(MagnetCM - CurrentRoute.Step->MagnetCorrection);//get next step expected magnet
            }
            
           RouteStepDxDouble = (double)(CurrentRoute.Step->dX + stepDistanceOffset);
            
            CurrentRoute.Step--;
            
            if(MagnetCMDouble < 0){//calculate correction
                MagnetCorrectionAngle = atan(-MagnetCMDouble/RouteStepDxDouble);
                MagnetCorrectionAngle = -MagnetCorrectionAngle * 180.0d / 3.1416d;
            }
            else if (MagnetCMDouble > 0){
                MagnetCorrectionAngle = atan(MagnetCMDouble/RouteStepDxDouble);
                MagnetCorrectionAngle = MagnetCorrectionAngle * 180.0d / 3.1416d;
            }
            else{
                MagnetCorrectionAngle = 0;
            }
            
          
            
        }
        else if(!stepRepeatFlag)//do not cancel the magnet correction if robot is returning to previous step
            MagnetCorrectionAngle = 0;
        
        
        if(CurrentRoute.Step->OperationType == NORM && CurrentRoute.Step->dX < 50)//if norm step is shorter than 50cm - limit magnet correction angle
            if(MagnetCorrectionAngle > 2)
                MagnetCorrectionAngle = 2;
            else if(MagnetCorrectionAngle <- 2)
                MagnetCorrectionAngle = -2;

        
        if(stepRepeatFlag)//if moover is going back to previous position reverse motors
        {
            if(CurrentRoute.Step->DirectionRight == R_FOR)
            {
               MotorManager_SetDirection(Motor_Right, R_REV);
               MotorManager_SetDirection(Motor_Left, L_REV);

               MotorManager_SetStepDirection(Motor_Right, R_REV);
               MotorManager_SetStepDirection(Motor_Left, L_REV);
            }
            else
            {
                MotorManager_SetDirection(Motor_Right, R_FOR);
                MotorManager_SetDirection(Motor_Left, L_FOR);

                MotorManager_SetStepDirection(Motor_Right, R_FOR);
                MotorManager_SetStepDirection(Motor_Left, L_FOR);
            }
        }
        else
        {
            MotorManager_SetDirection(Motor_Right, CurrentRoute.Step->DirectionRight);
            MotorManager_SetDirection(Motor_Left, CurrentRoute.Step->DirectionLeft);

            MotorManager_SetStepDirection(Motor_Right, CurrentRoute.Step->DirectionRight);
            MotorManager_SetStepDirection(Motor_Left, CurrentRoute.Step->DirectionLeft);
        }
        
        MotorManager_SetStepSpeed(Motor_Right, CurrentRoute.Step->RightSpeed);
        MotorManager_SetStepSpeed(Motor_Left, CurrentRoute.Step->LeftSpeed);
        
        if(accelerating)
        {
            MotorManager_SetSpeed(Motor_Right, CurrentRoute.Step->RightSpeed/2);
            MotorManager_SetSpeed(Motor_Left, CurrentRoute.Step->LeftSpeed/2);
        }
        else
        {
            MotorManager_SetSpeed(Motor_Right, CurrentRoute.Step->RightSpeed);
            MotorManager_SetSpeed(Motor_Left, CurrentRoute.Step->LeftSpeed);
        }
        
        cor_dX = CurrentRoute.Step->dX;
        cor_dY = CurrentRoute.Step->dY;
        cor_dX2 = cor_dX * cor_dX;
        cor_dY2 = cor_dY * cor_dY;
        
        
        
        if (CurrentRoute.Step->ThumbleEnabled){
            #if COMPILE_SWITCH_MOONION 
            if( stepRepeatFlag ){
                MotorManager_StopMotor(Motor_Thumble);
                MotorManager_StopMotor(Motor_Belt1);
                MotorManager_StopMotor(Motor_Belt2);
            }
            else{
                MotorManager_TriggerEnableMessageSend(500);
                MotorManager_StartMotor(Motor_Thumble, dRIGHT);
                
                //MotorManager_TriggerEnableMessageSend(400);
                MotorManager_StartMotor(Motor_Belt1, dRIGHT);
                
                //MotorManager_TriggerEnableMessageSend(300);
                MotorManager_StartMotor(Motor_Belt2, dLEFT);
                
                
                DBG1_SetLow();     //Motor Down
                DBG3_SetHigh();
            }
            #else
            if( stepRepeatFlag ){
                MotorManager_StopMotor(Motor_Thumble);
            }
            else{
                MotorManager_TriggerEnableMessageSend(500);
                MotorManager_SetDirection(Motor_Thumble, dRIGHT);
                MotorManager_SetMotorState(Motor_Thumble, 1);
            }
            #endif

        }
        else{
            #if COMPILE_SWITCH_MOONION 
            MotorManager_StopMotor(Motor_Belt1);
            MotorManager_StopMotor(Motor_Belt2);
           
            DBG1_SetHigh();     //Motor UP
            DBG3_SetLow();
            #endif

            MotorManager_StopMotor(Motor_Thumble);
        }
        
        Operation_Type =  CurrentRoute.Step->OperationType;
        if(Operation_Type == NORM || Operation_Type == NORM_NOMAGNET){
            if( CurrentRoute.Step->DirectionLeft == L_FOR && CurrentRoute.Step->DirectionRight == R_FOR )
                EncoderMultiplier = dENCODER_STEP_MAX_MULTIPLIER;
            else
                EncoderMultiplier = dENCODER_STEP_MAX_MULTIPLIER;
        }
        
        
        if(!stepRepeatFlag && CurrentRoute.Step->MagnetCorrection != dMAGNET_NO_CORRECTION && previusMagnetDeltaDistance != dMAGNET_NO_CORRECTION)//if going forward correct disared angle
        {
            float deltaMagnetDistance = MagnetCM - CurrentRoute.Step->MagnetCorrection;
            if(MagnetCM == dMAGNET_NO_DETECTION)
                deltaMagnetDistance = 0;
            
            float deltaDistance =  deltaMagnetDistance - previusMagnetDeltaDistance;
                float stepDistance = CurrentRoute.Step->dX;
                
                float deltaAngle = acos(deltaDistance/(sqrt((deltaDistance*deltaDistance)+(stepDistance*stepDistance))));
                deltaAngle = deltaAngle * 180.0 / 3.1416;
                deltaAngle -= 90;

            if(changedDirection)//alignment 
            {
                if(CurrentRoute.Step->DirectionRight == R_FOR)
                {
                    DesiredAngle += (deltaAngle+previusMagnetCorrectionAngle)*0.75;
                    
                }
                else
                {
                  DesiredAngle -= (deltaAngle-previusMagnetCorrectionAngle)*0.75; 
                }
            }
                
            else if(CurrentRoute.Step->OperationType==NORM && CurrentRoute.Step->DirectionRight == R_FOR && previusStepNormInTheSameDirection)//if going forward correct
            {
                DesiredAngle -= (deltaAngle-previusMagnetCorrectionAngle)/2;
                
            }
            else if(CurrentRoute.Step->OperationType==NORM && CurrentRoute.Step->DirectionRight == R_REV && previusStepNormInTheSameDirection)//if going in reverse correct
            {
                DesiredAngle += (deltaAngle+previusMagnetCorrectionAngle)/2;
                
            }
        }
        
        if(CurrentRoute.Step->OperationType==L_90)
            DesiredAngle -= CurrentRoute.Step->Angle;
        else if(CurrentRoute.Step->OperationType==R_90)
            DesiredAngle += CurrentRoute.Step->Angle;
        
        TurnAngle = CurrentRoute.Step->Angle;
        
        if (DesiredAngle <= -180) {
        DesiredAngle += 360;
        } else if (DesiredAngle > 180) {
            DesiredAngle -= 360;
        }
        
        if(MagnetCM != dMAGNET_NO_DETECTION &&  CurrentRoute.Step->MagnetCorrection != dMAGNET_NO_CORRECTION)//remember last distance error
            previusMagnetDeltaDistance = MagnetCM - CurrentRoute.Step->MagnetCorrection;
        else
            previusMagnetDeltaDistance = dMAGNET_NO_CORRECTION;
        
        
        if(MagnetCM != dMAGNET_NO_DETECTION)//remember position of last detected magnet
            previusMagnetDetected = MagnetCM ;
       
        
        previusMagnetCorrectionAngle = MagnetCorrectionAngle;
        
        previusTurnAngle = CurrentRoute.Step->Angle;//remember last turn angle
        
        StatusM = GetMagnets();
        if( StatusM.status != 0 ){
            MagnetsDiscoveredLatched = true;
        }
        else{
            MagnetsDiscoveredLatched = false;
        }
        
                        CurrentAngle2 = IMUHandler_GetAngle();
            StepAngle = CurrentAngle2 - PrevStepAngle;
            
            if (StepAngle > 180)
            {
                StepAngle = StepAngle - 360;
            }
            else if (StepAngle < -180)
            {
                StepAngle = StepAngle + 360;
            }
            
            PrevStepAngle = CurrentAngle2;
            IntStepAngle = (int)(fabs(StepAngle) * 10);
        
        if(!stepRepeatFlag)
        CurrentRoute.Step++;
     
    }
    else{
        IsEndOfRoute = 0;
    }
    
    /* Called one time - it calculates angle from encoders */
    CalculateOdometry_Data();
    return IsEndOfRoute;
 }

void CalculateOdometry_Data(void)
{
    Diagonal = sqrt(cor_dX2 + cor_dY2);
    Diagonal_O = Diagonal * 10;
    // Value in Radians:
    TetaAngle = acos(cor_dX / Diagonal_O) * 500; // should be *1000 but acos returns doubled value
    // Value in Degrees:
    AlfaAngle = TetaAngle * 5729 / 10000; // gives angle * 10 for better precision
}

void RouteManager_ResetRouteSettings()
{
    RouteManager_SetStepRequest(0);
    Operation_Type = OpTypeNoOperation;
    CurrentRouteStep = 0;
    RouteSelected = Route_NumOf;
}

bool RouteManager_IsCurrentStepDone()
{
    return CurrentStepDone;
}

void RouteManager_AutomaticCorrectionForward(float Angle){
    CalulatedAngle = IMUHandler_CalculateAngle(DesiredAngle+MagnetCorrectionAngle, Angle);

    uint16_t RightSpeed = MotorManager_GetStepSpeed(Motor_Right);
    uint16_t LeftSpeed = MotorManager_GetStepSpeed(Motor_Left);
    
    float accelPecent = 0.3; //precent of route the acceleration will stop
    float decelPecent = 0.7; //precent of route the deceleration will start
    float lowPrecent = 0.2;//slowest speed in precent of stepspeed
    
// Store previous speed adjustment to smoothly increase the speed
static float previousScaleFactor = 1.0; // Initially, no scaling
float scaleFactor = 1.0;
int thumbleCurrent = abs((int16_t)MotorManager_GetCurrent(Motor_Thumble)); // get thumble current

// If current is less than 20A, maintain original speed
if (thumbleCurrent < 20) {
    // Gradually increase scaleFactor if it's less than 1.0
    if (previousScaleFactor < 1.0) {
        previousScaleFactor += 0.01; // Smooth increase (adjust this step for desired smoothness)
        if (previousScaleFactor > 1.0) {
            previousScaleFactor = 1.0; // Limit it to the original speed
        }
    }
    scaleFactor = previousScaleFactor;
} 
else if (thumbleCurrent >= 20 && thumbleCurrent <= 40) {
    // Calculate the scale factor for current between 20A and 40A
    scaleFactor = 0.7 - ((thumbleCurrent - 20) * (0.7 - 0.1) / 20.0);
    
    // Apply the calculated scale factor and update previousScaleFactor
    previousScaleFactor = scaleFactor;
} 
else {
    // If current is greater than 40A, set speed to 0.1
    scaleFactor = 0.05;
    previousScaleFactor = scaleFactor;
}

// Apply the scale factor to the wheel speeds
RightSpeed = RightSpeed * scaleFactor;
LeftSpeed = LeftSpeed * scaleFactor;
    
    if(EncoderFinishedPercent<accelPecent && accelerating)//acceleration
    {
        RightSpeed = RightSpeed * ((EncoderFinishedPercent*(1/accelPecent-lowPrecent*2))+lowPrecent);//accelerate from 20 to 100% in first half of the step
        LeftSpeed = LeftSpeed * ((EncoderFinishedPercent*(1/accelPecent-lowPrecent*2))+lowPrecent);
    }
    else if(EncoderFinishedPercent>decelPecent && EncoderFinishedPercent<1 && decelerate)
    {
        RightSpeed = RightSpeed * (1-(EncoderFinishedPercent-decelPecent)*((1/(1-decelPecent))-lowPrecent*2));//decelerate from 100 to 20% in second half of the step
        LeftSpeed = LeftSpeed * (1-(EncoderFinishedPercent-decelPecent)*((1/(1-decelPecent))-lowPrecent*2));
    }
    else if(EncoderFinishedPercent>=1 && decelerate)
    {
        RightSpeed = RightSpeed * lowPrecent;
        LeftSpeed = LeftSpeed * lowPrecent;
    }
    
    float proportionalCorrectionTreasholdAngle = 3;//on this angle, correction proportional regulation is based
    float correctionFactor = -(fabs(CalulatedAngle / proportionalCorrectionTreasholdAngle) - 1.0f);

    
        if (CalulatedAngle <= - dCORRECTION_ANGLE_THRESHOLD){
                VelocityCorrection = true;
                
                if(fabs(CalulatedAngle)<proportionalCorrectionTreasholdAngle)
                {
                    MotorManager_SetSpeed(Motor_Right, RightSpeed*correctionFactor);
                    MotorManager_StartMotorKeepDirection(Motor_Right);
                }
                else
                {
                    MotorManager_SetSpeed(Motor_Right, RightSpeed);
                    MotorManager_StartMotor(Motor_Right, R_REV);
                }
                
                MotorManager_SetSpeed(Motor_Left, LeftSpeed);
                MotorManager_StartMotorKeepDirection(Motor_Left);
            }
        else if (CalulatedAngle > dCORRECTION_ANGLE_THRESHOLD){
                VelocityCorrection = true;
                
                if(fabs(CalulatedAngle)<proportionalCorrectionTreasholdAngle)
                {
                    MotorManager_SetSpeed(Motor_Left, LeftSpeed*correctionFactor);
                    MotorManager_StartMotorKeepDirection(Motor_Left);
                }
                else
                {
                    MotorManager_SetSpeed(Motor_Left, LeftSpeed);
                    MotorManager_StartMotor(Motor_Left, L_REV);
                }
                
                MotorManager_SetSpeed(Motor_Right, RightSpeed);
                MotorManager_StartMotorKeepDirection(Motor_Right);
        }
        else{
            if (1){
                VelocityCorrection = false;
                MotorManager_SetSpeed(Motor_Left, LeftSpeed);
                MotorManager_SetSpeed(Motor_Right, RightSpeed);
                
                MotorManager_StartMotor(Motor_Right, MotorManager_GetStepDirection(Motor_Right));
                MotorManager_StartMotor(Motor_Left, MotorManager_GetStepDirection(Motor_Left));
            }
        }
}

void RouteManager_AutomaticCorrectionReverse(float Angle)
{

    CalulatedAngle = IMUHandler_CalculateAngle(DesiredAngle+MagnetCorrectionAngle, Angle);

    uint16_t RightSpeed = MotorManager_GetStepSpeed(Motor_Right);
    uint16_t LeftSpeed = MotorManager_GetStepSpeed(Motor_Left);
    
    float accelPecent = 0.3; //precent of route the acceleration will stop
    float decelPecent = 0.7; //precent of route the deceleration will start
    float lowPrecent = 0.2;//slowest speed in precent of stepspeed
    
    if(EncoderFinishedPercent<accelPecent && accelerating)//acceleration
    {
        RightSpeed = RightSpeed * ((EncoderFinishedPercent*(1/accelPecent-lowPrecent*2))+lowPrecent);//accelerate from 20 to 100% in first half of the step
        LeftSpeed = LeftSpeed * ((EncoderFinishedPercent*(1/accelPecent-lowPrecent*2))+lowPrecent);
    }
    else if(EncoderFinishedPercent>decelPecent && EncoderFinishedPercent<1 && decelerate)
    {
        RightSpeed = RightSpeed * (1-(EncoderFinishedPercent-decelPecent)*((1/(1-decelPecent))-lowPrecent*2));//decelerate from 100 to 20% in second half of the step
        LeftSpeed = LeftSpeed * (1-(EncoderFinishedPercent-decelPecent)*((1/(1-decelPecent))-lowPrecent*2));
    }
    else if(EncoderFinishedPercent>=1 && decelerate)
    {
        RightSpeed = RightSpeed * lowPrecent;
        LeftSpeed = LeftSpeed * lowPrecent;
    }
    
    float proportionalCorrectionTreasholdAngle = 3;//on this angle, correction proportional regulation is based
    float correctionFactor = -(fabs(CalulatedAngle / proportionalCorrectionTreasholdAngle) - 1.0f);

    
        if (CalulatedAngle >= dCORRECTION_ANGLE_THRESHOLD){
                VelocityCorrection = true;
                
                if(fabs(CalulatedAngle)<proportionalCorrectionTreasholdAngle)
                {
                    MotorManager_SetSpeed(Motor_Right, RightSpeed*correctionFactor);
                    MotorManager_StartMotorKeepDirection(Motor_Right);
                }
                else
                {
                    MotorManager_SetSpeed(Motor_Right, RightSpeed);
                    MotorManager_StartMotor(Motor_Right, R_FOR);
                }
                
                MotorManager_SetSpeed(Motor_Left, LeftSpeed);
                MotorManager_StartMotor(Motor_Left, MotorManager_GetStepDirection(Motor_Left));
            }
        else if (CalulatedAngle < -dCORRECTION_ANGLE_THRESHOLD){
                VelocityCorrection = true;
                
                if(fabs(CalulatedAngle)<proportionalCorrectionTreasholdAngle)
                {
                    MotorManager_SetSpeed(Motor_Left, LeftSpeed*correctionFactor);
                    MotorManager_StartMotorKeepDirection(Motor_Left);
                }
                else
                {
                    MotorManager_SetSpeed(Motor_Left, LeftSpeed);
                    MotorManager_StartMotor(Motor_Left, L_FOR);
                }
                
                MotorManager_SetSpeed(Motor_Right, RightSpeed);
                MotorManager_StartMotor(Motor_Right, MotorManager_GetStepDirection(Motor_Right));
            }
        else{
            if (1){
                VelocityCorrection = false;
                MotorManager_SetSpeed(Motor_Left, LeftSpeed);
                MotorManager_SetSpeed(Motor_Right, RightSpeed);
                
                MotorManager_StartMotor(Motor_Right, MotorManager_GetStepDirection(Motor_Right));
                MotorManager_StartMotor(Motor_Left, MotorManager_GetStepDirection(Motor_Left));
            }
        }
    

}


void RouteManager_SendCurrentRouteStep(void){
    uint8_t RetVal = 255;
    if(RouteManager_IsRouteOngoing()){
        RetVal = CurrentRouteStep;
    }
    IMUHandler_SetCurrentRouteStep(RetVal);
}

void RouteManager_ChargeSensorHandler(void){
    isChargeMagnetDetected = DBG4_GetValue();
    if (isChargeMagnetDetected == 1)
    {
        DBG3_SetLow();
    }
    else
    {
        DBG3_SetHigh();
    }
}
