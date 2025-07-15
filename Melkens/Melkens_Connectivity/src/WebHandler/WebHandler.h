#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include <Arduino.h>
#include <HardwareSerial.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern String ssid;
extern String password;
extern IPAddress broker;
extern IPAddress espIp;
extern IPAddress gatewayIp;

extern const char* configFile;

void WebHandler_Init(void);
void WebHandler_SendData(void);
void WebHandler_CleanupClients(void);

#ifdef __cplusplus
}
#endif

#endif // WEBHANDLER_H
