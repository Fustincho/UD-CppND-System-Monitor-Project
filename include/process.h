#ifndef PROCESS_H
#define PROCESS_H

#include <string>

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  int Pid();                               
  std::string User();                      
  std::string Command();                   
  float CpuUtilization();                  
  std::string Ram();                       
  long int UpTime();                       
  bool operator<(Process const& a) const;  
  void RefreshCpuUtilization();

  Process(int pid);

  // Declare any necessary private members
 private:
  int Pid_;
  float CpuUtilization_;
  std::string Command_;
  std::string Ram_;
  std::string Uid_;
  long int UpTime_;

  // CPU values:

  float utime_{0};
  float stime_{0};
  float cutime_{0};
  float cstime_{0};
  float starttime_{0};
  float sys_utime_{0};
};

#endif