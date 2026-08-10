#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "UI.h"

void saveSchedule();
void loadSchedule();
void saveMode(int CurrentMode);
void loadMode(int CurrentMode);
void ClearState();
void ClearState(int CurrentMode);