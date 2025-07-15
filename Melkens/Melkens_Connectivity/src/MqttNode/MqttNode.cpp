
#include "MqttNode.h"
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

JsonDocument payloadDoc;

///const char* topic_data = "/moover/data";
const char* topic_standing = "/moover/data/standing";
const char* topic_manual = "/moover/data/manual";
const char* topic_route = "/moover/data/route";
const char* topic_status = "/moover/status";
const char* topic_charger = "/moover/charger";

/*
MqttNode_Send()
{
    // Handle rest of frames
    // Send status message

    if ((currentTimestamp - brokerMsgStatusTime) > dTIMEOUT_STATUS_BROKER)
    {
        brokerMsgStatusTime = millis();
        mqttClient.beginMessage(topic_status);
        serializeJson(JsonPayload_Status, mqttClient);
        mqttClient.endMessage();
        //         Serial.print("Data status sent to broker: ");
        //          serializeJson(JsonPayload_Status, Serial);
        //          Serial.println(" ");
    }

    switch (dataToSend)
    {
    case dDataType_Route:
        mqttClient.beginMessage(topic_route);
        serializeJson(JsonPayload_Route, mqttClient);
        mqttClient.endMessage();
        break;
    case dDataType_Manual:
        mqttClient.beginMessage(topic_manual);
        serializeJson(JsonPayload_Manual, mqttClient);
        mqttClient.endMessage();
        break;
    case dDataType_Standing:
        mqttClient.beginMessage(topic_standing);
        serializeJson(JsonPayload_Standing, mqttClient);
        mqttClient.endMessage();

        Serial.print("Data sent to broker: ");
        serializeJson(JsonPayload_Standing, Serial);
        Serial.println(" ");
        break;
    default:
        break;
    }
}
*/

// String SerializeToInflux(const MqttPublish& msg) {
//     ///StaticJsonDocument<256> doc;
//     doc["topic"] = msg.topic;
//     doc["payload"] = msg.payload;
//     doc["timestamp"] = msg.timestamp;
//     // Dodaj inne pola jeśli są w strukturze, np. device
//     if (!msg.device.empty()) {
//         doc["device"] = msg.device;
//     }
//     String output;
//     serializeJson(doc, output);
//     serializeJson(JsonPayload_Route, mqttClient);
//     return output;
// }

bool MqttNode_Connect(IPAddress broker, int port)
{
    uint8_t RepeatCount = 0;
    while (!mqttClient.connect(broker, port))
    { // todo: this while killing the web server!
        delay(500);
        if (RepeatCount >= 10)
        {
            Serial.print("Reached mqtt broker connection timeout");
            return 0;
        }
        else
        {
            Serial.print("Retrying mqtt connection...");
            Serial.println(10 - RepeatCount);
            RepeatCount = RepeatCount + 1;
        }
    }
    Serial.println("Connection to broker established...");
    return true;
}

void MqttNode_Publish(const char *publishTopic)
{
    JsonDocument doc;

    JsonObject doc_0 = doc.add<JsonObject>();

    doc_0["magnetBarStatus"] = Imu2EspFrame.magnetBarStatus;
    doc_0["pmbConnection"] = Imu2EspFrame.pmbConnection;
    doc_0["motorRightSpeed"] = Imu2EspFrame.motorRightSpeed;
    doc_0["motorLeftSpeed"] = Imu2EspFrame.motorLeftSpeed;
    doc_0["batteryVoltage"] = Imu2EspFrame.batteryVoltage;
    doc_0["adcCurrent"] = Imu2EspFrame.adcCurrent;
    doc_0["thumbleCurrent"] = Imu2EspFrame.thumbleCurrent;
    doc_0["crcImu2PmbErrorCount"] = Imu2EspFrame.crcImu2PmbErrorCount;
    doc_0["crcPmb2ImuErrorCount"] = Imu2EspFrame.crcPmb2ImuErrorCount;
    doc_0["crcEsp2ImuErrorCount"] = Imu2EspFrame.crcEsp2ImuErrorCount;

    doc[1]["tag1"] = "Imu2EspFrame";

    doc.shrinkToFit(); // optional

    mqttClient.beginMessage(publishTopic);
    serializeJson(doc, mqttClient);
    mqttClient.endMessage();
}