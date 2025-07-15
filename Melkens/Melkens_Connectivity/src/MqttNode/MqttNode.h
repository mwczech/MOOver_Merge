#ifndef MQTTNODE_H
#define MQTTNODE_H

#include <stdint.h>
#include <stdbool.h>
#include <WiFiClient.h>
#include "src/ImuCommunication/ImuCommunication.h"

bool MqttNode_Connect(IPAddress broker, int port);
void MqttNode_Publish(const char* publishTopic);

#endif // MQTTNODE_H
