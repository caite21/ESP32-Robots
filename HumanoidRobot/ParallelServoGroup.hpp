#ifndef PARALLELSERVOGROUP_H
#define PARALLELSERVOGROUP_H


#include "ParallelServo.hpp"

/* Allows one task to move multiple servos concurrently */
class ParallelServoGroup {
  private:
    std::vector<ParallelServo*> servos;
    std::vector<uint8_t> destAngles;

  public:
    uint8_t speed_delay;  // 0-255 ms 
    ParallelServoGroup();
    void add(ParallelServo* servo);
    static void multipleServosTask(void* parameter);
};


#endif