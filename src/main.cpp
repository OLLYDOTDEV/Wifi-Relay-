
#define relayPin 5  // GPIO5 connected to relay control pin

#define BUFLEN 10

int lastmode = 100;
int currentMode = 0;
int HeartBeat = 0;
int lastbeat = 0;
int statuscheck = 0;




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



  if (millis() - statuscheck >= 30000) {
    statuscheck = millis();
    Serial.println("Restarting Webserver...");
    server.stop();
    server.begin();
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

void pinMode_function(int pin, bool state) {

  if (relay_status != state) {
    updateUI();  // keeps UI updates faster
    relay_status = state;
    Serial.print("Pin State: ");
    Serial.println(state);
    delay(1000);  // delays required to prevent watchdog timer
    digitalWrite(pin, state);
    delay(4000);
  }
}
