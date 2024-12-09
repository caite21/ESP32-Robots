/* 
  Serves the web page and communicates with clients via WebSockets. 
  
  Users can control multiple ESP32 clients by clicking buttons on the web interface.
  Web interface clients send JSON messages of commands to the server. The server forwards 
  the message to the correct ESP32 client. The client receives the message and executes 
  the command. 
*/

// Enter Wi-Fi credentials
const char* ssid = "";
const char* password = "";


#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

AsyncWebSocket ws("/ws");
AsyncWebServer server(80);
std::map<String, uint8_t> deviceMap;

String getDeviceName(int id) {
  for (auto d : deviceMap) {
    if (d.second == id) {
      return d.first;
    } else {
      return "ID does not exist.";
    }
  }
}

void sendToInterface(String alert) {
  // TODO: can only send alerts to first web client, need to track all web clients
  Serial.println(alert);
  if (deviceMap.count("web") != 0) {
    ws.text(deviceMap["web"], alert);
  }
}

// Messages are in JSON
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint8_t client_id) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    DynamicJsonDocument doc(512);
    deserializeJson(doc, data);

    String name = doc["name"];
    String command = doc["command"];
    String state = doc["state"];
    uint8_t value = doc["value"];

    // Notify web interface that a new device is connected and remember its name
    if (command == "set_name") {
      sendToInterface("Device connected: " + name);
      deviceMap[name] = client_id;
      return;
    }

    if (deviceMap.count(name) == 0) {
      sendToInterface("Command denied: Device not connected.");
    }
    else {
      // forward message
      ws.binary(deviceMap[name], data, len);
    }
  }
}

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: {
      Serial.printf("Client %u connected\n", client->id());
      break;
    }
    case WS_EVT_DISCONNECT: {
      Serial.printf("Client %u disconnected\n", client->id());
      String name_disconnected = getDeviceName(client->id());
      sendToInterface("Device disconnected: " + name_disconnected);
      deviceMap.erase(name_disconnected);
      break;
    }
    case WS_EVT_DATA: {
      Serial.println("Server received message!");
      handleWebSocketMessage(arg, data, len, client->id());
      break;
    }
  }
}

// HTML for the web interface
const char* webpage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f9;
      color: #333;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    h1 {
      background-color: #4b8f38;
      color: white;
      width: 100%;
      padding: 20px;
      text-align: center;
      margin: 0;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
    }
    .device-card {
      background: white;
      border-radius: 10px;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
      margin: 20px;
      padding: 20px;
      max-width: 300px;
      min-width: 150px;
      text-align: center;
      flex: 1;
    }
    button {
      background-color: #4b8f38;
      color: white;
      border: none;
      padding: 10px 15px;
      margin: 5px;
      border-radius: 5px;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    button:hover {
      background-color: #326625;
    }
    button:active {
      transform: translateY(1px);
    }
    .slider {
      width: 100%;
      height: 25px;
    }
    .log-container {
      background: #fff;
      border-radius: 10px;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
      max-width: 600px;
      margin: 20px;
      padding: 20px;
      width: 90%;
    }
    #log_box {
      font-family: monospace;
      white-space: pre-wrap;
      background: #f4f4f9;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 5px;
      height: 200px;
      overflow-y: auto;
    }
    .devices {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
    }
    @media (max-width: 600px) {
      .device-card {
        max-width: 100%;
      }
    }
  </style>
  <script>
    const ws = new WebSocket(`ws://${location.hostname}/ws`);

    ws.onopen = function(event) {
      ws.send(JSON.stringify({ name: "web", command: "set_name" }));
    };

    ws.onmessage = function(event) {
      const logBox = document.getElementById('log_box');
      logBox.innerText += event.data + "\n";
      logBox.scrollTop = logBox.scrollHeight; // Auto-scroll to latest message
    };

    function displaySliderValue(value) {
      document.getElementById("slider_value").innerHTML = value;
    }

    function setLED(name, state) {
      ws.send(JSON.stringify({ name: name, command: "set_led", state: state }));
    }

    function setServo(name, servo_num) {
      var angle = document.getElementById("slider").value;
      ws.send(JSON.stringify({ name: name, command: "set_servo", servo_num: servo_num, value: angle }));
    }
  </script>
</head>
<body>
  <h1>ESP32 Control Panel</h1>
  <div class="devices">
    <div class="device-card">
      <h3>Device A</h3>
      <button onclick="setLED('deviceA', 'on')">Turn ON</button>
      <button onclick="setLED('deviceA', 'off')">Turn OFF</button>
    </div>
    <div class="device-card">
      <h3>Device B</h3>
      <button onclick="setLED('deviceB', 'on')">Turn ON</button>
      <button onclick="setLED('deviceB', 'off')">Turn OFF</button>
    </div>
    <div class="device-card">
      <h3>Device C</h3>
      <button onclick="setServo('deviceC', 0)">Set Servo 0</button>
      <button onclick="setServo('deviceC', 1)">Set Servo 1</button>
      <button onclick="setServo('deviceC', 2)">Set Servo 2</button>
      <button onclick="setServo('deviceC', 3)">Set Servo 3</button>
      <div class="slidecontainer">
        <input type="range" min="0" max="180" step=10 class="slider" id="slider" oninput="displaySliderValue(this.value)">
        <p>Angle: <span id="slider_value">X</span></p>
      </div>
    </div>

  </div>
  <div class="log-container">
    <h3>Log</h3>
    <div id="log_box"></div>
  </div>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println(WiFi.localIP());

  // Serve the web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", webpage);
  });

  // Setup WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Start the server
  server.begin();
}

void loop() {
  
}
