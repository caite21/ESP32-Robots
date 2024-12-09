
#include "ParallelServoGroup.hpp"
#include "ParallelServo.hpp"

ParallelServoGroup::ParallelServoGroup() {
  this->speed_delay = 3;
}

void ParallelServoGroup::add(ParallelServo* servo) {
  servos.push_back(servo);
  // Don't set new distination yet
  destAngles.push_back(servo->angle);
}

/* Move all managed servos at once 
  TODO: use EventGroup instead of queues because this is starving the cpu 
*/
void ParallelServoGroup::multipleServosTask(void* parameter) {
  ParallelServoGroup* this_group = static_cast<ParallelServoGroup*>(parameter);

  ParallelServo* servo;
  uint8_t dest_angle;
  uint8_t recv_dest_angle;
  for (;;) {
    Serial.printf("Task %s is running. Free stack: %u words\n", pcTaskGetName(NULL),  uxTaskGetStackHighWaterMark(NULL));
    for (uint8_t i = 0; i < this_group->servos.size(); i++) {
      servo = this_group->servos[i];
      dest_angle = this_group->destAngles[i];

      if(servo->angle == dest_angle) {
        // Reached destination so check for a new destination
        if (xQueueReceive(servo->queue, &recv_dest_angle, 0)) {
          // Update destination only if move() command sends a new destination
          this_group->destAngles[i] = recv_dest_angle;
          Serial.printf("Received val %u\n", recv_dest_angle);
        }
      } else {
        // Step through movement
        if (servo->angle < dest_angle) {
          servo->angle++;
          servo->setPWM(servo->angleToPulse(servo->angle));
        } else {
          servo->angle--;
          servo->setPWM(servo->angleToPulse(servo->angle));
        }
      }
    }

    // Slow down movement for all at the same time
    vTaskDelay(pdMS_TO_TICKS(this_group->speed_delay));
  }
}