#ifndef PWMSERVO_H
#define PWMSERVO_H


#include <Adafruit_PWMServoDriver.h>

/* Simplifies controlling a Servo using the 16-channel Adafruit PWMServoDriver board */
class PWMServo {
  protected:
    Adafruit_PWMServoDriver& board;
    uint8_t channel_num;  // 0-15 channels
    uint8_t straight_angle;  // 0-180 degrees
    uint8_t forward_angle;  // 0 or 180
    uint8_t speed_delay;  // 0-255 ms 

  public:
    uint8_t angle;  // 0-180 degrees
    PWMServo(Adafruit_PWMServoDriver& board, uint8_t channel_num, uint8_t forward_angle, uint8_t straight_angle);
    static uint16_t angleToPulse(uint8_t angle);
    uint8_t rotationToAngle(uint8_t rot_percent);
    void rotate(uint8_t rot_percent);
    void straighten();
    void setPWM(uint16_t pulse);

    virtual void move(uint8_t dest_angle) {
      while(this->angle != dest_angle) {
        // Step through movement
        if (this->angle < dest_angle) {
          this->angle++;
          setPWM(angleToPulse(this->angle));
        } else {
          this->angle--;
          setPWM(angleToPulse(this->angle));
        }
        // Slows down movement
        delay(this->speed_delay);
      }
    }
};


#endif