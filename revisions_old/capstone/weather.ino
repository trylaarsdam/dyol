#include <google-maps-device-locator.h>
#include "Particle.h"
#include "application.h"
#include <JsonParserGeneratorRK.h>

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(MANUAL);

Thread externalTempThread;
unsigned int nextTime = 30;    // Next time to contact the server
String tempReceived;
JsonParserStatic<512, 50> jsonParser;
String global_lat = ""; //variables for location data
String global_lon = "";
String global_city = "";
String global_regionCode = "";
String global_ip = ""; //through here

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

void setup(){
    externalTempThread = Thread("externalTemp", externalTempFunction); //GETs webhook data and parses it and sends it to screenWrite
    Serial.begin(9600);
    Particle.subscribe("particle/device/ip", ipHandler);//subscribes to global IP get
    WiFi.on();
    WiFi.clearCredentials();
    WiFi.setCredentials("WCL2", "atmega328");
    WiFi.connect();

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

void loop(){
  delay(CONCURRENT_WAIT_FOREVER);
}