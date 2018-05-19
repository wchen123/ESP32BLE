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

#define DIRECTION_MASK 0x80
#define STOP_SIGNAL_MASK 0x40

BLECharacteristic *pCharacteristic;
BLECharacteristic *rxCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

//esp lora uuid
//#define SERVICE_UUID                 "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
//#define CHARACTERISTIC_UUID_RX       "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX       "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RIGHT    "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_CONTROLS "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"



//std::string UUID_RX_LEFT =  "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
std::string UUID_RX_RIGHT = "6e400004-b5a3-f393-e0a9-e50e24dcca9e";
std::string UUID_CONTROLS = "6e400005-b5a3-f393-e0a9-e50e24dcca9e";


//0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00
//
#define SERVICE_UUID "000000FF-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_UUID_RX "0000FF01-0000-1000-8000-00805F9B34FB"

std::string UUID_RX_LEFT = "0000ff01-0000-1000-8000-00805f9b34fb";

int ledPin = 27;  
  int enA = 13;
  int in1 = 12;
  int in2 = 14;

   int enB = 17;
  int in3 = 23;
  int in4 = 2;

int left_motor_pwm = 0;
int left_motor_dir = 0;
int left_motor_stop = 0;

int right_motor_pwm = 0;
int right_motor_dir = 0;
int right_motor_stop = 0;

int flag = 0;
int rightFlag = 0;

std::string rxValue;
uint8_t pattern[10] = {0,0,0,0,0,0,0,0,0,0};
int stop_signal_counter = 0;

void LeftMotorStop(){
   digitalWrite(in1, LOW);
   digitalWrite(in2, LOW); 
   ledcWrite(1, 0);
}

void RightMotorStop(){
   digitalWrite(in3, LOW);
   digitalWrite(in4, LOW); 
   ledcWrite(2, 0);
}


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};




class MyLeftMotorCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      rxValue = pCharacteristic->getValue();
      std::string getUUID = pCharacteristic->getUUID().toString();

    if(getUUID.compare(UUID_RX_LEFT) != 0){
       return;
    }
      if (rxValue.length() > 0) {
        Serial.println("*********");    
        stop_signal_counter++;  
        left_motor_pwm = (byte)rxValue[3];
        left_motor_dir = ((byte)rxValue[2]) & DIRECTION_MASK;
        left_motor_stop = ((byte)rxValue[2]) ;//& STOP_SIGNAL_MASK;

        right_motor_pwm = (byte)rxValue[1];
        right_motor_dir = ((byte)rxValue[0]) & DIRECTION_MASK;
        right_motor_stop = ((byte)rxValue[0])& STOP_SIGNAL_MASK;
//        Serial.print("(byte)rxValue[0]: ");
//        Serial.print((byte)rxValue[0]);
//        Serial.print(" (byte)rxValue[1]: ");
//        Serial.print((byte)rxValue[1]);
//
//        Serial.print(" (byte)rxValue[2]: ");
//        Serial.print((byte)rxValue[2]);
//        Serial.print(" (byte)rxValue[3]: ");
//        Serial.print((byte)rxValue[3]);
        

        Serial.println("*********");
        
      }
    }
};


//class MyRightMotorCallbacks: public BLECharacteristicCallbacks {
//    void onWrite(BLECharacteristic *pCharacteristic) {
//      std::string rxValue = pCharacteristic->getValue();
//      std::string getUUID = pCharacteristic->getUUID().toString();
//
//      if (getUUID.compare(UUID_RX_RIGHT) != 0) {
//        return;
//      }
//      if (rxValue.length() > 0) {
//        Serial.println("*********");
//        Serial.print("Received length: ");
//        Serial.println(sizeof(rxValue));
//        Serial.print("Received Value: ");
//      
//        for (int i = 0; i < rxValue.length(); i++){
//           // check which characteristic it is talking to
//            if (getUUID.compare(UUID_RX_RIGHT) == 0) {
//              right_motor_dir = (byte)rxValue[0];
//              right_motor_pwm = (byte)rxValue[i];
//              if(flag == 0) {
//                  Serial.print("right motor pwm reached!");
//       
//                  flag = 1;
//              }
//              Serial.print(" dir: ");
//              Serial.print(right_motor_dir);
//              Serial.print(" pwm: ");
//              Serial.print((byte)right_motor_pwm);
//           }
//         
//          
//        }
//        flag = 0;
//
//        Serial.println("*********");
//        
//      }
//    }
//};
//
//class MyLeftMotorStopCallbacks: public BLECharacteristicCallbacks {
//    void onWrite(BLECharacteristic *pCharacteristic) {
//      std::string rxValue = pCharacteristic->getValue();
//      std::string getUUID = pCharacteristic->getUUID().toString();
//
//  if (getUUID.compare(UUID_CONTROLS) != 0) {
//     
//      return;
//  }
//      if (rxValue.length() > 0) {
//     
//        
//           // check which characteristic it is talking to
//        
//           if (rxValue[0] == 0xF0) {
//            
//                left_motor_pwm = 0;
//                
//           }else if(rxValue[0] == 0x0F) {
//              right_motor_pwm =0 ;
//           }
//           Serial.println("+++++++++++++++reach MyLeftMotorStopCallbacks+++++++++++\n");
//           Serial.print((byte)rxValue[0]);
//           Serial.print("\n");
//          
//        
//     
//
//        Serial.println("*********");
//        
//      }
//    }
//};


  
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

  rxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                      );
  //added an extra characteristic for the right side of the joystick
//  BLECharacteristic *rJoystickCharacteristic = pService->createCharacteristic(
//                                         CHARACTERISTIC_UUID_RIGHT,
//                                         BLECharacteristic::PROPERTY_WRITE
//                                       );                                   
//
//  BLECharacteristic *controlCharacteristic = pService->createCharacteristic(
//                                         CHARACTERISTIC_UUID_CONTROLS,
//                                         BLECharacteristic::PROPERTY_WRITE
//                                       );    

  //call the callback function for the left, right side of the mtor characteristic and stop signal
  rxCharacteristic->setCallbacks(new MyLeftMotorCallbacks());
//  rJoystickCharacteristic->setCallbacks(new MyRightMotorCallbacks());
//  controlCharacteristic->setCallbacks(new MyLeftMotorStopCallbacks());

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

   ledcAttachPin(enB, 2);
   ledcSetup(2, 12000, 8); 
  
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
}

void LeftMotorForwards(int x){
   //left 
   digitalWrite(in1, LOW);
   digitalWrite(in2, HIGH); 

//   //right
//   digitalWrite(in3, LOW);
//   digitalWrite(in4, HIGH);
   
   ledcWrite(1, x);
  // ledcWrite(2, x);
}
void LeftMotorBackwards(int x){
   //left 
   digitalWrite(in1, HIGH);
   digitalWrite(in2, LOW); 

//   //right
//   digitalWrite(in3, HIGH);
//   digitalWrite(in4, LOW);
   
   ledcWrite(1, x);
  // ledcWrite(2, x);
}


void RightMotorForwards(int x){ 

//   //right
   digitalWrite(in3, LOW);
   digitalWrite(in4, HIGH);
   
  
   ledcWrite(2, x);
}
void RightMotorBackwards(int x){

//   //right
   digitalWrite(in3, HIGH);
   digitalWrite(in4, LOW);
   
   ledcWrite(2, x);
}


int i = 0;
void loop() {
//digitalWrite(ledPin, HIGH);
  if (deviceConnected) {
      //Serial.printf("*** Sent Value: %d ***\n", txValue);
      //pCharacteristic->setValue(&txValue, 1);
      //pCharacteristic->notify();
      //txValue++;

//      if((rxCharacteristic->getValue()).empty()) {
//         M
//      }else {
//        std::string charValue = pCharacteristic->getValue();
//        Serial.print("*** something is in pCharacteristics ***\n");
//        Serial.print((byte)charValue[3]);
//        Serial.print("\n");
//      }
      
      pattern[i] = stop_signal_counter;
      Serial.printf("*** %d %d %d %d %d %d %d %d %d %d***\n", pattern[0], pattern[1],  pattern[2],  pattern[3],  pattern[4], 
                          pattern[5], pattern[6],  pattern[7],  pattern[8],  pattern[9]);
      if ((pattern[0] == pattern[1]) && (pattern[1] == pattern[2]) && (pattern[2] == pattern[3]) && (pattern[3] == pattern[4])  
              && (pattern[4] == pattern[5]) && (pattern[5] == pattern[6]) 
              && (pattern[6] == pattern[7]) && (pattern[7] == pattern[8]) && (pattern[8] == pattern[9])) {
                        //LeftMotorStop();
                        //RightMotorStop(); 
                        //left_motor_pwm = 0;
                        //right_motor_pwm = 0;
                        Serial.print("*** all pattern are the same )))))-------------> ***\n");
                        stop_signal_counter = 0;
      }
      
      

      i++;
      if(i >= 10) {
        i = 0;
      }

      if(left_motor_dir == DIRECTION_MASK) {
          LeftMotorForwards(left_motor_pwm);
          
      }else {
          LeftMotorBackwards(left_motor_pwm);
      }


      if(right_motor_dir == DIRECTION_MASK) {
        RightMotorForwards(right_motor_pwm);
      }else{
         RightMotorBackwards(right_motor_pwm);
      }
        Serial.println("-------------------");
        Serial.print("left pwm: ");
        Serial.print(left_motor_pwm);
        Serial.print(" ");
        Serial.print("stop signal: ");
        Serial.print(left_motor_stop);
        Serial.print(" ");
        Serial.print("right pwm: ");
        Serial.print(right_motor_pwm);
        Serial.print(" ");
        Serial.print("stop signal: ");
        Serial.print(right_motor_stop);
        Serial.print("\n");
        Serial.println("-------------------");

  

  }
   
  
 // delay(1000);
  //digitalWrite(ledPin, LOW);
 // 
 // delay(1000);
}
