#include "ImuCommunication.h"
#include "src/Melkens_Lib/CRC16/CRC16.h"
#include <Arduino.h>
#include <HardwareSerial.h>


#define IMU_SERIAL Serial1

#define IMU_TX GPIO_NUM_17
#define IMU_RX GPIO_NUM_18
#define IMU_BAUD    115200

#define IMU_SERIAL_TIMEOUT 5000 // Timeout for IMU serial communication

Imu2EspFrame_t Imu2EspFrame;
Esp2ImuFrame_t Esp2ImuFrame;

void ImuCommunication_Init(void)
{
    IMU_SERIAL.begin(IMU_BAUD, SERIAL_8N1, IMU_RX, IMU_TX);
    //IMU_SERIAL.setTimeout(dTIMEOUT_UART_ERROR); //todo: check if needed
    IMU_SERIAL.setRxBufferSize(sizeof(Imu2EspFrame_t));
    //IMU_SERIAL.setWriteTimeout(IMU_SERIAL_TIMEOUT);
}

bool ImuCommunication_Rx(Imu2EspFrame_t *frame)
{
    if (IMU_SERIAL.available() >= sizeof(Imu2EspFrame_t))
    {
        IMU_SERIAL.readBytes((uint8_t*)frame, sizeof(Imu2EspFrame_t));

        if ((frame->crc) == CRC16((uint8_t *)frame, sizeof(Imu2EspFrame_t) - sizeof(frame->crc)))
        {
            return true; // Valid frame received
        }
        else
        {
            frame->crcEsp2ImuErrorCount++; // Increment error count for CRC mismatch
            Serial.println("Invalid IMU frame received");
            return false; // Invalid frame
        }
    }
    return false; // Not enough data available
}

void ImuCommunication_Tx(Esp2ImuFrame_t *frame)
{
    frame->crc = CRC16((uint8_t*)frame, sizeof(Esp2ImuFrame_t) - sizeof(frame->crc));
    IMU_SERIAL.write((uint8_t *)frame, sizeof(Esp2ImuFrame_t));
    IMU_SERIAL.flush(); // Ensure all data is sent
}
