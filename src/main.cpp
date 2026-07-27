#include <Arduino.h>
#include "Persistence_Data.h"
#include "UI.h"
#include "Networking.h"
#include "Hardware_Control.h"


#define relayPin 5  // GPIO5 connected to relay control pin


int G_lastmode = 100;
int G_CurrentMode = 0;
int G_HeartBeat = 0;
int G_lastbeat = 0;
int G_statuscheck = 0;




void setup() {

Initalize_Hardware();
Initalize_ExistingNetwork();
Initalize_UI();

}




void loop() {

  unsigned long seconds_remaining = timerduration / 1000;
  unsigned long minutes_remaining = seconds_remaining / 60;
  unsigned long hours_remaining = minutes_remaining / 60;
  seconds_remaining = seconds_remaining % 60;
  minutes_remaining = minutes_remaining % 60;
  char formattedTime_remaining[9] = " ";  // HH:MM:SS\0
  sprintf(formattedTime_remaining, "%02lu:%02lu:%02lu", hours_remaining, minutes_remaining, seconds_remaining);





  // handle network. loopHook() simply calls server.handleClient(), in most but not all server implementations.
  driver.loopHook();


  // Check the webserver is still working and also helps to save CPU cycles
  //if (currentMinutes == 30) {



  if (millis() - statuscheck >= 30000) {
    statuscheck = millis();
    Serial.println("Restarting Webserver...");
    server.stop();
    server.begin();
    checkWiFi();
  }



  // }

  // if (WebErrorCount == 2 and millis() >= 60000) {

  //   Serial.println("Web Error");
  //   ESP.restart();
  // }





  if (CurrentMode == 0 || CurrentMode == 1) {  // Override - OFF

    if (lastmode != CurrentMode) {  // execute code only once  on mode switch
      lastmode = CurrentMode;
      Serial.print("Selected mode: ");
      Serial.println(CurrentMode);
      pinMode_function(relayPin, LOW);
    }



  } else if (CurrentMode == 2) {    // Override - ON
    if (lastmode != CurrentMode) {  // execute code only once on mode switch
      lastmode = CurrentMode;
      Serial.print("Selected mode: ");
      Serial.println(CurrentMode);
      pinMode_function(relayPin, HIGH);
    }
  }



  else if (CurrentMode == 3) {  // Automatic schedule

    for (int i = 0; i < 24; i++) {  // Update UI for current schedule
      Set_schedule[i].setValue(schedule[i] ? "✔️" : "❌");
    }


    timerduration = 0;  // reset timer
    if (schedule[CurrentMode]) {
      pinMode_function(relayPin, HIGH);  // Turn relay ON
    } else {
      pinMode_function(relayPin, LOW);  // Turn relay OFF
    }
  }


  else if (CurrentMode == 4) {      // Override - Delayed Timer
    if (lastmode != CurrentMode) {  // execute code only once on mode switch
      lastmode = CurrentMode;
      Serial.print("Selected mode: ");
      Serial.println(CurrentMode);
      pinMode_function(relayPin, LOW);
      timerduration = 0;  // reset timer
    }

    currenttime = millis();
    timepassed = currenttime - starttime;




    Remaining_Timer.setValue(formattedTime_remaining);

    //  Remaining_Timer.setValue((const char*) formattedTime_remaining);




    //Remaining_Timer.setValue(formattedTime_remaining);
    //Remaining_Timer.setValue(strncpy(formattedTime_remaining,formattedTime_remaining,BUFLEN),true);






    if (timepassed >= 1000 && timerduration != 0) {
      starttime = millis();
      Serial.print("Remaining duration: ");
      Serial.println(formattedTime_remaining);


      if (timerduration < 1000) {

        timerduration = 0;  // provent int overflow
      } else {
        timerduration = timerduration - 1000;  // decress timer by 1 second
      }
    }

    if (timerduration == 0) {
      pinMode_function(relayPin, LOW);
      Remaining_Timer.setValue("No Timer");

    } else {
      pinMode_function(relayPin, HIGH);
    }
  }



  char date_str[32] = "";
  timestring();


  Relay_enabled_status.setValue((relay_status == HIGH) ? "<span style=\"background-color:rgba(0,128,0,1); \">Relay Status: Enabled</span>" : "<span style=\"background-color:rgba(255,0,0,1); \">Relay Status: Disabled</span>", true);









  // Calling fuctions to handles webserver and Memory issues and other soft locks

  if (CurrentMode == 11 && currentMinutes == 59) {  // reboot at 11:59 am
    pinMode_function(relayPin, LOW);
    Serial.println("Automatic Reboot in 60 seconds");
    delay(1200000);  // ensure wont reboot twice in the same minute
    ESP.restart();   //
  }





  if (millis() - HeartBeat >= 10000) {
    HeartBeat = millis();
    Serial.print("HeartBeat: ");
    Serial.println(date_str);
  }
  delay(1);
}

