SYSTEM_THREAD(ENABLED);

#include <blynk.h>
#include <ITEADLIB_Nextion.h>
#include <google-maps-device-locator.h>
#include <Adafruit_DHT_Particle.h>
//#include <HttpClient.h>
#include <Nextion.h>
#include <unistd.h>
#include <cstdio>
#include <JsonParserGeneratorRK.h>

SerialLogHandler logHandler;
GoogleMapsDeviceLocator locator;
USARTSerial& nexSerial = Serial1;
unsigned int nextTime = 30;    // Next time to contact the server
//HttpClient http;

#define internalTempValue A2
#define rawEncoderValue A4

DHT dhtTemp = DHT(internalTempValue, DHT22);
int button_touched = 0;

Thread internalTempThread;
Thread externalTempThread;
Thread displayManagementThread; //handles UI and encoder interpret
Thread blynkDataPushThread;
Thread nfcPairingThread;

NexPage page0 = NexPage(0, 0, "page0");
NexPage page1 = NexPage(1, 0, "page1");
NexPage page2 = NexPage(2, 0, "page2");
NexPage page3 = NexPage(3, 0, "page3");

float global_lat = 0;
float global_lon = 0;

void internalTempFunction() {
  while(1){
    //display internal temp
    dhtTemp = DHT(internalTempValue, DHT22);
    //print it
  }
}

void externalTempFunction() {
  while(1){
    //display external temp     SPRINTF
    //double temp = https://api.weatherbit.io/v2.0/current?city=Lombard,NC&key=c175924274b5402e8413adbee368dd35;
    char data[64];
    sprintf(data, "%f,%f", global_lat, global_lon);
    Particle.publish("tempRead", data, PRIVATE);
    delay(10000);
  }
}

void displayManagementFunction() {
  while(1){
    //display managment (page select)
      //interpret encoder
    //touch == "enter"
    page0.show();
    delay(500);
    page1.show();
    delay(500);
    page2.show();
    delay(500);
    page3.show();
    delay(500);
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

void locationCallback(float lat, float lon, float accuracy) {
  // Handle the returned location data for the device. This method is passed three arguments:
  // - Latitude
  // - Longitude
  // - Accuracy of estimated location (in meters)
  global_lat = lat;
  global_lon = lon;
}

void tempHandler(const char *event, const char *data) {
  // Handle the integration response
  Serial.println("Recieved.");
}

void setup() {
  nfcPairingThread = Thread("nfcPair", nfcPairingFunction);
  blynkDataPushThread = Thread("blynkPush", blynkDataPushFunction);
  displayManagementThread = Thread("displayManagement", displayManagementFunction);
  externalTempThread = Thread("externalTemp", externalTempFunction);
  internalTempThread = Thread("internalTemp", internalTempFunction);
  pinMode(internalTempValue, INPUT);
  Serial.begin();
  locator.withSubscribe(locationCallback).withLocatePeriodic(4800);
  Particle.subscribe("hook-response/tempRead", tempHandler, MY_DEVICES);
}



void loop() {
  if(button_touched == 1){
    Serial.println("button_on");
  }
  locator.loop();
}