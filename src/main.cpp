#include <Arduino.h>
#include "UI.h"
#include "Networking.h"
#include "Hardware_Control.h"


#define relayPin 5  // GPIO5 connected to relay control pin


int i = 1; 


int G_lastmode = 100;
int G_CurrentMode = 0;
int G_HeartBeat = 0;
int G_lastbeat = 0;
int G_statuscheck = 0;
char G_date_str[10] {""};
//bool schedule[24] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

bool schedule[24] = {};
char date_str[32] = "";



char WIFI_ssid[100] {0}; // = restoreNetworkCreditianls(ssid)); 
char WIFI_password[100] {0};  // = restoreNetworkCreditianls(password); 

void setup() {

Initalize_Hardware(5);

// Check if ssid & password saved
// add function to restore saved SSID and password
// restoreNetworkCreditianls(); 

//  if (WIFI_ssid[0] == '\0' or WIFI_password[0] == '\0'){
// Serial.println("Networking configurations missing - Initalizing setup");

char Msg[20] = "Input WIFI SSID:";
GetInput(Msg,WIFI_ssid);


// else call function get network info an wait for PW over serial.  
// wait untill network has been connected
// Initalize_NewNetwork();
// }

//Initalize_ExistingNetwork(WIFI_ssid,WIFI_password);




  // Serial.println("");

  // Serial.println("Schedule loaded to EEPROM");
  // loadMode(G_CurrentMode);
  // loadSchedule();  // load schedule array from NVM
  // ClearState();  // Keep commented unless required [will clear set NVM keys]


//Initalize_UI();




}



void loop() {

Serial.println(i);
i++;



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

