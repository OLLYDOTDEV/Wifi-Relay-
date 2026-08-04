#include <Persistence_Data.h>
#include "UI.h"
Preferences preferences;

 

void saveSchedule(int* schedule) {
  preferences.begin("schedule", false);                          // Open namespace "schedule"
  preferences.putBytes("schedule", schedule, sizeof(schedule));  // Save schedule array
  preferences.end();
  delay(250);  // Close namespace
}

void loadSchedule(int* schedule) {
  preferences.begin("schedule", true);  // Open namespace "schedule" (read-only)

  if (preferences.isKey("schedule")) {
    // Load schedule array from flash memory
    preferences.getBytes("schedule", schedule, sizeof(schedule));
  } else {
    Serial.println("Failed to load from EEPROM");
  }
  preferences.end();  // Close namespace
  delay(250);
}



void saveMode(int CurrentMode) {
  preferences.begin("mode", false);
  preferences.putInt("CurrentMode", CurrentMode);
  preferences.end();
  Serial.println("Mode saved");
  delay(250);
}

void loadMode(int CurrentMode) {
  preferences.begin("mode", true);
  if (preferences.isKey("CurrentMode")) {
    CurrentMode = preferences.getInt("CurrentMode", 0);
    selectMode();
  } else {
    Serial.println("Failed to load from EEPROM");
    CurrentMode = 0;  // Set a default value
  }
  preferences.end();
  delay(250);
}

void ClearState(int CurrentMode, char* schedule) {
  Serial.println("Clearing Saved keys");
  preferences.begin("mode", false);
  preferences.clear();
  preferences.end();


  preferences.begin("schedule", false);
  preferences.clear();
  preferences.end();


  CurrentMode = 0;
  //schedule[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
}