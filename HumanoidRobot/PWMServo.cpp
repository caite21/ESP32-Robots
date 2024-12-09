#include <Adafruit_PWMServoDriver.h>
#include "PWMServo.hpp"


uint16_t SERVOMIN = 125;
uint16_t SERVOMAX = 625;

PWMServo::PWMServo(Adafruit_PWMServoDriver& board, uint8_t channel_num, uint8_t forward_angle, uint8_t straight_angle)
  : board(board), channel_num(channel_num), forward_angle(forward_angle), straight_angle(straight_angle) {
  // default speed delay
  this->speed_delay = 3; 
  // Starting angle is straight
  this->angle = straight_angle;
  setPWM(angleToPulse(this->angle));
}

uint16_t PWMServo::angleToPulse(uint8_t angle) {
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void PWMServo::setPWM(uint16_t pulse) {
  if (pulse >= SERVOMIN && pulse <= SERVOMAX) {
    this->board.setPWM(this->channel_num, 0, pulse);
  } else {
    Serial.printf("Error: Pulse %u out of range %u to %u\n", pulse, SERVOMIN, SERVOMAX);
  }
}

uint8_t PWMServo::rotationToAngle(uint8_t rot_percent) {
  return map(rot_percent, 0, 100, this->forward_angle, 180-this->forward_angle);
}

/* 0% is fully forward, 100% is fully backward */
void PWMServo::rotate(uint8_t rot_percent) { 
  move(rotationToAngle(rot_percent));
}

void PWMServo::straighten() {
  move(this->straight_angle);
}
