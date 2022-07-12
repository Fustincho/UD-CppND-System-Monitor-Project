#include "processor.h"

/* Logging Tools:
#include <cstdio>
#include <iostream>
*/

// Return the aggregate CPU utilization
float Processor::Utilization() { return utilization_; }

void Processor::Update(std::vector<float> cpu_data) {
  this->user_ = cpu_data[0];
  this->nice_ = cpu_data[1];
  this->system_ = cpu_data[2];
  this->idle_ = cpu_data[3];
  this->iowait_ = cpu_data[4];
  this->irq_ = cpu_data[5];
  this->softirq_ = cpu_data[6];
  this->steal_ = cpu_data[7];
  this->guest_ = cpu_data[8];
  this->guest_nice_ = cpu_data[9];

  float Idle = idle_ - prev_idle_ + iowait_ - prev_iowait_;
  float NonIdle = user_ - prev_user_ + nice_ - prev_nice_ + system_ -
                  prev_system_ + irq_ - prev_irq_ + softirq_ - prev_softirq_ +
                  steal_ - prev_steal_;
  float Total = Idle + NonIdle;

  this->prev_user_ = user_;
  this->prev_nice_ = nice_;
  this->prev_system_ = system_;
  this->prev_idle_ = idle_;
  this->prev_iowait_ = iowait_;
  this->prev_irq_ = irq_;
  this->prev_softirq_ = softirq_;
  this->prev_steal_ = steal_;
  this->prev_guest_ = guest_;
  this->prev_guest_nice_ = guest_nice_;

  this->utilization_ = 1 - (Idle / Total);
}

int Processor::GetCpuNumber() { return cpu_n_; }