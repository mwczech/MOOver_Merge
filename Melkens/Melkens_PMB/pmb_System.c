#include <libpic30.h>
#include <stdbool.h>
#include <xc.h>
#include "mcc_generated_files/pin_manager.h"

#include "Tools/Timer.h"
#include "pmb_System.h"
#include "DriveIndicator.h"
#include "Tools/Tools.h"
#include "Tools/Timer.h"

uint8_t DevVar = 0;
uint8_t DevVar2 = 0;

#define INITIAL 2
#define ENABLED 1
#define DISABLED 0
#define dPowerStageSequenceTimeout 250U

typedef enum SequenceSteps_t{
    Step_SoftStartHigh = 0,
    Step_StatusesToDigitalHigh,        
    Step_EnableHigh,
    Step_RstLow,
    Step_RstHigh,
    Step_RstLowSecond,
    Step_StatusesToAnalog,
    Step_EnableLow,
    Step_SoftStartLow,
    Step_NumOf
}SequenceSteps;

typedef struct PowerSequence_t{
    SequenceSteps CurrentStep;
    bool Enable;
    bool Request;
    Timer TimeToEnd;
}PowerSequence;

PowerSequence PowerRails[Sequence_NumOf];

int SoftStartCnt = 0;
/* TODO: Unused - CalibrationCurrent it not used anywhere, but calculated */
uint16_t CalibratedCurrent = 0;
Timer PowerStageTimer;

void System_SetEnablePin(PowerSequenceNames Name, bool Condition);
void System_SoftStartSet(PowerSequenceNames Name, bool Condition);
void System_SetRstPin(PowerSequenceNames Name, bool Condition);
void System_StatusesToDigitalHigh(PowerSequenceNames Name);
void System_StatusesToAnalog(PowerSequenceNames Name);

void System_Init(void)
{
    SoftStartCnt = 0;
}

void System_PowerRailRequestSequence(PowerSequenceNames Name){
    TimerSetCounter(&PowerRails[Name].TimeToEnd, 500);
    PowerRails[Name].CurrentStep = Step_SoftStartHigh;
    PowerRails[Name].Request = true;
    
    if (Name == Sequence_PowerStageOff)
    {
        __asm("nop");
    }
}

void System_Perform1ms(void){
    for(PowerSequenceNames Name=Sequence_PowerStageOn; Name < Sequence_NumOf; Name++ ){
        if(PowerRails[Name].Request){
            TimerTick(&PowerRails[Name].TimeToEnd);
            if( TimerIsExpired(&PowerRails[Name].TimeToEnd) ){
                TimerSetCounter(&PowerRails[Name].TimeToEnd, dPowerStageSequenceTimeout);
                switch(PowerRails[Name].CurrentStep){
                    case Step_SoftStartHigh:
                            System_SoftStartSet(Name, true);
                            PowerRails[Name].CurrentStep = Step_StatusesToDigitalHigh;
                            PowerRails[Name].Enable = false;
                            break;
                    case Step_StatusesToDigitalHigh:
                            System_StatusesToDigitalHigh(Name);   
                            PowerRails[Name].CurrentStep = Step_EnableHigh;
                            break;  
                    case Step_EnableHigh:
                            System_SetEnablePin(Name, true);
                            PowerRails[Name].CurrentStep = Step_RstLow;
                            break;
                    case Step_RstLow:
                            System_SetRstPin(Name, false);
                            PowerRails[Name].CurrentStep = Step_RstHigh;
                            break;
                    case Step_RstHigh:
                            System_SetRstPin(Name, true);
                            PowerRails[Name].CurrentStep = Step_RstLowSecond;
                            break;
                    case Step_RstLowSecond:
                            System_SetRstPin(Name, false);
                            PowerRails[Name].CurrentStep = Step_StatusesToAnalog;
                            break;   
                    case Step_StatusesToAnalog:
                            System_StatusesToAnalog(Name);
                            PowerRails[Name].CurrentStep = Step_EnableLow;
                            break;
                    case Step_EnableLow:
                            System_SetEnablePin(Name, false);
                            PowerRails[Name].CurrentStep = Step_SoftStartLow;
                            break;
                    case Step_SoftStartLow:
                            System_SoftStartSet(Name, false);
                            PowerRails[Name].CurrentStep = Step_NumOf;
                            PowerRails[Name].Request = false;
                            PowerRails[Name].Enable = true;
                            break;  
                    default:
                        break;
                }
            }
            break;
        }
    }
}

void System_SoftStartSet(PowerSequenceNames Name, bool Condition){
    if(Name == Sequence_PowerStageOn){
        if(Condition)
            Soft_Start_EN_SetHigh();
        else
            Soft_Start_EN_SetLow();
    }
    else
    {
        /* Soft start handling only for PowerStage */
    }
}

void System_SetEnablePin(PowerSequenceNames Name, bool Condition){
    if(Name == Sequence_PowerStageOn){
        if(Condition){
            EN_SafSwA_SetHigh();
            EN_SafSwB_SetHigh();
        }
        else{
            EN_SafSwA_SetLow();
            EN_SafSwB_SetLow();
        }
    }
    else if (Name == Sequence_PowerStageOff)
    {
        if(Condition){
            EN_SafSwA_SetLow();
            EN_SafSwB_SetLow();
        }
        else{
            EN_SafSwA_SetLow();
            EN_SafSwB_SetLow();
        }
    }   
    else if (Name == Sequence_ChargerOn)
    {
        if(Condition){
            EN_Char_SetHigh();
        }
        else{
            EN_Char_SetLow();
        }
    } 
    else if (Name == Sequence_ChargerOff)
    {
        if(Condition){
            EN_Char_SetLow();
        }
        else{
            EN_Char_SetLow();
        }
    } 
}

void System_SetRstPin(PowerSequenceNames Name, bool Condition){
    DevVar = PowerRails[2].Request;
    DevVar2 = Name;
    __asm("nop");
    if(Name == Sequence_PowerStageOn || Sequence_PowerStageOff){
        if(Condition){
            SafSwA_Rst_SetHigh();
            SafSwB_Rst_SetHigh();
        }
        else{
            SafSwA_Rst_SetLow();
            SafSwB_Rst_SetLow();
        }
    }
    if (Name == Sequence_ChargerOn || Sequence_ChargerOff)
    {
        __asm("nop");
        if(Condition){
            Char_Rst_SetHigh();
        }
        else{
            Char_Rst_SetLow();
        }
    }    
}

void System_DisableRail(PowerSequenceNames Name){
    PowerRails[Name].Enable = false;
    System_SetEnablePin(Name, false);
}

bool System_GetPowerRailState(void){
    return PowerRails[Sequence_PowerStageOn].Enable;
}

bool System_GetChargerState(void){
    return PowerRails[Sequence_ChargerOn].Enable;
}

uint32_t CalculateCurrent (uint16_t ADCcnt)
{
    uint32_t cnt, Amp_Val;
    uint32_t Current_Val;
    cnt = ADCcnt;
    
    if(true == PowerRails[Sequence_ChargerOn].Enable && false == PowerRails[Sequence_PowerStageOn].Enable)
    {
        //taken from linear function equation based on 3 points
        Current_Val = 131 - (cnt  * 100 / 1480);
        Amp_Val = Current_Val;       
    }
    else if(false == PowerRails[Sequence_ChargerOn].Enable && true == PowerRails[Sequence_PowerStageOn].Enable)
    {
        //taken from linear function equation based on 4 points
        Current_Val = cnt  * 100 / 1484 - 131;
        Amp_Val = Current_Val; 
    }
    else if(false == PowerRails[Sequence_ChargerOn].Enable && false == PowerRails[Sequence_PowerStageOn].Enable)
    {
        CalibratedCurrent = cnt;
        Amp_Val = 0;
    }
   
    return Amp_Val;
    
}

uint32_t CalculateVoltage (uint16_t ADCcnt)
{
    uint32_t cnt, Vol_Val;
    cnt = ADCcnt;
    //1 cnt is 0,8057
    //24V: div ratio resistors 47k and 6k8 -> k = 0,1264
    //48V: div ratio resistors 47k and 2k49 -> k = 0,05031
    //cnt/k -> 6,3742 for 24V
    //cnt/k -> 16,0147 for 48V
    //send to display in form XXXX i.e. 20,54V is being sent as 2054
    
    #if COMPILE_SWITCH_MOONION 
        //48V:
        Vol_Val = cnt * 1601 / 1000;
    #else
        //24V: 
        Vol_Val = cnt * 637 / 1000;
    #endif 
    
    return Vol_Val;
}

void System_StatusesToDigitalHigh(PowerSequenceNames Name)
{
    if(Name == Sequence_PowerStageOn || Sequence_PowerStageOff){
        StatSw1_SetDigitalOutput();
        StatSw2_SetDigitalOutput();
        StatSw3_SetDigitalOutput();
        StatSw4_SetDigitalOutput();
        
        StatSw1_SetHigh();
        StatSw2_SetHigh();
        StatSw3_SetHigh();
        StatSw4_SetHigh();
    }
}

void System_StatusesToAnalog(PowerSequenceNames Name)
{
    if(Name == Sequence_PowerStageOn || Sequence_PowerStageOff){ 
        StatSw1_SetDigitalInput();
        StatSw2_SetDigitalInput();
        StatSw3_SetDigitalInput();
        StatSw4_SetDigitalInput();
        
        StatSw1_SetAnalog();
        StatSw2_SetAnalog();
        StatSw3_SetAnalog();
        StatSw4_SetAnalog();
    }
}