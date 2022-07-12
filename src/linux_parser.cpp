#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <iomanip>

/* Logging Tools:
#include <cstdio>
#include <iostream>
*/

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

std::unordered_map<std::string, float> LinuxParser::MemoryUtilization() {
  string line, key, value;
  // Total used memory
  float MemTotal, MemFree, Buffers;
  // Cached memory
  float Cached, SReclaimable, Shmem;
  // Swap memory
  float SwapTotal, SwapFree;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        // In the file, the values are saved in kB.
        // They are converted to GB
        if (key == "MemTotal") {
          MemTotal = std::stof(value) / (1024 * 1024);
        }
        if (key == "MemFree") {
          MemFree = std::stof(value) / (1024 * 1024);
        }
        if (key == "Buffers") {
          Buffers = std::stof(value) / (1024 * 1024);
        }
        if (key == "Cached") {
          Cached = std::stof(value) / (1024 * 1024);
        }
        if (key == "SReclaimable") {
          SReclaimable = std::stof(value) / (1024 * 1024);
        }
        if (key == "Shmem") {
          Shmem = std::stof(value) / (1024 * 1024);
        }
        if (key == "SwapTotal") {
          SwapTotal = std::stof(value) / (1024 * 1024);
        }
        if (key == "SwapFree") {
          SwapFree = std::stof(value) / (1024 * 1024);
        }
      }
    }
  }
  std::unordered_map<string, float> memory_map;
  memory_map["MemTotal"] = MemTotal;
  memory_map["TotalUsedMemory"] = MemTotal - MemFree;
  memory_map["Buffers"] = Buffers;
  memory_map["NonCacheBufferMemory"] =
      (MemTotal - MemFree) - (Buffers + (Cached + SReclaimable - Shmem));
  memory_map["CachedMemory"] = Cached + SReclaimable - Shmem;
  memory_map["SwapTotal"] = SwapTotal;
  memory_map["SwapFree"] = SwapFree;

  return memory_map;
}

// Read and return the system uptime
std::vector<long> LinuxParser::UpTime() {
  float UpTime;
  float IdleTime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  std::string line;
  std::vector<long> times;
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> UpTime >> IdleTime;
    times = {static_cast<long>(UpTime), static_cast<long>(IdleTime)};
  }
  return times;
}

// Read and return CPU utilization
std::vector<std::vector<float>> LinuxParser::CpuUtilization() {
  string cpuN;
  float user;
  float nice;
  float system;
  float idle;
  float iowait;
  float irq;
  float softirq;
  float steal;
  float guest;
  float guest_nice;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  std::vector<std::vector<float>> result;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> cpuN >> user >> nice >> system >> idle >> iowait >>
             irq >> softirq >> steal >> guest >> guest_nice) {
        // We only care about individual CPUs
        if (std::regex_match(cpuN, std::regex("cpu[0-9]+"))) {
          std::vector<float> cpu_utilization;
          cpu_utilization = {user, nice,    system, idle,  iowait,
                             irq,  softirq, steal,  guest, guest_nice};
          result.push_back(cpu_utilization);
        }
      }
    }
  }
  return result;
}

int LinuxParser::NumberCPUs() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  std::vector<string> cpus;
  std::string cpuN;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> cpuN) {
        // We only care about individual CPUs
        if (std::regex_match(cpuN, std::regex("cpu[0-9]+"))) {
          cpus.push_back(cpuN);
        }
      }
    }
  }
  return static_cast<int>(cpus.size());
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        // We only care about individual CPUs
        if (key == "processes") {
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        // We only care about individual CPUs
        if (key == "procs_running") {
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  std::string line;
  std::getline(filestream, line);
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string key;
  float value;
  string ram;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  std::string line;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          // kB to MB
          value = value / 1024;
          value = std::round(value * 100) / 100;

          // Float to String (to_string() always outputs 6 decimal numbers)
          std::stringstream stream;
          stream << std::fixed << std::setprecision(2) << value;
          std::string ram = stream.str();
          // ram = to_string(value);
          return ram;
        }
      }
    }
  }
  return ram;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  std::string line;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string target_uid = Uid(pid);
  string line;
  string uid;
  string user;
  string x_char;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> x_char >> uid) {
        if (uid == target_uid) {
          return user;
        }
      }
    }
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string value;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      for (int i = 0; i < 22; i++) {
        linestream >> value;
      }
    }
  }
  long uptime = UpTime()[0] - (stol(value) / sysconf(_SC_CLK_TCK));
  return uptime;
}

std::vector<float> LinuxParser::CpuUtilization(int pid) {
  std::vector<float> values{};
  string value;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      for (int i = 0; i < 22; i++) {
        linestream >> value;
        if (i == 13 || i == 14 || i == 15 || i == 16 || i == 21) {
          values.push_back(stof(value));
        }
      }
    }
  }
  return values;
}