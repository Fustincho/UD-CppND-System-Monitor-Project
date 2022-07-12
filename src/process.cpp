#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"
#include "system.h"

/* Logging Tools:
#include <cstdio>
#include <iostream>
*/

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  
  Pid_ = pid;
  Ram_ = this->Ram();
  RefreshCpuUtilization();
}

void Process::RefreshCpuUtilization() {
  // This basically becomes total_time / SC_CLK, because we measure every second
  // the CpuUsage
  vector<float> cpu_values = LinuxParser::CpuUtilization(Pid_);

  float d_utime = cpu_values[0] - utime_;
  float d_stime = cpu_values[1] - stime_;
  float d_cutime = cpu_values[2] - cutime_;
  float d_cstime = cpu_values[3] - cstime_;
  // This is zero for all iterations after the first one
  float d_starttime_ = cpu_values[4] - starttime_;

  float total_time = d_utime + d_stime + d_cutime + d_cstime;

  long sys_utime = LinuxParser::UpTime()[0];
  long d_sys_utime = sys_utime - sys_utime_;

  float seconds = d_sys_utime - (d_starttime_ / sysconf(_SC_CLK_TCK));
  float cpu_usage = (total_time / sysconf(_SC_CLK_TCK)) / seconds;

  // Update values
  utime_ = cpu_values[0];
  stime_ = cpu_values[1];
  cutime_ = cpu_values[2];
  cstime_ = cpu_values[3];
  starttime_ = cpu_values[4];
  sys_utime_ = sys_utime;

  CpuUtilization_ = cpu_usage;

  Ram_ = this->Ram();
}

// Return this process's ID
int Process::Pid() { return Pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() { return CpuUtilization_; }

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(Pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  switch (System::sort_by)
  {
  case 2:
    System::sort_by_str = "[PID (asc)]";
    return a.Pid_ > Pid_;
    
  case 3:
    System::sort_by_str = "[PID (des)]";
    return a.Pid_ < Pid_; 
    
  case 4:
    System::sort_by_str = "[RAM (asc)]";
    return std::stof(a.Ram_) > std::stof(Ram_);
  
  case 5:
    System::sort_by_str = "[RAM (des)]";
    return std::stof(a.Ram_) < std::stof(Ram_);
  
  default:
    System::sort_by_str = "[CPU ( % )]";
    return a.CpuUtilization_ < CpuUtilization_;
  }
  
}