#include "Networking.h"
#include "HardwareSerial.h"
#include "Persistence_Data.h"


// Replace with your network credentials


// New Zealand Daylight Saving Time rules
TimeChangeRule nzDst = { "NZDT", Last, Sun, Sep, 2, 780 };  // Daylight time = UTC + 13 hours
TimeChangeRule nzStd = { "NZST", Last, Sun, Apr, 3, 720 };  // Standard time = UTC + 12 hours
Timezone TZ_NZ(nzDst, nzStd);

// Wifi UDP variables 
WiFiUDP Udp;
const int NTP_PACKET_SIZE = 48;      // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];  // Buffer to hold incoming and outgoing packets



void Initalize_NewNetwork(){

  Serial.println("Wifi credentials missing - Initalizing setup");

  // Serial.println("Size of array:");
  // Serial.println(sizeof(WIFI_SSID));

  char WIFI_SSID[100] {}; 
  char WIFI_Password[100] {};


  GetInput("Input WIFI SSID:",WIFI_SSID);
  SaveValue("WIFI_SSID",WIFI_SSID,sizeof(WIFI_SSID));

  GetInput("Input WIFI Password:",WIFI_Password);
  SaveValue("WIFI_Password",WIFI_Password,sizeof(WIFI_SSID));



  // for (uint i = 0; i <= sizeof(WIFI_SSID)/sizeof(WIFI_SSID[0])-1; i++){ // Debug returned valve from GetInput()
  // Serial.print(i);
  // Serial.print(" : ");
  // Serial.println(WIFI_SSID[i]);
  // }
}



void Initalize_ExistingNetwork(){

  char WIFI_SSID[100] {}; 
  char WIFI_Password[100] {};


  if (LoadValue("WIFI_SSID",WIFI_SSID,sizeof(WIFI_SSID)) == false || LoadValue("WIFI_Password",WIFI_Password,sizeof(WIFI_SSID)) == false){
    // If the network credentials missing from , setup and then reload the values into the current function.
    Initalize_NewNetwork(); 
    LoadValue("WIFI_SSID",WIFI_SSID,sizeof(WIFI_SSID));
    LoadValue("WIFI_Password",WIFI_Password,sizeof(WIFI_SSID));
  }

  Serial.println("Initializing Wifi connection");
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_Password); // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    // WiFi.printDiag(Serial); // Debug connection issues
    delay(1000);
    Serial.println();
  }


  Serial.println("WiFi connected");
  WiFi.hostname("Wifi Relay");
  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address to Serial Monitor
  Serial.println("WiFi status: " + String(WiFi.status()) + "\n");
  Serial.println("Starting UDP");
  Udp.begin(8888);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("Waiting for sync");
  setSyncProvider(getNtpTime);
  //setSyncInterval(86400);
  setSyncInterval(60);
  Serial.println("Synced\n");
  delay(5000);
  }

void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin();

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
  time_t local = TZ_NZ.toLocal(utc);
  sprintf(date_str, "%04d-%02d-%02d %02d:%02d", year(local), month(local), day(local), hour(local), minute(local));
 }

 int GetHours(){
  time_t utc = now();
  time_t local = TZ_NZ.toLocal(utc);
  return hour(local);  // Update the current hour variable
 }


 int GetMinutes(){
  time_t utc = now();
  time_t local = TZ_NZ.toLocal(utc);
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




