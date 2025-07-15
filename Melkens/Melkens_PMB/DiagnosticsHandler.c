#include "DiagnosticsHandler.h"
#include "pmb_Settings.h"
#include "Tools/Timer.h"

#include <stdbool.h>

bool CurrentDiagnosticsEvent[dDebug_NumOf];
bool Diagnostics[dDebug_NumOf];

Timer InverterConnectionTimer;
Timer IMUConnectionTimer;
Timer MagnetsConnectionTimer;

void Diagnostics_Init(void){
    Diagnostics[dDebug_LeftInverterConnected] = false;
    Diagnostics[dDebug_RightInverterConnected] = false;
    /* Division by 100 due to performing timer in 100ms loops */
    TimerSetCounter(&InverterConnectionTimer, dMOTOR_WHEEL_DISCONNECT_TIMEOU_MS / 100);
    TimerSetCounter(&IMUConnectionTimer, dIMU_DISCONNECT_TIMEOUT_MS / 100);
    TimerSetCounter(&MagnetsConnectionTimer, dMAGNETS_DISCONNECT_TIMEOUT_MS / 100);

}

void Diagnostics_Perform100ms(void){
    /* TODO: wrap if's into for loop and do this more automatically */
    
    /* Left inverter disconnected */
    if( CurrentDiagnosticsEvent[dDebug_LeftInverterConnected] == true ){
        CurrentDiagnosticsEvent[dDebug_LeftInverterConnected] = false;
        Diagnostics[dDebug_LeftInverterConnected] = true;
        TimerSetCounter(&InverterConnectionTimer, dMOTOR_WHEEL_DISCONNECT_TIMEOU_MS / 100);
    }
    else{
        if(Diagnostics[dDebug_LeftInverterConnected] == true){
            if( TimerIsExpired(&InverterConnectionTimer)){
                TimerSetCounter(&InverterConnectionTimer, dMOTOR_WHEEL_DISCONNECT_TIMEOU_MS / 100);
                Diagnostics[dDebug_LeftInverterConnected] = false;
            }
            else{
                TimerTick(&InverterConnectionTimer);
            }
        }
    }

    /* Right inverter disconnected */
    if( CurrentDiagnosticsEvent[dDebug_RightInverterConnected] == true ){
        CurrentDiagnosticsEvent[dDebug_RightInverterConnected] = false;
        Diagnostics[dDebug_RightInverterConnected] = true;
        TimerSetCounter(&InverterConnectionTimer, dMOTOR_WHEEL_DISCONNECT_TIMEOU_MS / 100);

    }
    else{
        if(Diagnostics[dDebug_RightInverterConnected] == true){
            if( TimerIsExpired(&InverterConnectionTimer)){
                TimerSetCounter(&InverterConnectionTimer, dMOTOR_WHEEL_DISCONNECT_TIMEOU_MS / 100);
                Diagnostics[dDebug_RightInverterConnected] = false;
            }
            else{
                TimerTick(&InverterConnectionTimer);
            }
        }
    }

    /* IMU disconnected */
    if( CurrentDiagnosticsEvent[dDebug_IMUConnected] == true ){
        CurrentDiagnosticsEvent[dDebug_IMUConnected] = false;
        Diagnostics[dDebug_IMUConnected] = true;
        TimerSetCounter(&IMUConnectionTimer, dIMU_DISCONNECT_TIMEOUT_MS / 100);

    }
    else{
        if(Diagnostics[dDebug_IMUConnected] == true){
            if( TimerIsExpired(&IMUConnectionTimer)){
                TimerSetCounter(&IMUConnectionTimer, dIMU_DISCONNECT_TIMEOUT_MS / 100);
                Diagnostics[dDebug_IMUConnected] = false;
            }
            else{
                TimerTick(&IMUConnectionTimer);
            }
        }
    }

    /* Magnets disconnected */
    if( CurrentDiagnosticsEvent[dDebug_MagnetsConnected] == true ){
        CurrentDiagnosticsEvent[dDebug_MagnetsConnected] = false;
        Diagnostics[dDebug_MagnetsConnected] = true;
        TimerSetCounter(&MagnetsConnectionTimer, dMAGNETS_DISCONNECT_TIMEOUT_MS / 100);

    }
    else{
        if(Diagnostics[dDebug_MagnetsConnected] == true){
            if( TimerIsExpired(&MagnetsConnectionTimer)){
                TimerSetCounter(&MagnetsConnectionTimer, dMAGNETS_DISCONNECT_TIMEOUT_MS / 100);
                Diagnostics[dDebug_MagnetsConnected] = false;
            }
            else{
                TimerTick(&MagnetsConnectionTimer);
            }
        }
    }
}

void Diagnostics_SetEvent(DiagnosticsEvent_t Event){
    CurrentDiagnosticsEvent[Event] = true;
}

bool Diagnostics_GetState(DiagnosticsEvent_t Event){
    return Diagnostics[Event];
}

bool Diagnostics_IsInvertersReady(void){
    return Diagnostics[dDebug_LeftInverterConnected] && Diagnostics[dDebug_RightInverterConnected];
}

bool Diagnostics_IsIMUReady(void){
    return Diagnostics[dDebug_IMUConnected] && Diagnostics[dDebug_MagnetsConnected];
}
