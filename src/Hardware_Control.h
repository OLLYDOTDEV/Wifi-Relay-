#pragma once

#include <Arduino.h>
#include "Networking.h"

void Initalize_Hardware(int RelayPin);
void PinMode_Function(int pin, bool state,bool relay_status);
void TimedRestart(int CurrentMode, int currentMinutes);
void HeartBeat(int HeartBeat,char* date_str);
