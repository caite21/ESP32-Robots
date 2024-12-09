#include "ParallelServo.hpp"
#include <Adafruit_PWMServoDriver.h>

ParallelServo::ParallelServo(Adafruit_PWMServoDriver& board, uint8_t channel_num, uint8_t forward_angle, uint8_t straight_angle) 
  : PWMServo(board, channel_num, forward_angle, straight_angle) {
  queue = xQueueCreate(1, sizeof(uint8_t));
}

void ParallelServo::move(uint8_t dest_angle) {
  xQueueSend(this->queue, &dest_angle, portMAX_DELAY);
}

void ParallelServo::servoTask(void* parameter) {
  ParallelServo* this_servo = static_cast<ParallelServo*>(parameter);
  uint8_t dest_angle;
  for (;;) {
    Serial.printf("Task %s is running. Free stack: %u words\n", pcTaskGetName(NULL),  uxTaskGetStackHighWaterMark(NULL));
    // Block until move() command sends a new destination
    xQueueReceive(this_servo->queue, &dest_angle, portMAX_DELAY);

    while(this_servo->angle != dest_angle) {
      // Step through movement
      if (this_servo->angle < dest_angle) {
        this_servo->angle++;
        this_servo->setPWM(this_servo->angleToPulse(this_servo->angle));
      } else {
        this_servo->angle--;
        this_servo->setPWM(this_servo->angleToPulse(this_servo->angle));
      }
      // Slows down movement
      vTaskDelay(pdMS_TO_TICKS(this_servo->speed_delay));
    }
  }
}