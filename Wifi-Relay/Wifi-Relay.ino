#include <ESP8266WiFi.h>
#include <EmbAJAX.h>

//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>  // https://github.com/JChristensen/Timezone
#include <TimeLib.h>

#include <Preferences.h>

Preferences preferences;

// Replace with your network credentials
const char* ssid = "testtest";
const char* password = "testtest";

#define relayPin 5  // GPIO5 connected to relay control pin

#define BUFLEN 10

int lastmode = 100;


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

int currentHour = 0;  // Variable to store the current hour in 24-hour format


// 24 hours array
bool schedule[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // Initialize with all hours OFF







EmbAJAXOutputDriverWebServerClass server(80);
EmbAJAXOutputDriver driver(&server);

long int starttime = 0;
int currenttime = 0;
int timepassed = 0;
int timerduration = 0;

bool relay_status = 0;

// Radio selector for mode select
const char* modes[] = { "Override - Off", "Override - On", "Automatic schedule", "Delayed Timer" };
EmbAJAXRadioGroup<4> Radio_mode("mode", modes);



// Automated

EmbAJAXMomentaryButton m_button_schedule_Set("Set_schedule", "Set Schedule");  // Timer set

EmbAJAXStatic nextCell("</td><td>&nbsp;</td><td>");
EmbAJAXStatic nextRow("</div></td></tr><tr><td>");

EmbAJAXCheckButton hours[] = {

  EmbAJAXCheckButton("Hours0", "12:00 AM - 01:00 AM"),
  EmbAJAXCheckButton("Hours1", "01:00 AM - 02:00 AM"),
  EmbAJAXCheckButton("Hours2", "02:00 AM - 03:00 AM"),
  EmbAJAXCheckButton("Hours3", "03:00 AM - 04:00 AM"),
  EmbAJAXCheckButton("Hours4", "04:00 AM - 05:00 AM"),
  EmbAJAXCheckButton("Hours5", "05:00 AM - 06:00 AM"),
  EmbAJAXCheckButton("Hours6", "06:00 AM - 07:00 AM"),
  EmbAJAXCheckButton("Hours7", "07:00 AM - 08:00 AM"),
  EmbAJAXCheckButton("Hours8", "08:00 AM - 09:00 AM"),
  EmbAJAXCheckButton("Hours9", "09:00 AM - 10:00 AM"),
  EmbAJAXCheckButton("Hours10", "10:00 AM - 11:00 AM"),
  EmbAJAXCheckButton("Hours11", "11:00 AM - 12:00 PM"),
  EmbAJAXCheckButton("Hours12", "12:00 PM - 01:00 PM"),
  EmbAJAXCheckButton("Hours13", "01:00 PM - 02:00 PM"),
  EmbAJAXCheckButton("Hours14", "02:00 PM - 03:00 PM"),
  EmbAJAXCheckButton("Hours15", "03:00 PM - 04:00 PM"),
  EmbAJAXCheckButton("Hours16", "04:00 PM - 05:00 PM"),
  EmbAJAXCheckButton("Hours17", "05:00 PM - 06:00 PM"),
  EmbAJAXCheckButton("Hours18", "06:00 PM - 07:00 PM"),
  EmbAJAXCheckButton("Hours19", "07:00 PM - 08:00 PM"),
  EmbAJAXCheckButton("Hours20", "08:00 PM - 09:00 PM"),
  EmbAJAXCheckButton("Hours21", "09:00 PM - 10:00 PM"),
  EmbAJAXCheckButton("Hours22", "10:00 PM - 11:00 PM"),
  EmbAJAXCheckButton("Hours23", "11:00 PM - 12:00 AM")
};

EmbAJAXMutableSpan Set_schedule[] = {

  EmbAJAXMutableSpan("Hours0_Set"),
  EmbAJAXMutableSpan("Hours1_Set"),
  EmbAJAXMutableSpan("Hours2_Set"),
  EmbAJAXMutableSpan("Hours3_Set"),
  EmbAJAXMutableSpan("Hours4_Set"),
  EmbAJAXMutableSpan("Hours5_Set"),
  EmbAJAXMutableSpan("Hours6_Set"),
  EmbAJAXMutableSpan("Hours7_Set"),
  EmbAJAXMutableSpan("Hours8_Set"),
  EmbAJAXMutableSpan("Hours9_Set"),
  EmbAJAXMutableSpan("Hours10_Set"),
  EmbAJAXMutableSpan("Hours11_Set"),
  EmbAJAXMutableSpan("Hours12_Set"),
  EmbAJAXMutableSpan("Hours13_Set"),
  EmbAJAXMutableSpan("Hours14_Set"),
  EmbAJAXMutableSpan("Hours15_Set"),
  EmbAJAXMutableSpan("Hours16_Set"),
  EmbAJAXMutableSpan("Hours17_Set"),
  EmbAJAXMutableSpan("Hours18_Set"),
  EmbAJAXMutableSpan("Hours19_Set"),
  EmbAJAXMutableSpan("Hours20_Set"),
  EmbAJAXMutableSpan("Hours21_Set"),
  EmbAJAXMutableSpan("Hours22_Set"),
  EmbAJAXMutableSpan("Hours23_Set")
};


EmbAJAXBase* hours_contents_array[] = {
  new EmbAJAXStatic("<center><span>Key: <br>&#x2713; added to schedule</span><p> x removed from schedule </p></center><center><table><caption>Current schedule</caption><tbody><tr><td>"),
  &Set_schedule[0],
  &hours[0],
  &nextCell,
  &Set_schedule[1],
  &hours[1],
  &nextRow,
  &Set_schedule[2],
  &hours[2],
  &nextCell,
  &Set_schedule[3],
  &hours[3],
  &nextRow,
  &Set_schedule[4],
  &hours[4],
  &nextCell,
  &Set_schedule[5],
  &hours[5],
  &nextRow,
  &Set_schedule[6],
  &hours[6],
  &nextCell,
  &Set_schedule[7],
  &hours[7],
  &nextRow,
  &Set_schedule[8],
  &hours[8],
  &nextCell,
  &Set_schedule[9],
  &hours[9],
  &nextRow,
  &Set_schedule[10],
  &hours[10],
  &nextCell,
  &Set_schedule[11],
  &hours[11],
  &nextRow,
  &Set_schedule[12],
  &hours[12],
  &nextCell,
  &Set_schedule[13],
  &hours[13],
  &nextRow,
  &Set_schedule[14],
  &hours[14],
  &nextCell,
  &Set_schedule[15],
  &hours[15],
  &nextRow,
  &Set_schedule[16],
  &hours[16],
  &nextCell,
  &Set_schedule[17],
  &hours[17],
  &nextRow,
  &Set_schedule[18],
  &hours[18],
  &nextCell,
  &Set_schedule[19],
  &hours[19],
  &nextRow,
  &Set_schedule[20],
  &hours[20],
  &nextCell,
  &Set_schedule[21],
  &hours[21],
  &nextRow,
  &Set_schedule[22],
  &hours[22],
  &nextCell,
  &Set_schedule[23],
  &hours[23],

  new EmbAJAXStatic("</b></td></tr></table>"),
  &m_button_schedule_Set,
  new EmbAJAXStatic("<br></center>")
};



EmbAJAXHideableContainer<74> hours_contents("hideable", hours_contents_array);
// Delayed Timer
EmbAJAXMomentaryButton m_button_Timer_Set("Timer_Set", "Timer_Set");  // Timer set


EmbAJAXTextInput<BUFLEN> input_time_duration("input_time_duration");
char input_time_duration_b[BUFLEN];


const char* Timer_Array[] = { "10min", "30min", "1 hour", "3 hours", "6 hours" };
EmbAJAXOptionSelect<5> Dropdown_Time("Time_Array", Timer_Array);



EmbAJAXMutableSpan Relay_enabled_status("relay_status");
EmbAJAXMutableSpan Current_time_status("Current_time");
char Current_timeM_status_b[BUFLEN];
char Current_timeH_status_b[BUFLEN];




// Define a page (named "page") with our elements of interest, above, interspersed by some uninteresting
// static HTML. Note: MAKE_EmbAJAXPage is just a convenience macro around the EmbAJAXPage<>-class.
MAKE_EmbAJAXPage(page, "Wifi Relay Interface",
                 "<meta charset=\"UTF-8\">\n"
                 "<style>\n"
                 "@charset \"utf-8\";\n"
                 "div.unselectable {\n"
                 "  -webkit-touch-callout: none;\n"
                 " -webkit-user-select: none;\n"
                 " -khtml-user-select: none;\n"
                 " -moz-user-select: none;\n"
                 " -ms-user-select: none;\n"
                 " user-select: none;\n"
                 "}\n"
                 "*{font-family:Fantasy}\n"
                 "</style>",
                 // Page Header



                 new EmbAJAXStatic("<h1>Wifi Relay - Overview </h1><p>Control mode: "),
                 &Radio_mode,
                 new EmbAJAXStatic("<hr><br>"),




                 // Override Display Control - On

                 // Override Display Control - Off

                 // Automatic Display Control

                 &hours_contents,


                 // Timer  Display Control

                 &Dropdown_Time,


                 &m_button_Timer_Set,


                 // Page Footer
                 new EmbAJAXStatic("<br><hr><center><table><tr><td>Connection status:</td><td>"),
                 new EmbAJAXConnectionIndicator(),
                 new EmbAJAXStatic("</td><td>"),
                 &Relay_enabled_status,
                 new EmbAJAXStatic("</td><td><p>    Timestamp: </p></td><td>"),
                 &Current_time_status,
                 new EmbAJAXStatic("</td></tr></center></table>"));


void pinMode_function(int pin, bool state) {

  if (relay_status != state) {
    relay_status = state;
    digitalWrite(pin, state);
    Serial.print("Pin State: ");
    Serial.println(state);
  }
}


void setup() {

  pinMode(relayPin, OUTPUT);        // Set relay pin as output
  pinMode_function(relayPin, LOW);  // Initialize relay to off

  Serial.println("Schedule Saved to EEPROM");
  loadSchedule();  // load schedule array from NVM

  // Setup Network
  Serial.begin(115200);
  WiFi.begin(ssid, password);  // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  delay(5000);
  Serial.println("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address to Serial Monitor
  Serial.println("WiFi status: " + String(WiFi.status()));

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("Waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(360);



  // Create Pages

  driver.installPage(&page, "/", updateUI);
  server.begin();

  updateUI();  // init displays
}





void updateUI() {
  // Enabled / disable the slider. Note that you could simply do this inside the loop. However,
  // placing it here makes the client UI more responsive (try it).
  // timeractive.setEnabled(timer!= 0);

  // Override Display Control - On


  // Override Display Control - Off

  // Automatic Display Control

  hours_contents.setVisible(Radio_mode.selectedOption() == 2);




  if (m_button_schedule_Set.status() == EmbAJAXMomentaryButton::Pressed) {


    Serial.println("Schedule submitted");

    for (int i = 0; i < 24; i++) {
      schedule[i] = hours[i].isChecked();
    }
    Serial.println("Schedule updated:");
    for (int i = 0; i < 24; i++) {
      Serial.print("Hour ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(schedule[i]);
    }

    saveSchedule();  // Save current schedule
    Serial.println("Schedule save to EEPROM");
  }









  // Timer  Display Control


  Dropdown_Time.setVisible(Radio_mode.selectedOption() == 3);
  input_time_duration.setVisible(Radio_mode.selectedOption() == 3);
  m_button_Timer_Set.setVisible(Radio_mode.selectedOption() == 3);

  // Save timer variables
  if (m_button_Timer_Set.status() == EmbAJAXMomentaryButton::Pressed) {

    Serial.println("Timer Data Submitted");
    switch (Dropdown_Time.selectedOption()) {
      case 0:
        timerduration = 10;
        break;
      case 1:
        timerduration = 30;
        break;
      case 2:
        timerduration = 60;
        break;
      case 3:
        timerduration = 180;
        break;
      case 4:
        timerduration = 360;
        break;
      case 5:
        timerduration = 0;  // Not used yet
        break;
      case 6:
        timerduration = 0;  // Not used yet
        break;
      default:
        Serial.println("Error Invalid Timer duration ");
        break;
    }
    Serial.print("Selected Timer duration: ");
    Serial.println(timerduration);
    timerduration = timerduration * 100;  // timer scaler value
    starttime = millis();                 // Save the time when the button was pushed
  }
}

// Footer Display




void loop() {
  // handle network. loopHook() simply calls server.handleClient(), in most but not all server implementations.
  driver.loopHook();

  // And these lines are all you have to write for the logic: Access the elements as if they were plain
  // local controls
  if (Radio_mode.selectedOption() == 0) {  // Override - OFF

    if (lastmode != Radio_mode.selectedOption()) {  // execute code only once  on mode switch
      lastmode = Radio_mode.selectedOption();
      Serial.print("Selected mode: ");
      Serial.println(Radio_mode.selectedOption());
      pinMode_function(relayPin, LOW);
    }



  } else if (Radio_mode.selectedOption() == 1) {    // Override - ON
    if (lastmode != Radio_mode.selectedOption()) {  // execute code only once on mode switch
      lastmode = Radio_mode.selectedOption();
      Serial.print("Selected mode: ");
      Serial.println(Radio_mode.selectedOption());
      pinMode_function(relayPin, HIGH);
    }
  }



  else if (Radio_mode.selectedOption() == 2) {  // Automatic schedule

    //   Serial.print("Current Now:");
    //  Serial.println(currentHour);
    if (schedule[currentHour]) {
      pinMode_function(relayPin, HIGH);  // Turn relay ON
    } else {
      pinMode_function(relayPin, LOW);  // Turn relay OFF
    }
  }


  else if (Radio_mode.selectedOption() == 3) {      // Override - ON
    if (lastmode != Radio_mode.selectedOption()) {  // execute code only once on mode switch
      lastmode = Radio_mode.selectedOption();
      Serial.print("Selected mode: ");
      Serial.println(Radio_mode.selectedOption());
      pinMode_function(relayPin, HIGH);
      timerduration = 0;  // reset timer
    }

    currenttime = millis();
    timepassed = currenttime - starttime;

    if (timepassed >= 1000 && timerduration != 0) {
      starttime = millis();
      Serial.print("Remaining duration: ");
      Serial.println(timerduration);

      if (timerduration < 1000) {

        timerduration = 0;  // provent int overflow
      } else {
        timerduration = timerduration - 1000;  // decress timer by 1 second
      }
    }

    if (timerduration != 0) {

      pinMode_function(relayPin, HIGH);
    } else {
      pinMode_function(relayPin, LOW);
    }
  }


  char date_str[32];
  time_t utc = now();
  time_t local = nz.toLocal(utc);
  currentHour = hour(local);  // Update the current hour variable
  sprintf(date_str, "%04d-%02d-%02d %02d:%02d", year(local), month(local), day(local), hour(local), minute(local));

  // Print full date
  //Serial.print("Date: ");
  //Serial.println(date_str);
  Current_time_status.setValue(date_str);



  for (int i = 0; i < 24; i++) { // Update UI for current schedule
    Set_schedule[i].setValue(schedule[i] ? "✔️" : "❌");
  }

  Relay_enabled_status.setValue((relay_status == HIGH) ? "Relay Status: Enabled" : "Relay Status: Disabled");

  delay(1);
}



// Functions below required for NTB timer server sync
time_t getNtpTime() {
  IPAddress ntpServerIP;

  while (Udp.parsePacket() > 0)
    ;  // Discard any previously received packets
  Serial.println("Transmit NTP Request");
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
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0;
}

void sendNTPpacket(IPAddress& address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}



// Fuctions below are required for storage NVM control

void saveSchedule() {
  preferences.begin("schedule", false);                                  // Open namespace "schedule"
  preferences.putBytes("schedule", schedule, sizeof(schedule));  // Save schedule array
  preferences.end();                                                     // Close namespace
}

void loadSchedule() {
  preferences.begin("schedule", true);  // Open namespace "schedule" (read-only)

  if (preferences.isKey("schedule")) {
    // Load schedule array from flash memory
    preferences.getBytes("schedule", schedule, sizeof(schedule));
  }else {
    Serial.println("Failed to load from EEPROM");
  }
  

  preferences.end();  // Close namespace
}