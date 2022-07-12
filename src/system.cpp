#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

// Logging
#include <cstdio>
#include <iostream>

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
vector<Processor> System::Cpu() { return cpu_; }

void System::UpdateCpu() {
  // vector<vector<float>> CPUs_data = LinuxParser::DeltaCpuUtilization();
  std::vector<std::vector<float>> CPUs_data = LinuxParser::CpuUtilization();

  for (int n = 0; n < n_cpu_; n++) {
    this->cpu_[n].Update(CPUs_data[n]);
  }
}

System::System() {
  this->n_cpu_ = LinuxParser::NumberCPUs();
  for (int n = 0; n < n_cpu_; n++) {
    this->cpu_.push_back(Processor(n));
  }
}

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> retrieved_pids = LinuxParser::Pids();
  vector<int> new_pids;

  vector<int> check_pids = pids_;
  for (int Pid : check_pids) {
    // If the old Pids are NOT in the new pids: Remove old PID
    if (std::find(retrieved_pids.begin(), retrieved_pids.end(), Pid) ==
        retrieved_pids.end()) {
      for (auto iter = pids_.begin(); iter != pids_.end(); ++iter) {
        if (*iter == Pid) {
          pids_.erase(iter);

          break;
        }
      }
      for (auto iter = processes_.begin(); iter != processes_.end(); ++iter) {
        if (iter->Pid() == Pid) {
          processes_.erase(iter);

          break;
        }
      }
    }
  }

  for (int Pid : retrieved_pids) {
    // If the new Pids are NOT in the old pids: add new PID
    if (std::find(pids_.begin(), pids_.end(), Pid) == pids_.end()) {
      Process process = Process(Pid);
      pids_.push_back(Pid);
      processes_.push_back(process);
      new_pids.push_back(Pid);
    }
  }

  for (auto& process : processes_) {
    bool need_to_refresh = true;

    for (auto new_pid : new_pids) {
      // New Pids (and processes) get the CPU Utilization immediately. If the
      // process is old
      // i.e., is not listed within the new pids, then refresh their CPU
      // Utilization.
      if (new_pid == process.Pid()) {
        need_to_refresh = false;
      }
    }

    if (need_to_refresh) {
      process.RefreshCpuUtilization();
    }
  }

  std::sort(processes_.begin(), processes_.end());

  return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// Return the system's memory utilization
std::unordered_map<std::string, float> System::MemoryUtilization() {
  return LinuxParser::MemoryUtilization();
}

// Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
std::vector<long> System::UpTime() { return LinuxParser::UpTime(); }

int System::NProcesses(){
  return static_cast<int>(processes_.size());
}

int System::sort_by = 1;
int System::offset = 0;

std::string System::sort_by_str = "[CPU %]";