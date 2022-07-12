#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>

#include "process.h"
#include "processor.h"

class System {
 public:
  std::vector<Processor> Cpu();       
  std::vector<Process>& Processes();  
  std::unordered_map<std::string, float>
  MemoryUtilization();            
  std::vector<long> UpTime();    
  int TotalProcesses();          
  int RunningProcesses();       
  std::string Kernel();          
  std::string OperatingSystem();  
  int n_cpu_;
  void UpdateCpu();
  static int sort_by;
  static int offset;
  static std::string sort_by_str;

  int NProcesses();

  System();

 private:
  std::vector<Processor> cpu_ = {};
  std::vector<Process> processes_ = {};
  std::vector<int> pids_;
};

#endif