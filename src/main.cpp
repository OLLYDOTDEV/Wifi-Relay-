#include <Arduino.h>
#include "HardwareSerial.h"
#include "UI.h"
#include "Networking.h"
#include "Hardware_Control.h"



int G_lastmode = 100;
int G_CurrentMode = 0;
int G_HeartBeat = 0;
int G_lastbeat = 0;
int G_statuscheck = 0;
char G_date_str[10] {""};
//bool schedule[24] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

bool schedule[24] = {};
char date_str[32] = "";





void setup() {
// ClearState();  // Keep commented unless required [will clear set NVM keys]

Initalize_Hardware(5);

Initalize_NewNetwork(); // Uncomment line to setup wifi network

Initalize_ExistingNetwork();




  // Serial.println("");

  // Serial.println("Schedule loaded to EEPROM");
  // loadMode(G_CurrentMode);
  // loadSchedule();  // load schedule array from NVM
  

//Initalize_UI();




}


int i = 1; 

void loop() {


i++;
Serial.println(i);

if(i>=1000){
Serial.println("Poweroff");
delay(500);
ESP.restart();
}


// Network connection

// Get time

// web server 

//WebserverSubroutine();


// checkWiFi(WIFI_ssid,WIFI_password); // 

//TimedRestart(int CurrentMode, int currentMinutes)
//HeartBeat(G_HeartBeat,G_date_str);

yield();
delay(50);
}

