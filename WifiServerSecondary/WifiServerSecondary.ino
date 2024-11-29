
const char* ssid = "";
const char* password = "";



#include <WiFi.h>
#include <ESPAsyncWebServer.h>


const int ledPin = 33;                   // GPIO for the local LED

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set the LED pin as an output
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

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

  // Route to handle LED control
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    String state = request->getParam("state")->value();
    if (state == "on") {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED turned ON");
      request->send(200, "text/plain", "LED is ON");
    } else if (state == "off") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED turned OFF");
      request->send(200, "text/plain", "LED is OFF");
    }
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed here, the server runs asynchronously
}
