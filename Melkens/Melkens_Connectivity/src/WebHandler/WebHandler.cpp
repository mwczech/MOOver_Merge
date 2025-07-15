
#include "WebHandler.h"
#include <AsyncTCP.h>          // https://github.com/ESP32Async/AsyncTCP.git
#include <ESPAsyncWebServer.h> // https://github.com/ESP32Async/ESPAsyncWebServer.git
///#include "src/WebPage/http_header.h"
#include "src/WebPage/http_settings.h"
#include "src/WebPage/IndexHtml.h"
#include "src/Settings.h"
#include "src/ImuCommunication/ImuCommunication.h"
#include <LittleFS.h>
#include <Update.h>
#include <ArduinoJson.h>

String ssid;
String password;
IPAddress broker;
IPAddress espIp;
IPAddress gatewayIp;

JsonDocument doc;
const char *configFile = "/config.json";

// Default config file in JSON format
const char* defaultConfig = R"rawliteral(
  {
    "wifi": {
      "ssid": "Office",
      "password": "2023"
    },
    "mqtt": {
      "broker": "192.168.2.200"
    },
    "network": {
      "espIp": "192.168.2.201",
      "gatewayIp": "192.168.2.1",
      "subnetIp": "255.255.255.0"
    }
  }
  )rawliteral";

AsyncWebServer server(80); //Default port number
AsyncWebSocket ws("/ws");

void handleRoot(AsyncWebServerRequest *request)
{
    Serial.println("Handling root");
    request->send(200, "text/html", html);
}

void handleSettings(AsyncWebServerRequest *request)
{
    Serial.println("Handling settings");
    request->send(200, "text/html", settings_html);
}

// XML page to listen for motor commands
void handleMotors(AsyncWebServerRequest *request)
{
    String motorState = "OFF";
    String t_state = request->arg("motorState"); // Refer  xhttp.open("GET", "setMotors?motorState="+motorData, true);
    Serial.println(t_state);

    if (t_state.startsWith("V")) // Drive Forward (UP Arrow)
    {
        Serial1.print("V");
        Serial1.write((uint8_t)t_state.substring(1).toInt());
        Serial.println(t_state.substring(1).toInt());
    }
    else if (t_state.startsWith("X"))
    {
        Serial1.print("X");
        Serial1.write((uint8_t)t_state.substring(1).toInt());
        Serial.println(t_state.substring(1).toInt());
    }
    else if (t_state.startsWith("W"))
    {
        if (t_state.substring(1) == "H")
        {
            Serial.println("Stan wysoki");
            /// sendPinStateOverMQTT(true); //todo
        }
        else if (t_state.substring(1) == "L")
        {
            Serial.println("Stan niski");
            /// sendPinStateOverMQTT(false); //todo
        }
        Serial1.print(t_state);
    }
    else
    {
        Serial1.print(t_state);
    }
    request->send(200, "text/plain", motorState); // Send web page
}

void handleSubmit(AsyncWebServerRequest *request)
{
    ssid = request->arg("ssid");
    password = request->arg("password");
    broker.fromString(request->arg("broker"));
    espIp.fromString(request->arg("espIp"));
    gatewayIp.fromString(request->arg("gatewayIp"));

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    Serial.print("Broker: ");
    Serial.println(broker);
    Serial.print("ESP IP: ");
    Serial.println(espIp);
    Serial.print("Gateway IP: ");
    Serial.println(gatewayIp);

    doc["wifi"]["ssid"] = ssid;
    doc["wifi"]["password"] = password;
    doc["mqtt"]["broker"] = broker.toString();
    doc["network"]["espIp"] = espIp.toString();
    doc["network"]["gatewayIp"] = gatewayIp.toString();

    File file = LittleFS.open(configFile, "w");
    if (!file)
    {
        Serial.printf("Failed to open file for writing: %s\n", configFile);
    }

    if (serializeJson(doc, file) == 0)
    {
        Serial.println("Failed to write JSON to file");
    }
    else
    {
        Serial.println("Configuration saved successfully");
    }
    file.close();

    request->send(200, "text/plain", "Config uploaded");
    delay(1000);
    ESP.restart();
}

void handleUpdateEsp(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    static size_t updateSize = 0;
    if (index == 0)
    {
        Serial.printf("Update: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        {
            Update.printError(Serial);
        }
        updateSize = 0;
    }
    if (len)
    {
        if (Update.write(data, len) != len)
        {
            Update.printError(Serial);
        }
        updateSize += len;
    }
    if (final)
    {
        if (Update.end(true))
        {
            Serial.printf("Update Success: %u\nRebooting...\n", updateSize);
            request->send(200, "text/plain", "Update Success. Rebooting...");
            delay(1000);
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
            request->send(200, "text/plain", "Update Failed");
        }
    }
}

void handleUpdateConfig(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    static File file;
    if (index == 0)
    {
        file = LittleFS.open(configFile, "w");
    }
    if (file)
    {
        file.write(data, len);
    }
    if (final)
    {
        if (file)
            file.close();
        request->send(200, "text/plain", "File uploaded successfully");
        delay(1000);
        ESP.restart();
    }
}

/* cannot handle request so return 404 */
void handleNotFound(AsyncWebServerRequest *request)
{
    String message = "File Not Found\n\n";
    request->send(404, "text/plain", message);
}

void initLittleFS(void)
{
// Initialize LittleFS
  if (!LittleFS.begin(true))
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }

  if (!LittleFS.exists(configFile)) {
    Serial.println("Configuration file does not exist! Creating default config file...");
    File file = LittleFS.open(configFile, "w");
    if (!file) {
      Serial.println("Failed to create config file!");
    }
    file.print(defaultConfig);
    file.close();
    Serial.println("Default configuration file created.");
  }
  
  File file = LittleFS.open(configFile, "r");
  if (!file) {
    Serial.println("Cannot open config file!");
  }

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) 
  {
      Serial.println("JSON parsing error!");
  }
  ssid = doc["wifi"]["ssid"].as<String>();
  password = doc["wifi"]["password"].as<String>();
  broker.fromString(doc["mqtt"]["broker"].as<String>());
  espIp.fromString(doc["network"]["espIp"].as<String>());
  gatewayIp.fromString(doc["network"]["gatewayIp"].as<String>());

  settings_html.replace("%SSID%", ssid);
  settings_html.replace("%PASSWORD%", password);
  settings_html.replace("%BROKER%", broker.toString());
  settings_html.replace("%ESP_IP%", espIp.toString());
  settings_html.replace("%GATEWAY_IP%", gatewayIp.toString());
  settings_html.replace("%FIRMWARE_VERSION%", FIRMWARE_V);

  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);
  Serial.print("Broker: "); Serial.println(broker);
  Serial.print("ESP IP: "); Serial.println(espIp);
  Serial.print("Gateway IP: "); Serial.println(gatewayIp);
}

/*
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    //String message = (char*)data;
    //Check if the message is "getReadings"
    if (strcmp((char*)data, "getReadings") == 0) {
    //   //if it is, send current sensor readings
    //   String sensorReadings = getSensorReadings();
    //   Serial.print(sensorReadings);
    //   notifyClients(sensorReadings);
    }
  }
}
*/

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, data);
        if (!error)
        {
            const char* type = doc["type"] | "";
            if (strcmp(type, "joystick") == 0) {
                Esp2ImuFrame.moveX = doc["x"];
                Esp2ImuFrame.moveY = doc["y"];
            } else if (strcmp(type, "auger") == 0) {
                Serial.print("Auger Speed: ");
                uint16_t augerSpeed = doc["value"];
                Esp2ImuFrame.augerSpeed = augerSpeed;
                Serial.println(augerSpeed);
            } else if (strcmp(type, "route") == 0) {
                Serial.print("route: ");
                uint8_t route = doc["value"];
                Esp2ImuFrame.rootNumber = route;
                Serial.println(route);
            } else if (strcmp(type, "button") == 0) {
                uint8_t button = doc["value"];
                Esp2ImuFrame.rootAction = button;
                Serial.print("Button: ");
                Serial.println(button);
            } else if (strcmp(type, "checkbox") == 0) {
                String id = doc["id"] | "";
                bool value = doc["value"];
                Serial.printf("Checkbox %s = %s\n", id.c_str(), value ? "ON" : "OFF");
                if (id == "power") {
                  Esp2ImuFrame.power = value;
                } else if (id == "charging") {
                  Esp2ImuFrame.charging = value;
                }   
            } else {
                Serial.println("undefined WebSocket message type");
            }
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      Esp2ImuFrame.moveX = 0;
      Esp2ImuFrame.moveY = 0;
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      Esp2ImuFrame.moveX = 0;
      Esp2ImuFrame.moveY = 0;
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

//ws.textAll(sensorReadings);

void WebHandler_Init(void)
{
    initLittleFS();
    initWebSocket();
    /* register the callbacks to process client request */
    /* root request we will read the memory card to get
    the content of index.html and respond that content to client */
    server.on("/", handleRoot);
    server.on("/settings", handleSettings);
    server.on("/setMotors", handleMotors);
    server.on("/submit", handleSubmit);

    ///    server.on("/updateEsp", HTTP_POST, []()
    ///{
    /// server.send(200, "text/plain", Update.hasError() ? "Update Failed" : "Update Success"); // todo:
    ///    ESP.restart(); }, handleUpdateEsp);

    server.on("/updateEsp", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleUpdateEsp);

    // server.on("/config", HTTP_POST, []()
    //           {
    // ///server.send(200, "text/plain", "File uploaded");
    // delay(1000);
    // ESP.restart(); }, handleUpdateConfig);

    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleUpdateConfig);

    // server.on("/updateImu", HTTP_POST, []() {
    //   server.send(200, "text/plain", "File uploaded");
    // }, HandleUpdateImu);

    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}

void WebHandler_SendData(void)
{
  DynamicJsonDocument doc(512);
  doc["magnetBarStatus"] = Imu2EspFrame.magnetBarStatus;
  doc["pmbConnection"] = Imu2EspFrame.pmbConnection;
  doc["motorRightSpeed"] = Imu2EspFrame.motorRightSpeed;
  doc["motorLeftSpeed"] = Imu2EspFrame.motorLeftSpeed;
  doc["batteryVoltage"] = Imu2EspFrame.batteryVoltage;
  doc["adcCurrent"] = Imu2EspFrame.adcCurrent;
  doc["thumbleCurrent"] = Imu2EspFrame.thumbleCurrent;
  doc["crcImu2PmbErrorCount"] = Imu2EspFrame.crcImu2PmbErrorCount;
  doc["crcPmb2ImuErrorCount"] = Imu2EspFrame.crcPmb2ImuErrorCount;
  doc["crcEsp2ImuErrorCount"] = Imu2EspFrame.crcEsp2ImuErrorCount;

  String json;
  serializeJson(doc, json);
  ws.textAll(json); // Send data to all connected WebSocket clients
}

void WebHandler_CleanupClients(void)
{
    // Cleanup disconnected clients
    ws.cleanupClients();
}