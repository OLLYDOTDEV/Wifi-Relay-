#include <Arduino.h>
#include "UI.h"
#include "Networking.h"
#include "Hardware_Control.h"


#define relayPin 5  // GPIO5 connected to relay control pin


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

// Initalize_Hardware(5);

// Check if ssid & password saved
// if true
//Initalize_ExistingNetwork();
// else call function get network info an wait for PW over serial.  
// wait untill network has been connected
//Initalize_UI();

}



void loop() {

// Network connection

// Get time

// web server 



//WebserverSubroutine();


//TimedRestart(int CurrentMode, int currentMinutes)
//HeartBeat(G_HeartBeat,G_date_str);
}

