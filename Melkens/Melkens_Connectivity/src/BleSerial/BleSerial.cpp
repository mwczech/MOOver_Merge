
#include "BleSerial.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      /// Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }

      Serial.println();
      /// Serial.println("*********");
    }
  }
};

void BleSerial_Init()
{
  /// Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Melkens_Serial");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void BleSerial_Perform()
{

  if (deviceConnected)
  {
    pTxCharacteristic->setValue(&txValue, 1);
    pTxCharacteristic->notify();
    txValue++;
    delay(10); // bluetooth stack will go into congestion, if too many packets are sent
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void BleSerial_SendData(const char *data, size_t length)
{
  if (deviceConnected)
  {
    pTxCharacteristic->setValue((uint8_t *)data, length);
    pTxCharacteristic->notify();
    //   Serial.print("Sent Value: ");
    //   for (size_t i = 0; i < length; i++) {
    //     Serial.print(data[i]);
    //   }
    //   Serial.println();
    // } else {
    //   Serial.println("No device connected to send data.");
    // }
  }
}

void BleSerial::begin()
{
  BleSerial_Init();
}

void BleSerial::perform()
{
  BleSerial_Perform();
}
void BleSerial::sendData(const char *data, size_t length)
{
  BleSerial_SendData(data, length);
}

void BleSerial::print(const char *msg)
{
  BleSerial_SendData(msg, strlen(msg));
}

void BleSerial::println(const char *msg)
{
  BleSerial_SendData(msg, strlen(msg));
  const char newline = '\n';
  BleSerial_SendData(&newline, 1);
}
