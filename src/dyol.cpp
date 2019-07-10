/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "c:/Users/Nerdom/Desktop/dyol/src/dyol.ino"
void internalTempFunction();
void externalTempFunction();
void displayManagementFunction();
void blynkDataPushFunction();
void nfcPairingFunction();
void setup();
void loop();
#line 1 "c:/Users/Nerdom/Desktop/dyol/src/dyol.ino"
SYSTEM_THREAD(ENABLED);

#include <blynk.h>
#include <ITEADLIB_Nextion.h>
#include <Adafruit_DHT_Particle.h>
#include <HttpClient.h>
#include <Nextion.h>

#define internalTempValue A2
#define rawEncoderValue A4

DHT dhtTemp = DHT(internalTempValue, DHT22);
int button_touched = 0;

Thread internalTempThread;
Thread externalTempThread;
Thread displayManagementThread; //handles UI and encoder interpret
Thread blynkDataPushThread;
Thread nfcPairingThread;

void internalTempFunction() {
  while(1){
    //display internal temp
    dhtTemp = DHT(internalTempValue, DHT22);
    //print it
  }
}

void externalTempFunction() {
  while(1){
    //display external temp
    //double temp = https://api.weatherbit.io/v2.0/current?city=Lombard,NC&key=c175924274b5402e8413adbee368dd35;

  }
}

void displayManagementFunction() {
  while(1){
    //display managment (page select)
      //interpret encoder
    //touch == "enter"
  }
}

void blynkDataPushFunction() {
  while(1){
    //pushes info to blynk
  }
}

void nfcPairingFunction() {
  while(1){
    //advertises on NFC

    //requests wifi creds

    //sets wifi creds
  }
}

void setup() {
  nfcPairingThread = Thread("nfcPair", nfcPairingFunction);
  blynkDataPushThread = Thread("blynkPush", blynkDataPushFunction);
  displayManagementThread = Thread("displayManagement", displayManagementFunction);
  externalTempThread = Thread("externalTemp", externalTempFunction);
  internalTempThread = Thread("internalTemp", internalTempFunction);
  pinMode(internalTempValue, INPUT);
  Serial.begin();
}



void loop() {
  if(button_touched == 1){
    Serial.println("button_on");
  }
  
}