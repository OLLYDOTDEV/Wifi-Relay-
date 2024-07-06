#include <ESP8266WiFi.h>
#include <EmbAJAX.h>
#include <EmbAJAXScriptedSpan.h>

//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>  // https://github.com/JChristensen/Timezone
#include <TimeLib.h>

#include <Preferences.h>

#include <ESP8266HTTPClient.h>

Preferences preferences;

// Replace with your network credentials
const char* ssid = "testtest";
const char* password = "testtest";

#define relayPin 5  // GPIO5 connected to relay control pin

#define BUFLEN 10

int lastmode = 100;
int currentMode = 0;
int HeartBeat = 0;
int lastbeat = 0;
int statuscheck = 0;

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

char date_str[32] = "";

int currentHour = 0;  // Variable to store the current hour in 24-hour format
int currentMinutes = 0;
// 24 hours array
bool schedule[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // Initialize with all hours OFF

EmbAJAXOutputDriverWebServerClass server(80);
EmbAJAXOutputDriver driver(&server);

long int starttime = 0;
int currenttime = 0;
int timepassed = 0;
int timerduration = 0;

bool relay_status = 0;

int WebErrorCount = 0;  // Variable to track the amount of web fails

// Radio selector for mode select
const char* modes[] = { "hidden", "Override - Off", "Override - On", "Automatic schedule", "Delayed Timer" };
EmbAJAXRadioGroup<5> Radio_mode("mode", modes);

EmbAJAXMomentaryButton m_button_Mode_submit("Submit_mode", "Submit mode");  // Timer set
EmbAJAXMutableSpan status_Mode_submit("mode_status");
char status_Mode_submit_b[BUFLEN];


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
  new EmbAJAXStatic("<center><p>Key: </p><p><b>&#x2713;</b> added to schedule</p><p> <b>x</b> removed from schedule </p></center><center><table id=\"schedule\" class=\"schedule\"><caption>Current schedule</caption><tbody><tr><td>"),
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
EmbAJAXMomentaryButton m_button_Timer_Set("Timer_Set", "Timer Set");  // Timer set


EmbAJAXTextInput<BUFLEN> input_time_duration("input_time_duration");
char input_time_duration_b[BUFLEN];


const char* Timer_Array[] = { "10 minutes ", "30 minutes ", "1 hour", "3 hours", "6 hours", "12 hours" };
EmbAJAXOptionSelect<6> Dropdown_Time("Time_Array", Timer_Array);



EmbAJAXMutableSpan Relay_enabled_status("relay_status");
EmbAJAXMutableSpan Current_time_status("Current_time");
char Current_timeM_status_b[BUFLEN];
char Current_timeH_status_b[BUFLEN];
EmbAJAXMutableSpan Remaining_Timer("Remaining_Timer_HH_MM");




// Define a page (named "page") with our elements of interest, above, interspersed by some uninteresting
// static HTML. Note: MAKE_EmbAJAXPage is just a convenience macro around the EmbAJAXPage<>-class.
MAKE_EmbAJAXPage(
  page, "Wifi Relay Interface",
  "<meta charset=\"UTF-8\">\n"
  "   <style>\n"
  "   @charset \"utf-8\";\n"
  "div.unselectable {\n"
  "  -webkit-touch-callout: none;\n"
  " -webkit-user-select: none;\n"
  " -khtml-user-select: none;\n"
  " -moz-user-select: none;\n"
  " -ms-user-select: none;\n"
  " user-select: none;\n"
  "}\n"
  "*{font-family:Fantasy; padding:0.1em;}\n"
  "td {text-align: center;}\n"
  "table.schedule{font-size: 1em}"
  "table.footer {  border-collapse: collapse;  width: 100%;}\n"
  "#Time_Array{font-size: 1.5em;padding:0.1em;}\n"
  "hr {padding:0px;}"


  "@charset \"utf-8\";\n"

  "/* General styles */\n"
  "* {\n"
  "    font-family: Arial, sans-serif;\n"
  "}\n"

  "h1 {\n"
  "    font-size: 2.6em;\n" /* Original size: 1.5em, now 2em */
  "    text-align: center;\n"
  "}\n"

  "input[type=radio], input[type=checkbox] {\n"
  "    width: 2.2em;\n" /* Increased width */
  "    height: 2em;\n"  /* Increased height */
  "}\n"

  "td {\n"
  "    text-align: center;\n"
  "    padding: 5px;\n"
  "    font-size: 1.2em;\n" /* Slightly increased font size */
  "}\n"

  "/* Styles for tables */\n"
  "table {\n"
  "    width: 100%;\n"
  "    margin: 10px 0;\n"
  "    font-size: 1.2em;\n" /* Slightly increased font size */
  "}\n"

  "table.schedule {\n"
  "    font-size: 0.9em;\n" /* Original size: 0.9em, now 1.1em */
  "}\n"

  "table.schedule td {\n"
  "   vertical-align: middle;\n"
  "}\n"

  "table.schedule td span:not(label) {\n"
  " font-size: 1.8em;\n" /* Adjust this value to your desired font size */
  "}\n"
  "table.schedule td label {\n"
  "  font-size: 0.8em;" /* Example: keeping labels at a smaller size */
  "}\n"
  "table.footer {\n"
  "    width: 100%;\n"
  "    border-collapse: collapse;\n"
  "    font-size: 1.2em;\n" /* Slightly increased font size */
  "}\n"

  "label {\n"
  "    display: inline-block;\n"
  "    margin-left: 5px;\n"
  "    font-size: 1.5em;\n" /* Slightly increased font size */
  "}\n"


  "/* Styles for smaller screens */\n"
  "@media (max-width: 600px) {\n"
  "    body {\n"
  "        padding: 10px;\n"
  "        font-size: 1.1em;\n" /* Original size: 0.9em, now 1.1em */
  "    }\n"
  "    \n"
  "    h1 {\n"
  "        font-size: 1.6em;\n" /* Original size: 1.2em, now 1.6em */
  "    }\n"
  "    \n"
  "    .radio, .checkbox {\n"
  "        padding:0.2em;"
  "        display: block;\n"
  "        margin-bottom: 10px;\n"
  "        font-size: 1.2em;\n" /* Slightly increased font size */
  "    }\n"
  "    \n"
  "    table.schedule tbody tr td {\n"
  "        display: block;\n"
  "        width: 100%;\n"
  "        margin-bottom: 10px;\n"
  "        font-size: 1.1em;\n" /* Slightly increased font size */
  "    }\n"
  "    \n"
  "    table.schedule tbody tr td span {\n"
  "        display: inline-block;\n"
  "        margin-bottom: 5px;\n"
  "        font-size: 1.2em;\n" /* Slightly increased font size */
  "    }\n"
  "    \n"
  "    table.schedule tbody tr td label {\n"
  "        width: 100%;\n"
  "        font-size: 1.2em;\n" /* Slightly increased font size */
  "        padding-top:0.5em;\n"
  "    }\n"
  "    table.schedule span {\n"
  "        padding-top:0.5em;\n"
  "        padding-bottom:1em;\n"
  "    }\n"
  "    \n"
  "    center {\n"
  "        display: block;\n"
  "    }\n"
  "    \n"
  "    center > * {\n"
  "        display: block;\n"
  "        margin: 10px auto;\n"
  "        font-size: 1.2em;\n" /* Slightly increased font size */
  "    }\n"
  "    \n"
  "    button {\n"
  "        width: 100%;\n"
  "        padding: 10px;\n"
  "        font-size: 1.2em;\n" /* Original size: 1em, now 1.2em */
  "        padding: 0.5em;\n"
  "    }\n"
  "    \n"
  "    .EmbAJAXStatus span {\n"
  "        display: block;\n"
  "        margin: 5px 0;\n"
  "        font-size: 1.2em;\n" /* Slightly increased font size */
  "    }\n"
  "    \n"
  "    table.footer td {\n"
  "        display: block;\n"
  "        width: 100%;\n"
  "        text-align: left;\n"
  "        font-size: 1.2em;\n" /* Slightly increased font size */
  "    }\n"
  "}\n"

  "/* New Styles */\n"
  "p {\n"
  "    font-size: 1.2em;\n" /* Increased size for text including "Control mode:" */
  "}\n"

  ".radio label, .checkbox label {\n"
  "    font-size: 1.4em;\n"       /* Slightly increased size for radio/checkbox text */
  "    vertical-align: middle;\n" /* Vertically centered text */
  "    padding-top: 0.5em;\n"
  "    padding-bottom: 0.73em;\n"
  "}\n"

  "button {\n"
  "    font-size: 1.5em;\n" /* Adjusted to be similar to the schedule text size */
  "}\n"

  "table.footer td {\n"
  "    display: table-cell;\n" /* Display cells side by side */
  "    width: auto;\n"
  "}\n"

  "table.footer tr {\n"
  "    display: table-row;\n"
  "}\n"

  "/* Additional styles to align span with checkbox text */\n"
  "input[type=checkbox] + label, input[type=radio] + label {\n"
  "    display: inline-flex;\n"
  "    align-items: center;\n"
  "}\n"

  "input[type=checkbox] + label span, input[type=radio] + label span {\n"
  "    font-size: 1.4em;\n"       /* Match the size of the checkbox text */
  "    vertical-align: middle;\n" /* Vertically center the span */
  "}\n"




  "</style>",
  // Page Header



  new EmbAJAXStatic("<h1>Wifi Relay - Overview </h1><center>"),
  &Radio_mode,
  new EmbAJAXStatic("<br>"),
  &m_button_Mode_submit,
  &status_Mode_submit,
  new EmbAJAXStatic("</center><hr><br>"),

  // Override Display Control - On

  // Override Display Control - Off

  // Automatic Display Control

  &hours_contents,


  // Timer  Display Control
  new EmbAJAXStatic("<center>"),
  &Dropdown_Time,


  &m_button_Timer_Set,
  new EmbAJAXStatic("<br><h2>"),
  &Remaining_Timer,
  new EmbAJAXStatic("</h2></center>"),
  // Page Footer
  new EmbAJAXStatic("<br><hr><center><table><tr><td>Connection status:</td><td text-align=\"Right\">"),
  new EmbAJAXConnectionIndicator(),
  new EmbAJAXStatic("</td><td>"),
  &Relay_enabled_status,
  new EmbAJAXStatic("</td><td text-align=\"Right\"><span>    Timestamp: </span></td><td text-align=\"left\">"),
  &Current_time_status,
  new EmbAJAXStatic("</td></tr></center></table>"),
  new EmbAJAXStatic(
    "    <script>"
    // Hide the element with ID 'mode0'
    "    document.getElementById('mode0').style.display = 'none';"
    // Hide the corresponding label
    "    var label = document.querySelector('label[for=\"mode0\"]');"
    "    if (label) {"
    "    label.style.display = 'none';"
    "    }</script>"));






void pinMode_function(int pin, bool state) {

  if (relay_status != state) {
    updateUI();  // keeps UI updates faster
    relay_status = state;
    Serial.print("Pin State: ");
    Serial.println(state);
    delay(1000);  // delays required to provent watchdog timer
    digitalWrite(pin, state);
    delay(4000);
  }
}


void setup() {
  pinMode(relayPin, OUTPUT);        // Set relay pin as output
  pinMode_function(relayPin, LOW);  // Initialize relay to off

  // Setup Network
  Serial.begin(115200);
  delay(1500);
  Serial.println("");
  WiFi.begin(ssid, password);  // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("Schedule loaded to EEPROM");

  loadMode();
  loadSchedule();  // load schedule array from NVM
  // ClearState();  // Keep Comneted unless required [will clear set NVM keys]

  Serial.println("WiFi connected");
  Serial.println("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address to Serial Monitor
  Serial.println("WiFi status: " + String(WiFi.status()));
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("Waiting for sync");
  setSyncProvider(getNtpTime);
  //setSyncInterval(86400);
  setSyncInterval(60);

  lastbeat = millis();

  // Create Pages
  delay(1000);
  driver.installPage(&page, "/", updateUI);
  server.begin();
  Serial.println("Webserver started");
  updateUI();  // init displays
  WebServerTest();
}





void updateUI() {
  // Enabled / disable the slider. Note that you could simply do this inside the loop. However,
  // placing it here makes the client UI more responsive (try it).
  // timeractive.setEnabled(timer!= 0);

  // Override Display Control - On


  // Override Display Control - Off

  // Automatic Display Control

  hours_contents.setVisible(currentMode == 3);


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


  Dropdown_Time.setVisible(currentMode == 4);
  input_time_duration.setVisible(currentMode == 4);
  m_button_Timer_Set.setVisible(currentMode == 4);
  Remaining_Timer.setVisible(currentMode == 4);
  // Save timer variables
  if (m_button_Timer_Set.status() == EmbAJAXMomentaryButton::Pressed) {

    Serial.println("Timer Data Submitted");
    switch (Dropdown_Time.selectedOption()) {  // duration in minutes * converation factor to ms
      case 0:
        timerduration = 10 * 60000;
        break;
      case 1:
        timerduration = 30 * 60000;
        break;
      case 2:
        timerduration = 60 * 60000;
        break;
      case 3:
        timerduration = 180 * 60000;
        break;
      case 4:
        timerduration = 360 * 60000;
        break;
      case 5:
        timerduration = 720 * 60000;  // Not used yet
        break;
      case 6:
        timerduration = 0 * 60000;  // Not used yet
        break;
      default:
        Serial.println("Error Invalid Timer duration ");
        break;
    }
    Serial.print("Selected Timer duration: ");
    Serial.println(timerduration);

    starttime = millis();  // Save the time when the button was pushed
  }





  if (m_button_Mode_submit.status() == EmbAJAXMomentaryButton::Pressed) {
    currentMode = Radio_mode.selectedOption();
    Serial.println("Mode updated");
    saveMode();

    status_Mode_submit.setValue(itoa(currentMode, status_Mode_submit_b, 10));

    selectMode();
  }

  Serial.println("updateUI Finished");
}






void loop() {

  unsigned long seconds_remaining = timerduration / 1000;
  unsigned long minutes_remaining = seconds_remaining / 60;
  unsigned long hours_remaining = minutes_remaining / 60;
  seconds_remaining = seconds_remaining % 60;
  minutes_remaining = minutes_remaining % 60;
  char formattedTime_remaining[9] = " ";  // HH:MM:SS\0
  sprintf(formattedTime_remaining, "%02lu:%02lu:%02lu", hours_remaining, minutes_remaining, seconds_remaining);





  // handle network. loopHook() simply calls server.handleClient(), in most but not all server implementations.
  driver.loopHook();


  // Check the webserver is still working and also helps to save CPU cycles
  //if (currentMinutes == 30) {



  if (millis() - statuscheck >= 60000) {
    statuscheck = millis();
    WebServerTest();  // Function Broken currently
    checkWiFi();
  }



  // }

  // if (WebErrorCount == 2 and millis() >= 60000) {

  //   Serial.println("Web Error");
  //   ESP.restart();
  // }





  if (currentMode == 0 || currentMode == 1) {  // Override - OFF

    if (lastmode != currentMode) {  // execute code only once  on mode switch
      lastmode = currentMode;
      Serial.print("Selected mode: ");
      Serial.println(currentMode);
      pinMode_function(relayPin, LOW);
    }



  } else if (currentMode == 2) {    // Override - ON
    if (lastmode != currentMode) {  // execute code only once on mode switch
      lastmode = currentMode;
      Serial.print("Selected mode: ");
      Serial.println(currentMode);
      pinMode_function(relayPin, HIGH);
    }
  }



  else if (currentMode == 3) {  // Automatic schedule

    for (int i = 0; i < 24; i++) {  // Update UI for current schedule
      Set_schedule[i].setValue(schedule[i] ? "✔️" : "❌");
    }


    timerduration = 0;  // reset timer
    if (schedule[currentHour]) {
      pinMode_function(relayPin, HIGH);  // Turn relay ON
    } else {
      pinMode_function(relayPin, LOW);  // Turn relay OFF
    }
  }


  else if (currentMode == 4) {      // Override - Delayed Timer
    if (lastmode != currentMode) {  // execute code only once on mode switch
      lastmode = currentMode;
      Serial.print("Selected mode: ");
      Serial.println(currentMode);
      pinMode_function(relayPin, LOW);
      timerduration = 0;  // reset timer
    }

    currenttime = millis();
    timepassed = currenttime - starttime;




    Remaining_Timer.setValue(formattedTime_remaining);

    //  Remaining_Timer.setValue((const char*) formattedTime_remaining);




    //Remaining_Timer.setValue(formattedTime_remaining);
    //Remaining_Timer.setValue(strncpy(formattedTime_remaining,formattedTime_remaining,BUFLEN),true);






    if (timepassed >= 1000 && timerduration != 0) {
      starttime = millis();
      Serial.print("Remaining duration: ");
      Serial.println(formattedTime_remaining);


      if (timerduration < 1000) {

        timerduration = 0;  // provent int overflow
      } else {
        timerduration = timerduration - 1000;  // decress timer by 1 second
      }
    }

    if (timerduration == 0) {
      pinMode_function(relayPin, LOW);
      Remaining_Timer.setValue("No Timer");

    } else {
      pinMode_function(relayPin, HIGH);
    }
  }




  timestring();


  Relay_enabled_status.setValue((relay_status == HIGH) ? "<span style=\"background-color:rgba(0,128,0,1); \">Relay Status: Enabled</span>" : "<span style=\"background-color:rgba(255,0,0,1); \">Relay Status: Disabled</span>", true);









  // Calling fuctions to handles webserver and Memory issues and other soft locks

  if (currentHour == 11 && currentMinutes == 59) {  // reboot at 11:59 am
    pinMode_function(relayPin, LOW);
    Serial.println("Automatic Reboot in 60 seconds");
    delay(1200000);  // ensure wont reboot twice in the same minute
    ESP.restart();   //
  }





  if (millis() - HeartBeat >= 10000) {
    HeartBeat = millis();
    Serial.print("HeartBeat: ");
    Serial.println(date_str);
  }
  delay(1);
}


void selectMode() {


  switch (currentMode) {
    case 1:
      status_Mode_submit.setValue("<br><h4>Current Mode: Override - Off</h4>", true);
      break;
    case 2:
      status_Mode_submit.setValue("<br><h4>Current Mode: Override - On</h4>", true);
      break;
    case 3:
      status_Mode_submit.setValue("<br><h4>Current Mode: Automatic schedule</h4>", true);
      break;
    case 4:
      status_Mode_submit.setValue("<br><h4>Current Mode: Delayed Timer</h4>", true);
      break;
  }
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


// Fuctions below are required for storage NVM control

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


// ensure
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

// ensures if the webserver stays operational

// Ensure the webserver stays operational
void WebServerTest() {
  driver.loopHook();
  Serial.println("Testing webserver status");

  WiFiClient client;  // Create a WiFiClient object
  HTTPClient http;
  String serverAddress = "http://" + WiFi.localIP().toString() + "/";

  http.begin(client, serverAddress);  // Use the new method with WiFiClient
  int httpCode = http.GET();
  http.end();

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("Webserver Status: ERROR");
    Serial.print("Unexpected HTTP code: ");
    Serial.println(httpCode);
    WebErrorCount = WebErrorCount + 1;
    server.stop();
    server.begin();
    delay(100);  // Give the server some time to start
    Serial.println("Webserver restarted");

  } else {
    Serial.println("Webserver Status: OK");
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
