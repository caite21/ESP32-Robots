/*
  Wireless Remote Control for RoboCar

  Description: Implements skid steering using dual joysticks. Converts 
  	joystick inputs into speed and direction commands and transmits 
  	them to a 2-wheel drive robot via ESP-NOW.
*/


#include <esp_now.h>
#include <WiFi.h>

// Receiver ESP32 MAC address
uint8_t recvAddr[] = {0x, 0x, 0x, 0x, 0x, 0x};

const uint8_t pinLeftX = 35;
const uint8_t pinRightX = 34;

typedef struct message_struct {
  uint8_t leftSpeed;
  bool leftHigh;
  bool leftLow;
  uint8_t rightSpeed;
  bool rightHigh;
  bool rightLow;
} message_struct;

message_struct myData;

esp_now_peer_info_t peerInfo;

uint16_t lxmin = 4096;
uint16_t lxmax = 0;
uint16_t rxmin = 4096;
uint16_t rxmax = 0;

uint8_t maxSpeed = 200;
uint8_t minSpeed = 70;


// ESP-NOW send callback 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nSent packet status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting:");

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  // Pair receiver ESP32
  memcpy(peerInfo.peer_addr, recvAddr, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error adding peer");
    return;
  }

  // Self calibrate
  uint16_t lx = 0;
  uint16_t rx = 0;

  for (uint16_t i = 0; i < 100; i++) {
    lx = analogRead(pinLeftX);
    rx = analogRead(pinRightX);

    if (lx > lxmax) {
      lxmax = lx;
    } else if (lx < lxmin) {
      lxmin = lx;
    }
    if (rx > rxmax) {
      rxmax = rx;
    } else if (rx < rxmin) {
      rxmin = rx;
    }
    delay(10);
  }
  Serial.printf("L rest: %u-%u \t\t R rest: %u-%u \n", lxmin, lxmax, rxmin, rxmax);
  delay(1000);
}

void loop() {
  uint16_t lx = analogRead(pinLeftX);
  uint16_t rx = analogRead(pinRightX);

  // Left wheel
  if (lx > lxmax) {
    // forwards
    myData.leftSpeed = map(lx, lxmax, 4095, minSpeed, maxSpeed);
    myData.leftHigh = 1;
    myData.leftLow = 0;
  }
  else if (lx < lxmin) {
    // backwards
    myData.leftSpeed = map(lx, 0, lxmin, maxSpeed, minSpeed);
    myData.leftHigh = 0;
    myData.leftLow = 1;
  } 
  else {
    // stop
    myData.leftSpeed = 0;
    myData.leftHigh = 0;
    myData.leftLow = 0;
  }

  // Right wheel
  if (rx > rxmax) {
    // forwards
    myData.rightSpeed = map(rx, rxmax, 4095, minSpeed, maxSpeed);
    myData.rightHigh = 1;
    myData.rightLow = 0;
  }
  else if (rx < rxmin) {
    // backwards
    myData.rightSpeed = map(rx, 0, rxmin, maxSpeed, minSpeed);
    myData.rightHigh = 0;
    myData.rightLow = 1;
  } 
  else {
    // stop
    myData.rightSpeed = 0;
    myData.rightHigh = 0;
    myData.rightLow = 0;
  }

  Serial.printf("left: %u \t right: %u \n", lx, rx);
  Serial.printf("L: %u %u %u\n", myData.leftHigh, myData.leftLow, myData.leftSpeed);
  Serial.printf("R: %u %u %u\n", myData.rightHigh, myData.rightLow, myData.rightSpeed);

  esp_err_t result = esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Send successful");
  } else {
    Serial.println("Error send failed");
  }

  delay(50);
}
