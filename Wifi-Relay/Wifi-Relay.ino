#include <ESP8266WiFi.h>
#include <EmbAJAX.h>

#include <NTPClient.h>
#include <WiFiUdp.h>


// Replace with your network credentials
const char* ssid = "testtest";
const char* password = "testtest";

#define relayPin 5  // GPIO5 connected to relay control pin

#define BUFLEN 10

int lastmode = 100;

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


// Delayed Timer
EmbAJAXMomentaryButton m_button_Timer_Set("Timer_Set", "Timer_Set");  // Timer set


EmbAJAXTextInput<BUFLEN> input_time_duration("input_time_duration");
char input_time_duration_b[BUFLEN];


const char* Timer_Array[] = { "10min", "30min", "1 hour", "3 hours", "6 hours" };
EmbAJAXOptionSelect<5> Dropdown_Time("Time_Array", Timer_Array);



EmbAJAXMutableSpan Relay_enabled_status("relay_status");


// Define a page (named "page") with our elements of interest, above, interspersed by some uninteresting
// static HTML. Note: MAKE_EmbAJAXPage is just a convenience macro around the EmbAJAXPage<>-class.
MAKE_EmbAJAXPage(page, "Wifi Relay Interface", "",
                 // Page Header

                 new EmbAJAXStatic("<h1>Wifi Relay - Overview </h1><p>Set the LED to: "),
                 &Radio_mode,
                 new EmbAJAXStatic("<hr><br>"),




                 // Override Display Control - On


                 // Override Display Control - Off

                 // Automatic Display Control

                 // Timer  Display Control

                 &Dropdown_Time,


                 &m_button_Timer_Set,


                 // Page Footer
                 new EmbAJAXStatic("<br><hr>"),

                 new EmbAJAXStatic("<Br >Connection status:"),
                 new EmbAJAXConnectionIndicator(),
                 &Relay_enabled_status)


  // posable options
  //
  // NTP server
  // TIMEZONE
  // daylight saving


  MAKE_EmbAJAXPage(Control_Configuration, "Wifi Relay - Configuration", "",
                   new EmbAJAXStatic("<h1>Configeration page</h1>"))

    void pinMode_function(int pin, bool state) {
  relay_status = state;
  digitalWrite(pin, state);
  Serial.print("Pin State: ");
    Serial.println(state);
}


void setup() {

  pinMode(relayPin, OUTPUT);    // Set relay pin as output
  pinMode_function(relayPin, LOW);  // Initialize relay to off


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

  // Create Pages

  driver.installPage(&page, "/", updateUI);
  driver.installPage(&Control_Configuration, "/config", updateUI);
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


Relay_enabled_status.setValue((relay_status == HIGH) ? "Relay Status: Enabled" : "Relay Status: Disabled");

}