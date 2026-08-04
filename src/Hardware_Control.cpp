#include "Hardware_Control.h"


void Initalize_Hardware(int RelayPin){

  pinMode(RelayPin, OUTPUT);        // Set relay pin as output
  PinMode_Function(RelayPin, LOW,HIGH);  // Initialize relay to off
  Serial.begin(115200);
  delay(1500);
  Serial.println("Serial Port Started \n");
}

void PinMode_Function(int pin, bool state,bool relay_status) {

  if (relay_status != state) {
    relay_status = state;
    Serial.print("Pin State: ");
    Serial.println(state);
    //delay(1000);  // delays required to prevent watchdog timer
    digitalWrite(pin, state);
    //delay(4000);
  }
}


void TimedRestart(int CurrentMode, int currentMinutes){
  if (CurrentMode == 11 && currentMinutes == 59) {  // reboot at 11:59 am
    //pinMode_function(relayPin, LOW);
    Serial.println("Automatic Reboot in 60 seconds");
    delay(1200000);  // ensure wont reboot twice in the same minute
    ESP.restart();   //
  }
}


void HeartBeat(int HeartBeat,char* date_str){
  if (millis() - HeartBeat >= 10000) {
    HeartBeat = millis();
    Serial.print("HeartBeat: ");
    Serial.println(date_str);
  }
}