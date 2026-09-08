#include "Hardware_Control.h"
#include "HardwareSerial.h"


void Initalize_Hardware(const int RelayPin){

  pinMode(RelayPin, OUTPUT);        // Set relay pin as output
  PinMode_Function(RelayPin, LOW,HIGH);  // Initialize relay to off
  Serial.begin(115200);
  while (!Serial) {}; // Wait for setup to be ready
  Serial.println();
	delay(1000);
	Serial.println("Serial Port Started \n");
	Serial.flush();
	delay(1000);
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

void GetInput(char* InputMsg,char* Array){

Serial.println(InputMsg); 
  
int InputAccepted = false;

while (InputAccepted == false){

  if (Serial.available() > 0){
      int n = Serial.readBytes(Array,98);
			Array[n] = '\0';
      Serial.print("Buffer Value received: ");
      Serial.println(Array);
			Serial.println("Confirm Input? (Y/N)");
      while (Serial.available() == 0 ){}
      char ShortBuff[0];
      Serial.readBytes(ShortBuff,1);

      InputAccepted = (ShortBuff[0] == 'y' || ShortBuff[0] == 'Y') ? true : false;
      if (InputAccepted == false){
        Serial.println("Input rejected by user, repeating input prompt. \n");
        Serial.println(InputMsg); 
      }
    }
	yield();

	}

delay(200);  
      Serial.println("Input Confirmed.\n\n\n");
}


