#include <Preferences.h>

Preferences preferences;

void saveSchedule() {
  preferences.begin("schedule", false);                          // Open namespace "schedule"
  preferences.putBytes("schedule", schedule, sizeof(schedule));  // Save schedule array
  preferences.end();
  delay(250);  // Close namespace
}

void loadSchedule() {
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



void saveMode() {
  preferences.begin("mode", false);
  preferences.putInt("currentMode", currentMode);
  preferences.end();
  Serial.println("Mode saved");
  delay(250);
}

void loadMode() {
  preferences.begin("mode", true);
  if (preferences.isKey("currentMode")) {
    currentMode = preferences.getInt("currentMode", 0);
    selectMode();
  } else {
    Serial.println("Failed to load from EEPROM");
    currentMode = 0;  // Set a default value
  }
  preferences.end();
  delay(250);
}

void ClearState() {
  Serial.println("Clearing Saved keys");
  preferences.begin("mode", false);
  preferences.clear();
  preferences.end();


  preferences.begin("schedule", false);
  preferences.clear();
  preferences.end();


  currentMode = 0;
  bool schedule[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
}