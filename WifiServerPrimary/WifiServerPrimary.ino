
const char* ssid = "";
const char* password = "";



#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

const int localLedPin = 33;              // GPIO for local LED
const char* secondaryIPs[] = {           // IPs of secondary ESP32 boards
  "10.0.0.99",
  "10.0.0.101"
};

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void sendCommandToSecondary(const char* ip, const String& state, int ledId) {
  HTTPClient http;
  String url = String("http://") + ip + "/led?state=" + state + "&led=" + ledId;
  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response from " + String(ip) + ": " + response);
  } else {
    Serial.println("Failed to send command to " + String(ip));
  }
  http.end();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set the local LED pin as an output
  pinMode(localLedPin, OUTPUT);
  digitalWrite(localLedPin, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  // Route to serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
      "<!DOCTYPE html><html><body>"
      "<h1>ESP32 LED Control</h1>"
      "<h3>Local LED</h3>"
      "<button onclick=\"toggleLED('local', 'on')\">Turn ON</button>"
      "<button onclick=\"toggleLED('local', 'off')\">Turn OFF</button>"
      "<h3>Secondary LED 0</h3>"
      "<button onclick=\"toggleLED(0, 'on')\">Turn Secondary 0 ON</button>"
      "<button onclick=\"toggleLED(0, 'off')\">Turn Secondary 0 OFF</button>"
      "<h3>Secondary LED 1</h3>"
      "<button onclick=\"toggleLED(1, 'on')\">Turn Secondary 1 ON</button>"
      "<button onclick=\"toggleLED(1, 'off')\">Turn Secondary 1 OFF</button>"
      "<script>"
      "function toggleLED(id, state) {"
      "  fetch('/led?state=' + state + '&id=' + id);"
      "}"
      "</script>"
      "</body></html>");
  });

  // Route to handle LED control requests
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    String state = request->getParam("state")->value();
    String id = request->getParam("id")->value();

    if (id == "local") {
      if (state == "on") {
        digitalWrite(localLedPin, HIGH);
        Serial.println("Local LED turned ON");
      } else if (state == "off") {
        digitalWrite(localLedPin, LOW);
        Serial.println("Local LED turned OFF");
      }
    } else {
      int ledId = id.toInt();
      sendCommandToSecondary(secondaryIPs[ledId], state, ledId);
    }
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed here, the server runs asynchronously
}
