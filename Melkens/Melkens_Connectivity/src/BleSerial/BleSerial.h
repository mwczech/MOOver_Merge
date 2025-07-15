#ifndef BLESERIAL_H
#define BLESERIAL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

// UART service UUID
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class BleSerial
{
public:
    void begin();
    void perform();
    void sendData(const char *data, size_t length);
    void print(const char *msg);
    void println(const char *msg);
};

#ifdef __cplusplus
    }
#endif

#endif // BLESERIAL_H
