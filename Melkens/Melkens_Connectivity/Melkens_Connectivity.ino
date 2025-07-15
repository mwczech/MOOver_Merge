#include <WiFiClient.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "src/Settings.h"
#include "src/ImuCommunication/ImuCommunication.h"
#include "src/MqttNode/MqttNode.h"
#include "src/WebHandler/WebHandler.h"

#ifdef BLE_SERIAL
   #include "src/BleSerial/BleSerial.h"
#endif

#include <AsyncTCP.h>          // https://github.com/ESP32Async/AsyncTCP.git
#include <ESPAsyncWebServer.h> // https://github.com/ESP32Async/ESPAsyncWebServer.git

#define LED_PIN_RED GPIO_NUM_20
#define LED_PIN_YELLOW GPIO_NUM_19

///#define dTIMEOUT_UART_ERROR 5000
#define dTIMEOUT_STATUS_BROKER 1000


#define FIXED_POINT_BITS 8
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)

#define DELAY_TIME 500  /* 200ms delay */ 
#define WIFI_CONNECTION_TIMEOUT 10 // [s] If it does not connect after this time, it switches to AP mode.
#define SOFT_AP_TIMEOUT 120 // [s] If after this time no one connects to the server, it resets.

const char* topic_data = "/moover/data"; //todo: duplicated

typedef struct {
  uint32_t time;
  bool  enabled;
} Timer;

Timer softApTimeout;

// typedef enum SendDataType_s{
//   dDataType_Route = 0,
//   dDataType_Manual,
//   dDataType_Standing,
//   dDataType_WrongData,
//   dDataType_NoConnPMB,
//   dDataType_NoConnIMU,
//   dDataType_NumOf
// }SendDataType;

IPAddress subnetIp(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

int port     = 1883;
JsonDocument JsonPayload_Route, JsonPayload_Manual,JsonPayload_Standing, JsonPayload_Status, JsonPayload_Charger;

uint32_t Counter1min;
int route_step;
bool SendStandingData, SendRouteData;
bool IsPMBConnected = false;
void(* resetFunc) (void) = 0;
static unsigned long StartTime, CurrentTime, LastIMUConnectedTime, brokerMsgStatusTime,previousTimestamp, currentTimestamp;

///SendDataType dataToSend, previousDataToSend;

#ifdef BLE_SERIAL
  BleSerial bleSerial;
#endif

void onAPStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Client connected:");
  softApTimeout.enabled = false;
}

void onAPStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Client disconeccted");
  delay(1000);
  ESP.restart();
}

void setup(void){
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_YELLOW, OUTPUT);

  Serial.begin(115200);
  ImuCommunication_Init();
  
  Serial.println("Firmware verion: "FIRMWARE_V);

  if(Wifi_Connect())
  {
     MqttNode_Connect(broker, port);

     Serial.println("");
     Serial.print("Connected to ");
     Serial.println(ssid);
     Serial.print("IP address: ");
     Serial.println(WiFi.localIP());
     digitalWrite(LED_PIN_RED, HIGH);
  }else
  {
    // Sets an open Access Point
    Serial.println("Set soft AP");
    String mac = WiFi.macAddress(); // mac address is only available in WIFI_STA mode 
    WiFi.mode(WIFI_AP);
    WiFi.softAP(("MELKENS-WIFI_" + mac).c_str(), NULL, 6);
    WiFi.onEvent(onAPStationConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    WiFi.onEvent(onAPStationDisconnected, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    softApTimeout.time = SOFT_AP_TIMEOUT;
    softApTimeout.enabled = true;
  }
  
  WebHandler_Init();
#ifdef BLE_SERIAL
  bleSerial.begin();
#endif  
  StartTime = millis();
  Serial.println("initialization done.");
  digitalWrite(LED_PIN_YELLOW, HIGH);

    previousTimestamp = millis();

    brokerMsgStatusTime = millis();

    LastIMUConnectedTime = millis();
}

void loop(void)
{
  static unsigned long timeBaseCurrent;
  static unsigned long timeBase10ms;
  static unsigned long timeBase100ms;
  static unsigned long timeBase500ms;
  static unsigned long timeBase1s;

    // LastIMUConnectedTime = millis();

    // currentTimestamp = millis();
    // if ((currentTimestamp - LastIMUConnectedTime) > dTIMEOUT_UART_ERROR)
    // {
    //   Serial.println("No comm from IMU");
    //   /* No connection to PMB for longer then dTIMEOUT_UART_ERROR miliseconds*/
    //   JsonPayload_Status["StatusIMU"] = "NC";
    //   JsonPayload_Status["StaMqttNode_PublishtusPMB"] = "NC";
    //   // Serial.println(currentTimestamp - LastIMUConnectedTime);
    //   mqttClient.beginMessage(topic_status);
    //   serializeJson(JsonPayload_Status, mqttClient);
    //   mqttClient.endMessage();
    //   dataToSend = dDataType_NoConnIMU;
    // }

    timeBaseCurrent = millis();
    //------- 10ms --------//
    if ((unsigned long)(timeBaseCurrent - timeBase10ms) >= (10U))
    {
      timeBase10ms = timeBaseCurrent;

      if(ImuCommunication_Rx(&Imu2EspFrame))
      {
#ifdef BLE_SERIAL
        bleSerial.println("IMU Data received");
#endif
/*
        Serial.println("IMU Data received");
        Serial.print("magnetBarStatus: "); Serial.println(Imu2EspFrame.magnetBarStatus);
        Serial.print("pmbConnection: "); Serial.println(Imu2EspFrame.pmbConnection);
        Serial.print("motorRightSpeed: "); Serial.println(Imu2EspFrame.motorRightSpeed);
        Serial.print("motorLeftSpeed: "); Serial.println(Imu2EspFrame.motorLeftSpeed);
        Serial.print("batteryVoltage: "); Serial.println(Imu2EspFrame.batteryVoltage);
        Serial.print("adcCurrent: "); Serial.println(Imu2EspFrame.adcCurrent);
        Serial.print("thumbleCurrent: "); Serial.println(Imu2EspFrame.thumbleCurrent);
        Serial.print("crcImu2PmbErrorCount: "); Serial.println(Imu2EspFrame.crcImu2PmbErrorCount);
        Serial.print("crcPmb2ImuErrorCount: "); Serial.println(Imu2EspFrame.crcPmb2ImuErrorCount);
        Serial.print("crcEsp2ImuErrorCount: "); Serial.println(Imu2EspFrame.crcEsp2ImuErrorCount);
*/      
      }

    } // end of 10ms

    //------- 100ms --------//
    if ((unsigned long)(timeBaseCurrent - timeBase100ms) >= (100U))
    {
      timeBase100ms = timeBaseCurrent;

      ImuCommunication_Tx(&Esp2ImuFrame);

    } // end of 100ms

    //------- 500ms -----//
    if ((unsigned long)(timeBaseCurrent - timeBase500ms) >= (500U))
    {
      timeBase500ms = timeBaseCurrent;

      if (WiFi.getMode() == WIFI_STA)
      {
        // todo: add connection protection in more elegant way
        // if (!mqttClient.connected())
        // {
        //   Serial.println("Disconnected from broker, reconnecting...");
        //   if (!MqttNode_Connect())
        //   {
        //     /* Check for WIFI connection is mqtt is broken */
        //     Wifi_Connect();
        //   }
        // }
        // else
        // {
        //   /* Send ACK to mqtt broker */
        //   mqttClient.poll();
        // }

        /* Time to send data over mqtt */
        // if (dataToSend == dDataType_NoConnIMU)
        // {
        // }
        // else
        // {
           MqttNode_Publish(topic_data); //todo://///////////////////////////////////////////////
        // }
      }
    } // end of 500ms

    //------- 1s --------//
    if ((unsigned long)(timeBaseCurrent - timeBase1s) >= (1000U))
    {
      timeBase1s = timeBaseCurrent;

      if (WiFi.getMode() == WIFI_AP && softApTimeout.enabled)
      {
        if (softApTimeout.time <= 0u)
        {
          ESP.restart();
        }
        else
        {
          softApTimeout.time--;
        }
      }

      WebHandler_SendData();

      ///Serial.print("moveX: "); Serial.println(Esp2ImuFrame.moveX);
      ///Serial.print("moveY: "); Serial.println(Esp2ImuFrame.moveY);
      WebHandler_CleanupClients();

    } // end of 1s

    // if (WiFi.getMode() == WIFI_STA && !MqttNode_IsConnected())
    // {
    //   Serial.println("Disconnected from broker, reconnecting...");
    //   if (!MqttNode_Connect(broker, port))
    //   {
    //     /* Check for WIFI connection is mqtt is broken */
    //     Wifi_Connect();
    //   }
    // }

    // Check for IMU connection
    // if ((millis() - LastIMUConnectedTime) > dTIMEOUT_UART_ERROR)
    // {
    //   Serial.println("No comm from IMU");
    //   /* No connection to PMB for longer then dTIMEOUT_UART_ERROR miliseconds*/
    //   JsonPayload_Status["StatusIMU"] = "NC";
    //   JsonPayload_Status["StatusPMB"] = "NC";
    //   mqttClient.beginMessage(topic_status);
    //   serializeJson(JsonPayload_Status, mqttClient);
    //   mqttClient.endMessage();
    //   dataToSend = dDataType_NoConnIMU;
    // }

#ifdef BLE_SERIAL    
    bleSerial.perform();
#endif
  } // end of loop ///////////////////////////////////////////////////////////////

  float CalculateDegreeFromPi(int32_t Degree)
  {
    float Angle;
    float Angle2;
    int32_t ScaledInput;
    
    ScaledInput = Degree * FIXED_POINT_SCALE / 3141;
    Angle = ScaledInput * 180 / FIXED_POINT_SCALE;
    Angle2 = (float)Degree / 3141.0f * 180.0f;
    /* Normalize angle (change position 0 angle, from -180/180 to -1/1 */
     if(Angle2 < 0){
         Angle2 = Angle2 + 180.0f;
         return -Angle2;

     }
     else{
        Angle2 = 180.0f - Angle2;
        return Angle2;
     }
}

double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}

// void sendPinStateOverMQTT(bool State){
//   if(State){
//     JsonPayload_Charger["State"] = 1;
//     mqttClient.beginMessage(topic_charger);
//     serializeJson(JsonPayload_Charger, mqttClient);
//     mqttClient.endMessage();      
//   }
//   else{
//     JsonPayload_Charger["State"] = 0;
//     mqttClient.beginMessage(topic_charger);
//     serializeJson(JsonPayload_Charger, mqttClient);
//     mqttClient.endMessage();   
//   }
// }

bool Wifi_Connect(){
  uint8_t timeout = WIFI_CONNECTION_TIMEOUT;

  WiFi.config(espIp, gatewayIp, subnetIp);
  WiFi.mode(WIFI_STA);

  if(ssid=="" || espIp=="")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Disconnected from Wifi...");
    if(!timeout)
    {
      Serial.println("Wifi connection timeout");
      return false;
    }
    timeout--;
  }

  return true;
}

