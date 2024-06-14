#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid = "testtest";
const char* password = "testtest";

ESP8266WebServer server(80);  // Create a webserver object on port 80

const int relayPin = 5;  // GPIO5 connected to relay control pin

bool timerType = false;    // false = Ton, true = Toff
unsigned long timerValue = 0;  // Timer value in milliseconds

void handleRoot() {
  String html = "<html><body><h1>Relay Control</h1>";

  html += "<br><a href='#' id='relayOnBtn'>Turn On</a> <a href='#' id='relayOffBtn'>Turn Off</a>";
  html += "<h2>Timer Settings</h2>";
  html += "<form id='timerForm' action='/'>";
  html += "<select name='timerType'>";
  html += "<option value='0'";
  if (!timerType) {
    html += " selected";
  }
  html += ">Ton</option>";
  html += "<option value='1'";
  if (timerType) {
    html += " selected";
  }
  html += ">Toff</option>";
  html += "</select>";
  html += " Timer: ";
  html += "<input type='number' name='timerValue' min='0' max='3600' value='" + String(timerValue / 1000) + "'> seconds";
  html += "<br><input type='submit' value='Set Timer'>";
  html += "</form>";
  html += "<p>Time remaining: <span id='timerStatus'>" + String(timerValue / 1000) + " seconds</span></p>";
  html += "<script src='https://code.jquery.com/jquery-3.6.0.min.js'></script>";
  html += "<script>";
  html += "$(document).ready(function() {";
  html += "  $('#timerForm').submit(function(event) {";
  html += "    event.preventDefault();";
  html += "    $.ajax({";
  html += "      type: 'POST',";
  html += "      url: $(this).attr('action'),";
  html += "      data: $(this).serialize(),";
  html += "      success: function(response) {";
  html += "        $('#timerStatus').text(response.message);";
  html += "      }";
  html += "    });";
  html += "  });";
  html += "  $('#relayOnBtn').click(function(event) {";
  html += "    event.preventDefault();";
  html += "    $.ajax({";
  html += "      type: 'POST',";
  html += "      url: '/',";
  html += "      data: { relay: 'on' },";
  html += "      success: function(response) {";
  html += "        $('#timerStatus').text(response.message);";
  html += "      }";
  html += "    });";
  html += "  });";
  html += "  $('#relayOffBtn').click(function(event) {";
  html += "    event.preventDefault();";
  html += "    $.ajax({";
  html += "      type: 'POST',";
  html += "      url: '/',";
  html += "      data: { relay: 'off' },";
  html += "      success: function(response) {";
  html += "        $('#timerStatus').text(response.message);";
  html += "      }";
  html += "    });";
  html += "  });";
  html += "});";
  html += "</script>";
  html += "</body></html>";

  if (server.method() == HTTP_POST) {
    // Handle AJAX request
    if (server.hasArg("relay")) {
      if (server.arg("relay") == "on") {
        digitalWrite(relayPin, HIGH);  // Turn relay on
        server.send(200, "application/json", "{\"message\":\"Relay turned on\"}");
      } else if (server.arg("relay") == "off") {
        digitalWrite(relayPin, LOW);  // Turn relay off
        server.send(200, "application/json", "{\"message\":\"Relay turned off\"}");
      }
    } else {
      // Handle timer settings
      if (server.hasArg("timerType")) {
        timerType = server.arg("timerType").toInt() == 1;
        timerValue = server.arg("timerValue").toInt() * 1000;  // Convert seconds to milliseconds
      } else {
        timerValue = 0;
      }
      server.send(200, "application/json", "{\"message\":\"Timer updated\"}");
    }
  } else {
    // Handle regular request
    server.send(200, "text/html", html);
  }
}

void setup() {
  pinMode(relayPin, OUTPUT);    // Set relay pin as output
  digitalWrite(relayPin, LOW);  // Initialize relay to off

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

  server.on("/", handleRoot);  // Associate handleRoot function to web server root URL
  server.begin();              // Start the web server
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  // Handle incoming HTTP requests

  // Check timer
  if (timerValue > 0) {
    if (!timerType) {
      digitalWrite(relayPin, HIGH);  // Turn relay on
    } else {
      digitalWrite(relayPin, LOW);  // Turn relay off
    }
    timerValue -= 1000;             // Decrement timer by 1 second
    delay(1000);
    Serial.print("Time remaining: ");
    Serial.println(timerValue);
    
    // Update timer status
    server.send(200, "application/json", "{\"message\":\"" + String(timerValue / 1000) + " seconds\"}");
    
    if (timerValue <= 0) {
      if (!timerType) {
        digitalWrite(relayPin, LOW);  // Turn relay off
        Serial.println("Ton Timer expired, relay turned off");
      } else {
        digitalWrite(relayPin, HIGH);  // Turn relay on
        Serial.println("Toff Timer expired, relay turned on");
      }
    }
  }
}
