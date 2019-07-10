SYSTEM_THREAD(ENABLED);

#include <blynk.h>
#include <ITEADLIB_Nextion.h>
#include <Adafruit_DHT_Particle.h>



#define internalTempValue A2
#define rawEncoderValue A4

Thread internalTempThread;
Thread externalTempThread;
Thread displayManagementThread; //handles UI and encoder interpret
Thread blynkDataPushThread;
Thread nfcPairingThread;

void internalTempFunction() {
  while(1){
    //display internal temp
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
}



void loop() {

}