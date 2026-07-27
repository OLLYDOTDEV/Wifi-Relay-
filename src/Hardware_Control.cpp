#include "Hardware_Control.h"


void Initalize_Hardware(){

  pinMode(relayPin, OUTPUT);        // Set relay pin as output
  pinMode_function(relayPin, LOW);  // Initialize relay to off
  Serial.begin(115200);
  delay(1500);
  Serial.println("Serial Port Started \n");
}

void pinMode_function(int pin, bool state,bool relay_status) {

  if (relay_status != state) {
    updateUI();  // keeps UI updates faster
    relay_status = state;
    Serial.print("Pin State: ");
    Serial.println(state);
    delay(1000);  // delays required to prevent watchdog timer
    digitalWrite(pin, state);
    delay(4000);
  }
}
