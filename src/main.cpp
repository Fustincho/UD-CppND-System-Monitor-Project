#include "ncurses_display.h"
#include "system.h"
#include <string>

// Logging Tools:
#include <cstdio>
#include <iostream>



int main(int argc, char** argv) {
  std::freopen("/home/workspace/CppND-System-Monitor/build/output.txt", "w",
   stderr);

  // TODO: make this better. Include other arguments, check if PIDs < n, etc.
  System system;
  
  if (argc > 1) {
    NCursesDisplay::Display(system, std::stoi(argv[1]));
  } else {
    NCursesDisplay::Display(system);
  }
}
