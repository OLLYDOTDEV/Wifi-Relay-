



#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>  // https://github.com/JChristensen/Timezone
#include <TimeLib.h>





// Replace with your network credentials
const char* ssid = "testtest";
const char* password = "testtest";


// NTP Server


const char* ntpServer = "nz.pool.ntp.org";
static const char ntpServerName[] = "nz.pool.ntp.org";


// New Zealand Daylight Saving Time rules
TimeChangeRule nzDst = { "NZDT", Last, Sun, Sep, 2, 780 };  // Daylight time = UTC + 13 hours
TimeChangeRule nzStd = { "NZST", Last, Sun, Apr, 3, 720 };  // Standard time = UTC + 12 hours

Timezone nz(nzDst, nzStd);

WiFiUDP Udp;
unsigned int localPort = 8888;

const int NTP_PACKET_SIZE = 48;      // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];  // Buffer to hold incoming and outgoing packets

time_t getNtpTime();
void sendNTPpacket(IPAddress& address);






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

void timestring() {
  time_t utc = now();
  time_t local = nz.toLocal(utc);
  currentHour = hour(local);  // Update the current hour variable
  currentMinutes = minute(local);
  sprintf(date_str, "%04d-%02d-%02d %02d:%02d", year(local), month(local), day(local), hour(local), minute(local));
  Current_time_status.setValue(date_str);
}

// Functions below required for NTB timer server sync
time_t getNtpTime() {
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



