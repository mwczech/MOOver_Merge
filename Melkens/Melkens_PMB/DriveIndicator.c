#include "DriveIndicator.h"
#include "Tools/Timer.h"
#include "pmb_System.h"

#include "stdbool.h"
#include "mcc_generated_files/pin_manager.h"

// #define dTimer_3s 3000U
// #define dTimer_05s 500U


Timer DriveIndicator_Lamp,DriveIndicator_Buzzer;
// bool IsBuzzerEnabled;
// bool IsLampEnabled;

void DriveIndicator_Init(void){
    //TimerSetCounter(&DriveIndicator, dTimer_3s);
}

// void DriveIndicator_BuzzerOnly_Init(void)
// {
//     TimerSetCounter(&DriveIndicator, dTimer_05s);
// }

// void DriveIndicator_LampOnly_Init(void)
// {
//     TimerSetCounter(&DriveIndicator, dTimer_05s);
// }


void DriveIndicator_SetIndication(uint32_t BuzzerMs, uint32_t LampMs){
    if (BuzzerMs > 0){
        Buzzer_SetHigh();
        TimerSetCounter(&DriveIndicator_Buzzer, BuzzerMs);

    }
    if (LampMs > 0){
        Warning_Light_SetHigh();
        TimerSetCounter(&DriveIndicator_Lamp, LampMs);
    }
}

void DriveIndicator_1msPerform(void){
    if(!TimerIsExpired(&DriveIndicator_Lamp)){
        TimerTick(&DriveIndicator_Lamp);
        if( TimerIsExpired(&DriveIndicator_Lamp) ){
            Warning_Light_SetLow();
        }
    }

    if(!TimerIsExpired(&DriveIndicator_Buzzer)){
        TimerTick(&DriveIndicator_Buzzer);
        if( TimerIsExpired(&DriveIndicator_Buzzer) ){
            Buzzer_SetLow();
        }
    }
}


// void DriveIndicator_SetEnable(void){
//     /* Inejblowanie buzzera i lampy */
//     IsBuzzerEnabled = true;
//     IsLampEnabled = true;
//     TimerSetCounter(&DriveIndicator, dTimer_3s);
//     Buzzer_SetHigh();
//     Warning_Light_SetHigh();
    
// }

// void DriveIndicator_BuzzerOnly_SetEnable(void)
// {
//     Buzzer_SetHigh();
// }

// void DriveIndicator_LampOnly_SetEnable(void)
// {
//     IsBuzzerEnabled = true;
//     IsLampEnabled = true;
//     TimerSetCounter(&DriveIndicator, dTimer_05s);
//     Warning_Light_SetHigh();
// }
// bool IsIndicatorEnable(void){
//     return IsBuzzerEnabled && IsLampEnabled;
// }


void DriveIndicator_SetDisable(IndicationType Indication){
    if( dIndicationType_Buzzer == Indication ){
        TimerSetCounter(&DriveIndicator_Buzzer, 0);
        Buzzer_SetLow();
    }
    else if(dIndicationType_Lamp == Indication){
        TimerSetCounter(&DriveIndicator_Lamp, 0);
        Warning_Light_SetLow();
    }
    else if(dIndicationType_Both == Indication){
        TimerSetCounter(&DriveIndicator_Lamp, 0);
        TimerSetCounter(&DriveIndicator_Buzzer, 0);
        Buzzer_SetLow();
        Warning_Light_SetLow();       
    }
}

bool DriveIndicator_IsFinishedIndication(void){
    bool Lamp = false, Buzzer = false;
    if(TimerIsExpired(&DriveIndicator_Lamp)){
        Buzzer = true;
    }
    if(TimerIsExpired(&DriveIndicator_Buzzer)){
        Lamp = true;
    }

    return Buzzer & Lamp;
}