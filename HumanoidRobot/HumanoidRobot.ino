#include "PWMServo.hpp"
#include "ParallelServo.hpp"
#include "ParallelServoGroup.hpp"

#include <Adafruit_PWMServoDriver.h>
// #include <Wire.h>
// #include <MPU6050.h>


Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
// MPU6050 mpu;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting");

  board1.begin();
  board1.setPWMFreq(60);
  // mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  
  ParallelServo leftKnee =  ParallelServo(board1, 1, 0, 20);
  ParallelServo rightKnee = ParallelServo(board1, 2, 180, 160);
  ParallelServo leftHip =   ParallelServo(board1, 3, 180, 90);
  ParallelServo rightHip =  ParallelServo(board1, 0, 0, 90);
  
  // Each servo is a parallel task
  TaskHandle_t taskHandles[4];
  xTaskCreate(leftKnee.servoTask, "LeftKneeTask", 2048, &leftKnee, 1, &taskHandles[0]);
  xTaskCreate(rightKnee.servoTask, "RightKneeTask", 2048, &rightKnee, 1, &taskHandles[1]);
  xTaskCreate(leftHip.servoTask, "LeftHipTask", 2048, &leftHip, 1, &taskHandles[2]);
  xTaskCreate(rightHip.servoTask, "RightHipTask", 2048, &rightHip, 1, &taskHandles[3]);

  leftKnee.rotate(75);
  rightKnee.rotate(75);
  leftHip.rotate(25);
  rightHip.rotate(25);
  vTaskDelay(pdMS_TO_TICKS(2000));

  leftKnee.straighten();
  rightKnee.straighten();
  leftHip.straighten();
  rightHip.straighten();
  vTaskDelay(pdMS_TO_TICKS(1000));

  leftKnee.rotate(75);
  rightKnee.rotate(75);
  leftHip.rotate(25);
  rightHip.rotate(25);
  vTaskDelay(pdMS_TO_TICKS(1000));

  leftKnee.straighten();
  rightKnee.straighten();
  leftHip.straighten();
  rightHip.straighten();
  vTaskDelay(pdMS_TO_TICKS(2000));

  // Delete all tasks
  for (int i = 0; i < 4; i++) {
    if (taskHandles[i] != nullptr) {
      vTaskDelete(taskHandles[i]);
      taskHandles[i] = nullptr; // Clear the handle
    }
  }
  Serial.println("All tasks deleted");
}

void loop() {

}
