
#ifndef DIAHNOSTICS_HANDLER_H
#define	DIAHNOSTICS_HANDLER_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>

typedef enum Event{
dDebug_NoEvent = 0, //0
dDebug_LeftInverterConnected,   //1
dDebug_RightInverterConnected,     //2    
dDebug_Pause,   //3
dDebug_Undervoltage,    //4
dDebug_Safety,  //5
dDebug_IMUConnected,    //6
dDebug_MagnetsConnected,    //7
dDebug_Wifi_NC, //8
dDebug_PendantNC,   //9
dDebug_StopEmergancy,   //10
dDebug_MQTT_NC, //11
dDebug_NGROK_NC,    //12
dDebug_NumOf    //13
}DiagnosticsEvent_t;


void Diagnostics_SetEvent(DiagnosticsEvent_t Event);
void Diagnostics_GetEvent(void);
void Diagnostics_Perform100ms(void);

bool Diagnostics_IsInvertersReady(void);
bool Diagnostics_IsIMUReady(void);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* DIAHNOSTICS_HANDLER_H */

