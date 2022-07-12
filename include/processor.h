#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>

class Processor {
 public:
  float Utilization();  
  void Update(std::vector<float> cpu_data);
  int GetCpuNumber();
  Processor(int cpu_n) : cpu_n_(cpu_n) {}

 private:
  int cpu_n_;
  float user_, prev_user_{0};
  float nice_, prev_nice_{0};
  float system_, prev_system_{0};
  float idle_, prev_idle_{0};
  float iowait_, prev_iowait_{0};
  float irq_, prev_irq_{0};
  float softirq_, prev_softirq_{0};
  float steal_, prev_steal_{0};
  float guest_, prev_guest_{0};
  float guest_nice_, prev_guest_nice_{0};
  float utilization_;
};

#endif