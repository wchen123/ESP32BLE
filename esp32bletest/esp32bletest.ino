/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdio.h>     
#include <stdlib.h>
#include <sstream>
#include <string>
#include "Arduino.h"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID             "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX   "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX   "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RIGHT "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"

std::string UUID_RX_LEFT = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
std::string UUID_RX_RIGHT = "6e400004-b5a3-f393-e0a9-e50e24dcca9e";
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
int left_motor_pwm = 0;
int right_motor_pwm = 0;
int flag = 0;
int rightFlag = 0;
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      std::string getUUID = pCharacteristic->getUUID().toString();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received length: ");
        Serial.println(sizeof(rxValue));
        Serial.print("Received Value: ");
      
        for (int i = 0; i < rxValue.length(); i++){
           // check which characteristic it is talking to
           //left joystick characteristic 
           if(getUUID.compare(UUID_RX_LEFT) == 0){
              left_motor_pwm = (byte)rxValue[i];
              if (flag == 0) {
                  Serial.print("left motor pwm reached: ");
                  flag = 1;
              }
              Serial.print((byte)left_motor_pwm);
           
              //right joystick characteristic
           }else if (getUUID.compare(UUID_RX_RIGHT) == 0) {
              right_motor_pwm = (byte)rxValue[i];
              if(flag == 0) {
                  Serial.print("right motor pwm reached: ");
                  flag = 1;
              }
              Serial.print((byte)right_motor_pwm);
           }
          
        }
        flag = 0;
        Serial.println();
        Serial.println("*********");
      }
    }
};

int ledPin = 27;  
  int enA = 13;
  int in1 = 12;
  int in2 = 14;
  
void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                      );
  //added an extra characteristic for the right side of the joystick
  BLECharacteristic *rJoystickCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RIGHT,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );                                   



  pCharacteristic->setCallbacks(new MyCallbacks());
  //call the callback function for the right side of the characteristic
  rJoystickCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  // PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[ledPin], PIN_FUNC_GPIO);
  

   pinMode(ledPin, OUTPUT);
   // set all the motor control pins to outputs
     ledcAttachPin(enA, 1);
   ledcSetup(1, 12000, 8); 
  //pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
//  pinMode(in3, OUTPUT);
 // pinMode(in4, OUTPUT);
}

void motorForwards(int x){
   digitalWrite(in1, LOW);
   digitalWrite(in2, HIGH); 
   ledcWrite(1, x);
}
void loop() {
//digitalWrite(ledPin, HIGH);
  if (deviceConnected) {
    Serial.printf("*** Sent Value: %d ***\n", txValue);
    pCharacteristic->setValue(&txValue, 1);
    pCharacteristic->notify();
    txValue++;
    motorForwards(left_motor_pwm);
  }
  
  delay(1000);
  //digitalWrite(ledPin, LOW);
 // 
 // delay(1000);
}
