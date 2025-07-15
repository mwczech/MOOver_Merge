#ifndef IMU_COMMUNICATION_H
#define IMU_COMMUNICATION_H

#include "src/Melkens_Lib/Types/MessageTypes.h"

#include <stdint.h>
#include <stdbool.h>

extern Imu2EspFrame_t Imu2EspFrame;
extern Esp2ImuFrame_t Esp2ImuFrame;

void ImuCommunication_Init(void);
bool ImuCommunication_Rx(Imu2EspFrame_t *frame);
void ImuCommunication_Tx(Esp2ImuFrame_t *frame);

#endif // IMU_COMMUNICATION_H
