#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Preferences.h>
#include "UI.h"
#include "Hardware_Control.h"
#include "Persistence_Data.h"

#include "HardwareSerial.h"

void SaveValue(const char* Key,char* Data, int Data_Size); 
void LoadValue(const char* Key,char* Data, int Data_Size);
void ClearState();
void ClearState(int CurrentMode);