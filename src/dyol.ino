#include "ntp-time.h"
#include <blynk.h>
#include <MCP9808.h>
#include <ITEADLIB_Nextion.h>
#include <google-maps-device-locator.h>
#include <Adafruit_DHT_Particle.h>
//#include <HttpClient.h>
#include <unistd.h>
#include <cstdio>
#include <JsonParserGeneratorRK.h>
#include "Particle.h"
#include "application.h"

SYSTEM_THREAD(ENABLED);

int i = 0;
NtpTime* ntpTime;

String hhmmss(unsigned long int now)  //format value as "hh:mm:ss"
{
    int time = 0;
    if (Time.hour(now) -5 < 0 )
    {
        time = Time.hour(now) + 7;
    }
    String hour = String (time);
   //String hour = String(Time.hour(now));
   String minute = String::format("%02i",Time.minute(now));
   String second = String::format("%02i",Time.second(now));
   return hour + ":" + minute + ":" + second;
};

MCP9808 mcp = MCP9808();
int lastResult = 0;

//void nfcCallback(nfc_event_type_t type, nfc_event_t* event, void* context);

JsonParserStatic<512, 50> jsonParser;
GoogleMapsDeviceLocator locator;
unsigned int nextTime = 30;    // Next time to contact the server
//HttpClient http;

#define internalTempValue A2
#define rawEncoderValue A4
//#define UART_TX_BUF_SIZE        20

//DHT dhtTemp = DHT(internalTempValue, DHT11);
int button_touched = 0;

//uint8_t txBuf[UART_TX_BUF_SIZE];
size_t txLen = 0;
String tempRecieved; 

Thread internalTempThread;
Thread externalTempThread;
Thread displayManagementThread; //handles UI and encoder interpret
Thread blynkDataPushThread;
Thread nfcPairingThread;

int counter = 0;
volatile bool updateCounter = true;

//NexPage page0 = NexPage(0, 0, "page0");
//NexPage page1 = NexPage(1, 0, "page1");
//NexPage page2 = NexPage(2, 0, "page2");
//NexPage page3 = NexPage(3, 0, "page3");

//NexText t0 = NexText(2,12,"t0");

/*NexTouch *nex_listen_list[] = 
{
    &page0,
    &page1,
    &page2,
    &page3,
    NULL
};
*/

float global_lat = 0;
float global_lon = 0;

void internalTempFunction() {
  while(1){
    //display internal temp
    //dhtTemp = DHT(internalTempValue, DHT11);
    //print it
    float celcius = mcp.getTemperature();
    Serial.println(celcius); Serial.println(" (C)");

    float fahreinheit = (celcius * (1.8)) + 32;
    Serial.println(fahreinheit); Serial.println(" (F)");
    delay(500);
  }
}

void externalTempFunction() {
  while(1){
    //display external temp     SPRINTF
    //double temp = https://api.weatherbit.io/v2.0/current?city=Lombard,NC&key=c175924274b5402e8413adbee368dd35;
    char data[64];
    sprintf(data, "%f,%f", 41.8781, -87.6298);
    Particle.publish("tempRead", data, PRIVATE);
    delay(30000);
  }
}

/*void googleLocationFunction() {
  while(1){
    //display external temp     SPRINTF
    //double temp = https://api.weatherbit.io/v2.0/current?city=Lombard,NC&key=c175924274b5402e8413adbee368dd35;
    char data[64];
    sprintf(data, "%f,%f", 41.8781, -87.6298);
    Particle.publish("tempRead", data, PRIVATE);
    delay(30000);
  }
}*/

void displayManagementFunction() {
  while(1){
    //display managment (page select)
      //interpret encoder
    //touch == "enter"
    /*page0.show();
    delay(500);
    page1.show();
    delay(500);
    page2.show();
    delay(500);
    page3.show();
    delay(500);*/
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

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
/*     LOG(TRACE, "Received data from: %02X:%02X:%02X:%02X:%02X:%02X:",
            peer.address()[0], peer.address()[1], peer.address()[2], peer.address()[3], peer.address()[4], peer.address()[5]);

    BleCharacteristic* characteristic = static_cast<BleCharacteristic*>(context);
    BleUuid uuid = characteristic->UUID();
    Serial1.print("Characteristic UUID: ");
    for (int i = 0; i < 16; i++) {
        Serial1.printf("0x%02X,", uuid.full()[i]);
    }
    Serial1.println("");*/

    for (uint8_t i = 0; i < len; i++) {
        Serial.write(data[i]);
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
  jsonParser.addString(data);
  if (jsonParser.parse()) {
    jsonParser.getOuterValueByKey("tempRecieved", tempRecieved);
    
    // Put code to do something with tempMin and tempMax here
    Serial.println(tempRecieved);

  }
}

void setup() {
  nfcPairingThread = Thread("nfcPair", nfcPairingFunction);
  blynkDataPushThread = Thread("blynkPush", blynkDataPushFunction);
  displayManagementThread = Thread("displayManagement", displayManagementFunction);
  externalTempThread = Thread("externalTemp", externalTempFunction);
  internalTempThread = Thread("internalTemp", internalTempFunction);
  pinMode(internalTempValue, INPUT);
  Serial.begin(9600);
  //locator.withSubscribe(locationCallback).withLocatePeriodic(4800);
  Particle.subscribe("hook-response/tempRead", tempHandler, MY_DEVICES);
  pinMode(D7, OUTPUT);
  digitalWrite(D7, 0);
  LOG(TRACE, "Application started.");
  Serial1.begin(115200);
  //BLE.addCharacteristic(txCharacteristic);
  //BLE.addCharacteristic(rxCharacteristic);
  BleAdvertisingData data;
  //data.appendServiceUUID(serviceUuid);
  data.appendLocalName("Todd");
  BLE.advertise(&data);
  while(! mcp.begin()){
      Serial.println("MCP9808 not found");
      delay(500);
    }

  Serial.println("MCP9808 OK");

}



void loop() {
  if(button_touched == 1){
    Serial.println("button_on");
  }
  Serial1.printf("t0.txt=\"Test!\"\xFF\xFF\xFF");
  delay(500);
  Serial1.printf("t0.txt=\"Test!\"\xFF\xFF\xFF");

  /*if (updateCounter) {
        updateCounter = false;

        char buf[64];
        snprintf(buf, sizeof(buf), "testing counter=%d", ++counter);

        NFC.setText(buf, "en");
        NFC.on(nfcCallback);

        Log.info("next read should show: %s", buf);
  }*/
//}
    static unsigned long waitMillis;
    struct epochMillis now;  //holds the unix epoch time to millisecond resolution
    if(millis() > waitMillis) {
        ntpTime->nowMillis(&now);  //get the current NTP time
        Particle.publish("NTP clock is: ", hhmmss(now.seconds) + "." + String::format("%03i",now.millis));
        Particle.publish("System clock is: ", hhmmss(Time.now()));
        Serial.println(hhmmss(now.seconds) + "." + String::format("%03i",now.millis));
        Serial.println("System: " + hhmmss(Time.now()));
        waitMillis = millis() + (15*1000);  // wait 15 seconds
    }
 /* if (BLE.connected()) {
    while (Serial.available() && txLen < UART_TX_BUF_SIZE) {
      txBuf[txLen++] = Serial.read();
      Serial.write(txBuf[txLen - 1]);
    }

    if (txLen > 0) {
      //txCharacteristic.setValue(txBuf, txLen);
      //txLen = 0;
    }
  }*/
  //t0.setText("79.8");
/*
static void nfcCallback(nfc_event_type_t type, nfc_event_t* event, void* context) {
    switch (type) {
    case NFC_EVENT_FIELD_ON: {
        digitalWrite(D7, 1);
        break;
    }
    case NFC_EVENT_FIELD_OFF: {
        digitalWrite(D7, 0);
        break;
    }
    case NFC_EVENT_READ: {
        updateCounter = true;
        break;
    }
    default:
        break;
    }
}*/

}