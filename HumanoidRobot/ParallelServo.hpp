#ifndef PARALLELSERVO_H
#define PARALLELSERVO_H


#include <Adafruit_PWMServoDriver.h>
#include "PWMServo.hpp"

/* 
  Wraps PWMServo and changes the behaviour of move() so that a task 
  can execute movement in parallel (servoTask) 
*/
class ParallelServo : public PWMServo {
  public:
    QueueHandle_t queue;
    ParallelServo(Adafruit_PWMServoDriver& board, uint8_t channel_num, uint8_t forward_angle, uint8_t straight_angle);
    void move(uint8_t dest_angle);
    static void servoTask(void* parameter);
};


#endif