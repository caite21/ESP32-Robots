#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN  125
#define SERVOMAX  625


// Servo IDs
#define HIP_RIGHT   0
#define KNEE_LEFT   1
#define KNEE_RIGHT  2
#define HIP_LEFT    3

// Starting and ending servo positions
int s0_straight = 90; // s0 is right hip
int s1_straight = 20; // s1 is left knee
int s2_straight = 160; // s2 is right knee
int s3_straight = 90; // s3 is left hip

// Define queues for each servo
QueueHandle_t servoQueue[4];

int angleToPulse(int ang);
int moveServo(int servo_num, int curr_angle, int dest_angle, int speed_delay);
void moveServoTask(void *parameter);


void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
  board1.begin();
  board1.setPWMFreq(60);
  delay(10);

  // Create  1 item queues for each servo
  for (int i = 0; i < 4; i++) {
    servoQueue[i] = xQueueCreate(1, sizeof(int));
  }

  // Create FreeRTOS tasks for each servo
  xTaskCreate(moveServoTask, "MoveServo_0", 2048, (void *)HIP_RIGHT, 1, NULL);
  xTaskCreate(moveServoTask, "MoveServo_1", 2048, (void *)KNEE_LEFT, 1, NULL);
  xTaskCreate(moveServoTask, "MoveServo_2", 2048, (void *)KNEE_RIGHT, 1, NULL);
  xTaskCreate(moveServoTask, "MoveServo_3", 2048, (void *)HIP_LEFT, 1, NULL);

  Serial.println("Tasks Created");
  delay(10);
}

void loop() {
  int new_angle;

  // Bend legs
  new_angle = 45;
  xQueueSend(servoQueue[HIP_RIGHT], &new_angle, portMAX_DELAY);
  new_angle = 80;
  xQueueSend(servoQueue[KNEE_RIGHT], &new_angle, portMAX_DELAY);
  new_angle = 60;
  xQueueSend(servoQueue[HIP_LEFT], &new_angle, portMAX_DELAY);
  new_angle = 100;
  xQueueSend(servoQueue[KNEE_LEFT], &new_angle, portMAX_DELAY);

  delay(2000);

  // Straighten legs
  new_angle = 90;
  xQueueSend(servoQueue[HIP_RIGHT], &new_angle, portMAX_DELAY);
  new_angle = 20;
  xQueueSend(servoQueue[KNEE_LEFT], &new_angle, portMAX_DELAY);  
  new_angle = 160;
  xQueueSend(servoQueue[KNEE_RIGHT], &new_angle, portMAX_DELAY);
  new_angle = 90;
  xQueueSend(servoQueue[HIP_LEFT], &new_angle, portMAX_DELAY);

  delay(5000); 
}


int angleToPulse(int ang) {
  return map(ang, 0, 180, SERVOMIN, SERVOMAX);
}

int moveServo(int servo_num, int curr_angle, int dest_angle, int speed_delay) {
  float damping_factor = 1.2;
  int percent_start_damping = 90;

  int curr_pulse = angleToPulse(curr_angle);
  board1.setPWM(servo_num, 0, curr_pulse);  
  int dest_pulse = angleToPulse(dest_angle);
  int percent_done = 0;
  int start_pulse = curr_pulse;

  if (curr_angle < dest_angle) {
    // Increase angle with acceleration and deceleration
    for (int i = start_pulse; i <= dest_pulse; i++) {
      board1.setPWM(servo_num, 0, i);
      curr_pulse = i;
      percent_done = ((curr_pulse - start_pulse) * 100) / (dest_pulse - start_pulse);
      
      if (percent_done >= percent_start_damping) {
        delay(speed_delay + damping_factor * (percent_done - percent_start_damping));
      } else {
        if (speed_delay < damping_factor * (100 - percent_done)) {
          delay(speed_delay);
        } else {
          delay(damping_factor * (100 - percent_done));
        }
      }
    }
  } else if (curr_angle > dest_angle) {
    // Decrease angle with acceleration and deceleration
    for (int i = start_pulse; i >= dest_pulse; i--) {
      board1.setPWM(servo_num, 0, i);
      curr_pulse = i;
      percent_done = ((start_pulse - curr_pulse) * 100) / (start_pulse - dest_pulse);
      
      if (percent_done >= percent_start_damping) {
        delay(speed_delay + damping_factor * (percent_done - percent_start_damping));
      } else {
        if (speed_delay < damping_factor * (100 - percent_done)) {
          delay(speed_delay);
        } else {
          delay(damping_factor * (100 - percent_done));
        }
      }
    }
  }

  return dest_angle;
}

void moveServoTask(void *parameter) {
  int servo_num = (int)parameter;
  int current_angle, target_angle;

  // Default initial positions
  if (servo_num == HIP_RIGHT) {
    current_angle = s0_straight;
  } else if (servo_num == KNEE_LEFT ) {
    current_angle = s1_straight;
  } else if (servo_num == KNEE_RIGHT ) {
    current_angle = s2_straight;
  } else if (servo_num == HIP_LEFT) {
    current_angle = s3_straight;
  }

  // Wait for new destination angle from queue
  while (true) {
    xQueueReceive(servoQueue[servo_num], &target_angle, portMAX_DELAY);
    // Move servo to target angle
    if (target_angle < 161 && target_angle > 19) {
      current_angle = moveServo(servo_num, current_angle, target_angle, 2);
    } else {
      Serial.printf("WARNING: Q received unexpected target angle: %u\n", target_angle);
    }
  }
}
