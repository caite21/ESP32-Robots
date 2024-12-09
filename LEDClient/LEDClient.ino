/* Client executes LED commands from the web interface */

// Enter Wi-Fi credentials 
const char* ssid = "";
const char* password = "";
// Give this client a name
const char* MY_NAME = "";

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>


const char* serverIP = "10.0.0.101"; // Primary ESP32 IP
const int serverPort = 80;

WebSocketsClient webSocket;
const int ledPin = 33; // GPIO for LED

/* Commands are in JSON */
void handleWebSocketMessage(const char* payload) {
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  String name = doc["name"];
  String command = doc["command"];

  if (name == MY_NAME) {
    String command = doc["command"];
    if (command == "set_led") {
      String state = doc["state"];
      if (state == "on") {
        digitalWrite(ledPin, HIGH);
      } else if (state == "off") {
        digitalWrite(ledPin, LOW);
      }
    }
  }
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      Serial.println("Received message!");
      handleWebSocketMessage((const char*)payload);
      break;
    case WStype_BIN:
      Serial.println("Received binary message!");
      handleWebSocketMessage((const char*)payload);
      break;
    case WStype_CONNECTED: {
      Serial.println("WebSocket connected!");

      // Notify the web interface that this client is connected
      DynamicJsonDocument message(128);
      String jsonString;
      message["command"] = "set_name";
      message["name"] = MY_NAME;
      serializeJson(message, jsonString);
      webSocket.sendTXT(jsonString);
      break;
    }
    case WStype_DISCONNECTED:
      Serial.println("WebSocket disconnected!");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Set up WebSocket
  webSocket.begin(serverIP, serverPort, "/ws");
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}
