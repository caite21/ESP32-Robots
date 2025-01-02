/*
  Remote-Controlled Robo Car

  Description: Handles wireless communication via ESP-NOW to 
    receive speed and direction data from a remote control. 
    Controls the motor drivers to execute skid-steering commands 
    for a 2-wheel drive robot.
*/


#include <esp_now.h>
#include <WiFi.h>

// Left wheel: ENA, IN1, IN2
uint8_t const lEn = 23;
uint8_t const lH = 19;
uint8_t const lL = 18;
// Right wheel: IN3, IN4, ENB
uint8_t const rL = 5;
uint8_t const rH = 17;
uint8_t const rEn = 16;

const uint16_t PWMFreq = 1000;
const uint16_t PWMResolution = 8;
const uint16_t leftPWMChannel = 4;
const uint16_t rightPWMChannel = 5;

const uint16_t timeout = 750;
uint16_t recvTime = 0;


typedef struct message_struct {
  uint8_t leftSpeed;
  bool leftHigh;
  bool leftLow;
  uint8_t rightSpeed;
  bool rightHigh;
  bool rightLow;
} message_struct;

message_struct myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len != 0) {
    memcpy(&myData, incomingData, sizeof(myData));

    // Left
    ledcWrite(lEn, myData.leftSpeed);
    digitalWrite(lH, myData.leftHigh);
    digitalWrite(lL, myData.leftLow);

    // Right
    ledcWrite(rEn, myData.rightSpeed);
    digitalWrite(rH, myData.rightHigh);
    digitalWrite(rL, myData.rightLow);
  }

}

void setup() {
  pinMode(lEn, OUTPUT);
  pinMode(rEn, OUTPUT);
  pinMode(lH, OUTPUT);
  pinMode(lL, OUTPUT);
  pinMode(rH, OUTPUT);
  pinMode(rL, OUTPUT);

  ledcAttachChannel(lEn, PWMFreq, PWMResolution, leftPWMChannel);
  ledcAttachChannel(rEn, PWMFreq, PWMResolution, rightPWMChannel);

  digitalWrite(lH, 0);
  digitalWrite(lL, 0);
  digitalWrite(rH, 0);
  digitalWrite(rL, 0);

  Serial.begin(115200);
  Serial.println("Starting:");

  delay(500);

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  esp_now
}

void loop() {
  
}
