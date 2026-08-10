#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>  // https://github.com/JChristensen/Timezone
#include <TimeLib.h>
#include "Persistence_Data.h"

void checkWiFi(char* ssid, char* password);
void timestring(char* date_str);
int GetHours();
int GetMinutes();
void sendNTPpacket(IPAddress& address);
time_t getNtpTime();

void Initalize_ExistingNetwork(char* ssid, char* password);
