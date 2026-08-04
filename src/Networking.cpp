#include "Networking.h"

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// New Zealand Daylight Saving Time rules
TimeChangeRule nzDst = { "NZDT", Last, Sun, Sep, 2, 780 };  // Daylight time = UTC + 13 hours
TimeChangeRule nzStd = { "NZST", Last, Sun, Apr, 3, 720 };  // Standard time = UTC + 12 hours
Timezone nz(nzDst, nzStd);

// Wifi UDP variables 
WiFiUDP Udp;
const int NTP_PACKET_SIZE = 48;      // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];  // Buffer to hold incoming and outgoing packets




void Initalize_ExistingNetwork(int CurrentMode){
  WiFi.begin(ssid, password);  // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("Schedule loaded to EEPROM");

  loadMode(CurrentMode);
  loadSchedule();  // load schedule array from NVM
  // ClearState();  // Keep Comneted unless required [will clear set NVM keys]

  Serial.println("WiFi connected");
  Serial.println("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address to Serial Monitor
  Serial.println("WiFi status: " + String(WiFi.status()));
  Serial.println("Starting UDP");
  Udp.begin(8888);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("Waiting for sync");
  setSyncProvider(getNtpTime);
  //setSyncInterval(86400);
  setSyncInterval(60);
  }






void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startAttemptTime >= 60000) {  // 60 seconds timeout
        Serial.println("\nFailed to reconnect to WiFi within 60 seconds.");
        return;  // Exit the function
      }
      delay(500);
      Serial.print(".");
    }

    Serial.println("\nReconnected to WiFi");
  }
}

void timestring(char* date_str) {
  time_t utc = now();
  time_t local = nz.toLocal(utc);
  sprintf(date_str, "%04d-%02d-%02d %02d:%02d", year(local), month(local), day(local), hour(local), minute(local));
 }

 int GetHours(){
  time_t utc = now();
  time_t local = nz.toLocal(utc);
  return hour(local);  // Update the current hour variable
 }


 int GetMinutes(){
  time_t utc = now();
  time_t local = nz.toLocal(utc);
  return minute(local);
 }


// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123);  //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

// Functions below required for NTB timer server sync
time_t getNtpTime() {
  // Set NTP Server
  //const char* ntpServer = "nz.pool.ntp.org";
  static const char ntpServerName[] = "nz.pool.ntp.org";

  IPAddress ntpServerIP;  // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ;  // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0;  // return 0 if unable to get the time
}




