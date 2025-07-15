#include "../pmb_System.h"
#include "BatteryManager.h"
#include "../AnalogHandler/AnalogHandler.h"
#include "../pmb_Settings.h"
#include "../Tools/Timer.h"

#define BATTERY_INIT_5s 30 /* 1s of circuit and ADC stabilisation */


Timer InitTimer;
BatteryLevel CurrentBatteryLevel, PreviousBatteryLevel;
bool IsDeviceDisabled;

BatteryLevel BatteryManager_CalculateLevel(uint32_t Voltage);
void BatteryManager_HandleStates(void);

void BatteryManager_Perform100ms(void){
    uint16_t Voltage_ADC;
    uint32_t Voltage_Real;
    Voltage_ADC = AnalogHandler_GetADCFiltered(BAT_STATUS);
    Voltage_Real = CalculateVoltage(Voltage_ADC);
    BatteryLevel CalculatedBatteryLevel;
    
    switch(CurrentBatteryLevel){
        case BatteryLevel_Init:
            TimerSetCounter(&InitTimer, BATTERY_INIT_5s);
            CurrentBatteryLevel = BatteryLevel_Stabilisation;
            break;
        case BatteryLevel_Stabilisation:
            if(!TimerIsExpired(&InitTimer)){
                TimerTick(&InitTimer);
                if( TimerIsExpired(&InitTimer) ){
                    CurrentBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real);
                }
            }          
            break;
        case BatteryLevel_Good:
            CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real);
            if( CalculatedBatteryLevel == BatteryLevel_Low ){
                /* Check lower threshold with hysteresis */
                CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real + dBATTERY_HYSTERESIS);
                if( CalculatedBatteryLevel == BatteryLevel_Low ){
                    CurrentBatteryLevel = BatteryLevel_Low;
                }
            }
            else if( CalculatedBatteryLevel == BatteryLevel_Overvoltage ){
                /* Check upper threshold with hysteresis */
                CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real - dBATTERY_HYSTERESIS);
                if( CalculatedBatteryLevel == BatteryLevel_Overvoltage ){
                    CurrentBatteryLevel = BatteryLevel_Overvoltage;
                }
            }else{
                CurrentBatteryLevel = CalculatedBatteryLevel;
            }
            break;
        case BatteryLevel_Low:
            CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real);
            if( CalculatedBatteryLevel == BatteryLevel_Critical ){
                CurrentBatteryLevel = BatteryLevel_Critical;
            }
            else if( CalculatedBatteryLevel == BatteryLevel_Good ){
                CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real - dBATTERY_HYSTERESIS);
                if( CalculatedBatteryLevel == BatteryLevel_Good){
                    CurrentBatteryLevel = BatteryLevel_Good;
                }
            }
            else{
                CurrentBatteryLevel = CalculatedBatteryLevel;
            }
            break;
        case BatteryLevel_Critical:
            CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real - dBATTERY_HYSTERESIS);
            if( CalculatedBatteryLevel == BatteryLevel_Low ){
                CurrentBatteryLevel = BatteryLevel_Low;
            }
            else{
                CurrentBatteryLevel = CalculatedBatteryLevel;
            }
            break;
        case BatteryLevel_Overvoltage:
            CalculatedBatteryLevel = BatteryManager_CalculateLevel(Voltage_Real + dBATTERY_HYSTERESIS);
            if( CalculatedBatteryLevel == BatteryLevel_Good ){
                CurrentBatteryLevel = BatteryLevel_Good;
            }
            else{
                CurrentBatteryLevel = CalculatedBatteryLevel;
            }
            break;
            
        default:
            break;
    }
    
    BatteryManager_HandleStates();
    PreviousBatteryLevel = CurrentBatteryLevel;
}

void BatteryManager_ResetBattery(void){
    CurrentBatteryLevel = BatteryLevel_Init;
    PreviousBatteryLevel = BatteryLevel_Init;

}

BatteryLevel BatteryManager_CalculateLevel(uint32_t Voltage){
    BatteryLevel RetVal = BatteryLevel_Init;

    if( Voltage > dBATTERU_CRITICAL_VOLTAGE && Voltage < dBATTERY_OVERVOLTAGE ){
        if( Voltage > dBATTERY_LOW_VOLTAGE )
            RetVal = BatteryLevel_Good;
        else
            RetVal = BatteryLevel_Low;
    }
    else if ( Voltage < dBATTERU_CRITICAL_VOLTAGE ) 
        RetVal = BatteryLevel_Critical;
    else if( Voltage > dBATTERY_OVERVOLTAGE )
        RetVal = BatteryLevel_Overvoltage;
    
    return RetVal;
}

void BatteryManager_HandleStates(){
    /* Current requirements:
    *   1. When overvoltage, ChargerOff is triggered
    *   2. When Low voltage, cannot trigger route
    *   3. When critical voltage - PowerOff and ChargerOn is trigerred. All buttons are blocked
     */
    if( CurrentBatteryLevel == BatteryLevel_Overvoltage 
    &&  PreviousBatteryLevel != BatteryLevel_Overvoltage){
        /* 1. */
        System_PowerRailRequestSequence(Sequence_ChargerOff);
    }

    if(CurrentBatteryLevel == BatteryLevel_Overvoltage){
        /* 2. Currently do nothing except block setting route in RouteManager */
    }

    if( CurrentBatteryLevel == BatteryLevel_Critical ){
        /* 3. Go to disable state */
        if(!IsDeviceDisabled){
             System_PowerRailRequestSequence(Sequence_PowerStageOff);
             System_PowerRailRequestSequence(Sequence_ChargerOff);

             IsDeviceDisabled = true;
        }
    }
    else{
        /* 3. If not critical voltage, device is enabled always */
       IsDeviceDisabled = false; 
    }
}

 BatteryLevel BatteryManager_GetBatteryLevel(void){
     return CurrentBatteryLevel;
 }