#include <cstdio>
#include "Particle.h"

SYSTEM_THREAD(ENABLED);

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

void tempHandler(const char *event, const char *data) {
  
}

void setup() {
  Serial.begin(9600);
  Particle.subscribe("hook-response/tempRead", tempHandler, MY_DEVICES);
}

void loop() {
  externalTempFunction();
}