/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "c:/Users/Nerdom/Desktop/capstone/src/capstone.ino"
#include <google-maps-device-locator.h>
#include "Particle.h"
#include "application.h"
#include <cstdio>
#include <JsonParserGeneratorRK.h>
#include <ntp-time.h>
#include <cstring>
#include "MCP9808.h"

String hhmmss(unsigned long int now);
void timeFunction();
void bluetoothReceive();
void encoderDial();
void encoderDecision();
void dialCounter();
void externalTempFunction();
void setup();
void locationCallback(float lat, float lon, float accuracy);
void internalTemp();
void stopwatch();
void tempHandler(const char *event, const char *data);
void geoIpHandler(const char *event, const char *data);
void ipHandler(const char *event, const char *data);
void screenWrite();
void cityWrite();
void loop();
#line 10 "c:/Users/Nerdom/Desktop/capstone/src/capstone.ino"
#define UART_TX_BUF_SIZE        20

SYSTEM_THREAD(ENABLED);

Thread internalTempThread;
Thread externalTempThread;
Thread timeThread;
Thread screenWriteThread;
Thread stopWatchThread;
Thread cityWriteThread;
Thread encoderDialThread;
Thread dialCounterThread;

bool previousA3=HIGH; // Previous value (High or Low) of A3
bool previousA4=HIGH; // Previous value (High or Low) of A4
int counter = 0; // Value of encoder

int currentPage = 1;
int page=0;
MCP9808 mcp = MCP9808();
int lastResult = 0;
unsigned int nextTime = 30;    // Next time to contact the server
String global_lat = ""; //variables for location data
String global_lon = "";
String global_city = "";
String global_regionCode = "";
String global_ip = ""; //through here
int i = 0; //yeah this is here
float convertedTemp = 0;
String tempReceived;
JsonParserStatic<512, 50> jsonParser;
NtpTime* ntpTime;
float internalTempValue = 0;
bool wifiKeyState = false;
char ssid[64]; //ssid buffer
char passkey[64]; //passkey buffer


//bluetooth inits
void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);//ble inits for setting handlers

const char* serviceUuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";//ble inits setting uuids
const char* rxUuid = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";//ble inits setting uuids
const char* txUuid = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";//ble inits setting uuids

BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);//ble inits
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, &rxCharacteristic);//ble inits

uint8_t txBuf[UART_TX_BUF_SIZE]; //buffer for BLE
size_t txLen = 0;

String hhmmss(unsigned long int now)  //format value as "hh:mm:ss"
{
  int time = 0;
  if (Time.hour(now) - 5 < 0 )
  {
    time = Time.hour(now) + 7; // not sure what this does but it seems to be required
  }
  String hour = String (0);
  if (Time.hour(now) > 12) {
    hour = String (Time.hour(now) - 17); //hour adjustment for 12hours
  }
  if (Time.hour(now) < 1) { //for AM times
    hour = String(Time.hour(now) + 12);
  }
  else {
    hour = String (Time.hour(now) - 5); //for time zone
  }
  //String hour = String(Time.hour(now));
  String minute = String::format("%02i",Time.minute(now));//formatting
  String second = String::format("%02i",Time.second(now));
  return hour + ":" + minute;
};

void timeFunction(){//thread for time calculation and conversion
  while(1){
    static unsigned long waitMillis;
    struct epochMillis now;  //holds the unix epoch time to millisecond resolution
    char dataClock [64];
    if(millis() > waitMillis) {
        ntpTime->nowMillis(&now);  //get the current NTP time
        Particle.publish("NTP clock is: ", hhmmss(now.seconds) + "." + String::format("%03i",now.millis)); //asks for time
        Particle.publish("System clock is: ", hhmmss(Time.now()));
        Serial.println(hhmmss(now.seconds) + "." + String::format("%03i",now.millis));//prints time
        Serial.println("System: " + hhmmss(Time.now()));
        sprintf(dataClock, "t3.txt=\"%s\"\xFF\xFF\xFF", hhmmss(Time.now()).c_str());
        Serial1.println(dataClock);//prints time to screen
        Serial.println(dataClock);
        waitMillis = millis() + (3000);  // wait 3 seconds
    }
  }
}

void bluetoothReceive() { //handler for when BLE commands are received 
  if (BLE.connected()) {
        while (Serial.available() && txLen < UART_TX_BUF_SIZE) {
            txBuf[txLen++] = Serial.read();
            Serial.write(txBuf[txLen - 1]);
        }

        if (txLen > 0) {
            txCharacteristic.setValue(txBuf, txLen);
            txLen = 0;
        }
    }
}

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
    WiFi.on(); //turns wifi on again
    WiFi.clearCredentials(); //clears credentials to prevent having 2 sets of credentials.
    char buffer[10]; //databuffer for stuff im not sure
    if(wifiKeyState == false){
        for (uint8_t i = 0; i < len; i++) {
            ssid[i] = (char) data[i]; //loops through incoming data to make ssid array
        }
        ssid[len] = 0x00; //terminates SSID with a NULL
        wifiKeyState = true; //sets next BLE UART string to be sent to passkey
        Serial.printf("Wifi SSID set to: %s\n", ssid); //prints for debugging
    }
    else if(wifiKeyState == true){
        for (uint8_t i = 0; i < len; i++) {
            passkey[i] = (char) data[i]; //loops through passkey array to set to char*
        }
        passkey[len] = 0x00; //passkey is terminated with NULL
        wifiKeyState = false;//sets the next BLE UART string sent to be the SSID not the passkey
        Serial.printf("Wifi PSK set to: %s\n", passkey); //prints for debugging
        WiFi.disconnect();//disconnects from wifi to prevent possible interferance that i dont know actually exists
        WiFi.setCredentials(ssid, passkey, WPA2); //sets creds
        WiFiAccessPoint ap[5]; //number = how many wifi credentials that are saved to print (from most recent, should be only one)
        int found = WiFi.getCredentials(ap, 5);
        for (int i = 0; i < found; i++) { //prints out current wifi data
          Serial.print("ssid: ");
          Serial.println(ap[i].ssid);
          // security is one of WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2, WLAN_SEC_WPA_ENTERPRISE, WLAN_SEC_WPA2_ENTERPRISE
          Serial.print("security: ");
          Serial.println(ap[i].security);
          // cipher is one of WLAN_CIPHER_AES, WLAN_CIPHER_TKIP or WLAN_CIPHER_AES_TKIP
          Serial.print("cipher: ");
          Serial.println(ap[i].cipher);
        }
        Serial.println("WiFi Set");
        WiFi.connect(); //*should* connect to wifi based on current credentials
    }
}

char data5 [64];
void encoderDial() {//thread for encoder interpretations
  while(1){  
    if (digitalRead(A3) == LOW && previousA3 == HIGH){
      counter++; //increase counter for turn right
      if(digitalRead(A4) == LOW && digitalRead(A3) == LOW){
        counter--;//decrease counter for turn left
        counter--; // Cancels out the counter++
      }
      Serial.println(counter); //prints out counter to serial for debugging
    }
    previousA3 = digitalRead(A3); //Sets new "previous" variables
    previousA4 = digitalRead(A4); 
    //Serial1.print(data5);
    //Serial.print(data5);
  }
}

void encoderDecision() {
  Serial1.print("page 2\xFF\xFF\xFF");
  while(1){
    Serial.println(currentPage);
    Serial.println(counter);
    if(currentPage != counter){
      sprintf(data5, "page %i\xFF\xFF\xFF", (int) counter); //constructs command for nextion. format: "page x" where x=page number
      Serial1.print(data5);//sends command to screen
      Serial.println(data5);//debugging serial send
      currentPage = counter;
      delay(500);
    }
    delay(500);
  }
}

// page is global

void dialCounter(){
  /*while(1) {
    if(counter>-20){
      counter=0;
      page++;
    }
      if(counter<-20){
      counter=0;
      page--;
    }
    if(page>5){
      page=0;
    }
    if(page<0){
      page=5;
    }
  }*/
}

void externalTempFunction() {//GETs webhook data and parses it and sends it to screenWrite
    while(1) {
      // Step 1: get IP
      Serial.println("externalTempFunction: calling IP");
      Particle.publish("particle/device/ip"); //requests public ip from particle
      delay(4000);

      // Step 2: get geo location
      Serial.println("externalTempFunction: calling geoip");
      Particle.publish("geoip", global_ip.c_str(), PRIVATE); //requests data from particle and translates it into a char*
      delay(4000);

      // Step 3: get temp
      Serial.printlnf("externalTempFunction:currentTemp is %s", tempReceived.c_str());
      //double temp = https://api.weatherbit.io/v2.0/current?city=Lombard,NC&key=c175924274b5402e8413adbee368dd35;
      char data[64]; //data buffer for integrating lat and lon for webhook
      sprintf(data, "%s,%s", global_lat.c_str(), global_lon.c_str());//move to geoip handler so is step by step
      Particle.publish("tempRead", data, PRIVATE);//                    that way it is event driven
      Serial.println("externalTempFunction:published"); //debugging
      delay(10000);
    }
}

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200);
    Particle.subscribe("hook-response/tempRead", tempHandler, MY_DEVICES);//subscribes to external temp webhook
    Particle.subscribe("hook-response/geoip", geoIpHandler, MY_DEVICES);//subscribes to geoip data webhook
    externalTempThread = Thread("externalTemp", externalTempFunction); //GETs webhook data and parses it and sends it to screenWrite
    stopWatchThread = Thread("stopWatchTask", stopwatch); //thread for stopwatch code that really isn't being used currently
    internalTempThread = Thread("sensorTemp", internalTemp);//thread for internal temp read, conversion, and writing to display
    cityWriteThread = Thread("cityThread", cityWrite);//thread for the geoip interpretation and parsing
    screenWriteThread = Thread("screenThread", screenWrite);//thread for writing to the screen
    encoderDialThread = Thread("encoderThread", encoderDial);//thread for encoder interpretations
    timeThread = Thread("timeTask", timeFunction);//thread for time calculation and conversion
    dialCounterThread = Thread("dial", dialCounter);
    tempReceived = "No IP";//sets default value for nextion external temp readout
    Particle.subscribe("particle/device/ip", ipHandler);//subscribes to global IP get
    WiFi.on();//turns wifi on in the case device is in manual mode
    pinMode(A2, INPUT);//for temp sensor
    /*while(! mcp.begin()) {
      Serial.println("MCP9808 not found");
      delay(500);
    }
    Serial.println("MCP9808 OK");*/
    pinMode (A3, INPUT_PULLUP);//for encoder
    pinMode (A4, INPUT_PULLUP);//for encoder
    BLE.addCharacteristic(txCharacteristic);//BLE init stuff
    BLE.addCharacteristic(rxCharacteristic);//BLE init stuff
    BleAdvertisingData data;//BLE init stuff
    data.appendServiceUUID(serviceUuid);//BLE init stuff
    data.appendLocalName("DYOL");//BLE init stuff, sets device broadcast name
    BLE.advertise(&data);//BLE init stuff
    WiFi.setCredentials("WCL2","atmega328", WPA2); //sets creds
    WiFi.connect();//connects to wifi 
    encoderDecision(); //starts loop to send page switching commands based on encoder value
    Particle.connect();
}

void locationCallback(float lat, float lon, float accuracy) {
}

void internalTemp(){//thread for internal temp read, conversion, and writing to display
  Serial.println("internalTemp:called");
  while(1){
    Serial.println("internalTemp:while-entered");
    char dataInternal [64]; //data buffer for internal temp
    convertedTemp = (analogRead(A2) * (3300.0 / 4095)); //converts analog input
    convertedTemp = ((convertedTemp - 50)/10);//further converts analog input
    Serial.printlnf("internalTemp:%f",convertedTemp);
    sprintf(dataInternal, "t1.txt=\"%i\"\xFF\xFF\xFF", (int) convertedTemp); //does some stuff to convert into command
    Serial1.println(dataInternal); //sends to nextion
    delay(10000);
  }
}

void stopwatch() {//thread for stopwatch code that really isn't being used currently
  Serial.println("stopwatch:called");
  int timeElapsed = 0;
  bool startedButton = digitalRead(D7);
  bool started = false;
  char data [64];
  while(1) {
    if(startedButton == true){
      started = true;
      Serial.println("stopwatch:started");
    }
    while(started == true){
      timeElapsed=timeElapsed+1;
      Serial.print(timeElapsed);
      Serial1.printf("t7.txt=\"%i\"\xFF\xFF\xFF", timeElapsed);
      Serial.printlnf("t7.txt=\"%i\"\xFF\xFF\xFF", timeElapsed);
      if(startedButton == true){
        started = false;
        Serial.println("stopwatch:stopped");
      }
      delay(100);
    }
  }
}

void tempHandler(const char *event, const char *data) { //callback for temp webhook
  // Handle the integration response
  Serial.printlnf("tempHandler:dataReceived=%s", data);
  jsonParser.addString(data); //adds json to buffer
  if (jsonParser.parse()) {
    jsonParser.getOuterValueByKey("tempReceived", tempReceived); //parses
    
    // Put code to do something with tempMin and tempMax here
    Serial.printlnf("tempReceived=%s", tempReceived.c_str()); //translates String to char* 
  }
  jsonParser.clear();//clears buffer so that geographical data can be parsed correctly
}

void geoIpHandler(const char *event, const char *data) { //callback for geoip webhook
  Serial.println("geoIpHandler:called");
  Serial.println(data);

  jsonParser.addString(data); //this input uses a response template on the particle console to simplify the response
  if (jsonParser.parse()) {
    jsonParser.getOuterValueByKey("latitude", global_lat);
    jsonParser.getOuterValueByKey("longitude", global_lon);
    jsonParser.getOuterValueByKey("city", global_city);
    jsonParser.getOuterValueByKey("region_code", global_regionCode);

    //serial debugging
    Serial.printlnf("lat=%s,lon=%s,city=%s,region_code=%s", global_lat.c_str(), global_lon.c_str(), global_city.c_str(), global_regionCode.c_str());
  }
  jsonParser.clear(); //clears the parser buffer so the temp can be parsed correctly
}

void ipHandler(const char *event, const char *data) {
  Serial.println("ipHandler: called");
  Serial.println(data);
  global_ip = data; //sets ip
  Serial.printlnf("global_ip=%s", global_ip.c_str()); //prints ip
}

void screenWrite() {//thread for writing to the screen
  Serial.println("screenWrite:entered");
  while(1) {
    Serial.println("screenWrite:while-loop-entered");
    char data [64];
    sprintf(data, "t0.txt=\"%s\"\xFF\xFF\xFF", tempReceived.c_str());//translates tempReceived into a char* and writes to buffer
    Serial.println(data); //writes buffer to usb serial
    Serial1.printf(data);//writes buffer to nextion 
    //Serial1.printf("t0.txt=\"Waiting for temp\"\xFF\xFF\xFF");
    //Serial.printlnf("t0.txt=\"Waiting for temp\"\xFF\xFF\xFF");
    delay(10000);
  }
}

void cityWrite() {//thread for internal temp read, conversion, and writing to display
  Serial.println("cityWrite:entered");
  while(1) {
    char dataCity [64];
    sprintf(dataCity, "t2.txt=\"%s,%s\"\xFF\xFF\xFF", global_city.c_str(), global_regionCode.c_str()); //writes city and state to buffer
    Serial.println(dataCity);//writes buffer to usb serial
    Serial1.printf(dataCity);//writes buffer to nextion
    delay(10000);
  }
}

void loop(){
  delay(CONCURRENT_WAIT_FOREVER);
} 