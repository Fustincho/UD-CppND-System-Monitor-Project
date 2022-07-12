#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Logging Tools:
#include <cstdio>
#include <iostream>


#include "format.h"
#include "ncurses_display.h"
#include "system.h"

using std::string;
using std::to_string;

// 40 bars uniformly displayed from 0 - 100 %
// 2.5% is one bar(|)
void NCursesDisplay::CPUProgressBar(float percent, WINDOW* window, int size) {
  int color_pair{2};
  // High Consumption -> Red , Medium -> Yellow, Low -> Green
  if (0.5 <= percent) {
    if (percent < 0.8) {
      color_pair = 4;
    } else {
      color_pair = 5;
    }
  }

  BarHeader(percent, color_pair, window);
  std::string result{""};

  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  wattron(window, COLOR_PAIR(color_pair));
  wprintw(window, result.c_str());
  wattroff(window, COLOR_PAIR(color_pair));
}

std::string NCursesDisplay::PopulateProgressBar(float percent, std::string icon,
                                                int size) {
  std::string result{""};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    if (i <= bars) {
      result += icon;
    }
  }
  return result;
}

void NCursesDisplay::MemoryProgressBar(
    std::unordered_map<std::string, float> data_map, WINDOW* window, int row) {
  // Why does it work with only one % on the original code?!
  // wprintw(window, {"0%%"});

  float MemTotal = data_map.find("MemTotal")->second;
  float NCBM = data_map.find("NonCacheBufferMemory")->second;
  // float TUM_percent = (data_map.find("TotalUsedMemory")->second) / MemTotal;
  float NCBM_percent = NCBM / MemTotal;
  float B_percent = (data_map.find("Buffers")->second) / MemTotal;
  float CM_percent = (data_map.find("CachedMemory")->second) / MemTotal;

  BarHeader(NCBM_percent, 2, window);

  // Green Bar (Non Cache Buffer Memory)
  wattron(window, COLOR_PAIR(2));
  wprintw(window,
          NCursesDisplay::PopulateProgressBar(NCBM_percent, "|", getmaxx(stdscr) - 17 - 18).c_str());
  wattroff(window, COLOR_PAIR(2));
  // Blue Bar (Buffers)
  wattron(window, COLOR_PAIR(3));
  wprintw(window,
          NCursesDisplay::PopulateProgressBar(B_percent, "|", getmaxx(stdscr) - 17 - 18).c_str());
  wattroff(window, COLOR_PAIR(3));
  // Yellow Bar (Cache)
  wattron(window, COLOR_PAIR(4));
  wprintw(window,
          NCursesDisplay::PopulateProgressBar(CM_percent, "|", getmaxx(stdscr) - 17 - 18).c_str());
  wattroff(window, COLOR_PAIR(4));
  // Move to the end of a theoretical filled progress bar. 10 + 8 + 40 possible
  // bars
  mvwprintw(window, row, getmaxx(stdscr) - 17, "");

  BarTail(NCBM, MemTotal, window);
}

void NCursesDisplay::BarHeader(float percent, int color_pair, WINDOW* window) {
  // e.g., [50%]
  wprintw(window, "[");
  // Ensures that the number has a length of 4.
  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  wattron(window, COLOR_PAIR(color_pair));
  wprintw(window, (display + "%%").c_str());
  wattroff(window, COLOR_PAIR(color_pair));
  wprintw(window, "] ");
}

void NCursesDisplay::BarTail(float usedGB, float totalGB, WINDOW* window) {
  std::string display{"[" + to_string(usedGB).substr(0, 4) + "/"};
  if (usedGB < 10) display = "[ " + to_string(usedGB).substr(0, 3) + "/";
  if (totalGB < 10) {
    display += " " + to_string(totalGB).substr(0, 3) + " GB]";
  } else {
    display += to_string(totalGB).substr(0, 4) + " GB]";
  }
  wprintw(window, display.c_str());
}

void NCursesDisplay::SwapProgressBar(
    std::unordered_map<std::string, float> data_map, WINDOW* window, int row) {
  float SwapTotal = data_map.find("SwapTotal")->second;
  float SwapUsed = SwapTotal - (data_map.find("SwapFree")->second);
  float Swap_percent = SwapUsed / SwapTotal;

  BarHeader(Swap_percent, 2, window);
  // Move to the end of a theoretical filled progress bar. 10 + 8 + 40 possible
  // bars
  mvwprintw(window, row, getmaxx(stdscr) - 17 , "");

  BarTail(SwapUsed, SwapTotal, window);
}

void NCursesDisplay::DisplayCPUs(System& system, WINDOW* window, int& row) {
  system.UpdateCpu();
  for (auto& cpu : system.Cpu()) {
    CleanLine(window, ++row, 2);
    std::string cpu_name = "CPU " + to_string(cpu.GetCpuNumber()) + ": ";
    mvwprintw(window, row, 2, cpu_name.c_str());
    mvwprintw(window, row, 10, "");
    // wprintw(window, CPUProgressBar(cpus[i].Utilization()).c_str());
    CPUProgressBar(cpu.Utilization(), window, getmaxx(stdscr) - 17 - 18);
  }
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  CleanLine(window, ++row, 2);
  mvwprintw(window, row, 2, ("OS: " + system.OperatingSystem()).c_str());
  mvwprintw(window, row, getmaxx(stdscr) - 38 , "by: Udacity n' github.com/Fustincho");
  CleanLine(window, ++row, 2);
  mvwprintw(window, row, 2, ("Kernel: " + system.Kernel()).c_str());
  DisplayCPUs(system, window, row);
  CleanLine(window, ++row, 2);
  mvwprintw(window, row, 2, "Memory: ");
  MemoryProgressBar(system.MemoryUtilization(), window, row);
  CleanLine(window, ++row, 2);
  mvwprintw(window, row, 2, "Swap: ");
  mvwprintw(window, row, 10, "");
  SwapProgressBar(system.MemoryUtilization(), window, row);
  CleanLine(window, ++row, 2);
  mvwprintw(window, row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  CleanLine(window, ++row, 2);
  mvwprintw(
      window, row, 2,
      ("Running Processes: " + to_string(system.RunningProcesses())).c_str());
  std::vector<long> up_times = system.UpTime();
  CleanLine(window, ++row, 2);
  mvwprintw(window, row, 2,
            ("Up Time: " + Format::ElapsedTime(up_times[0])).c_str());
  wprintw(window, (" - Idle Time (Combined of all CPUs): " +
                   Format::ElapsedTime(up_times[1]))
                      .c_str());
  wrefresh(window);
}

void NCursesDisplay::CleanLine(WINDOW* window, int& row, int column){
  mvwprintw(window, row, column, (string(window->_maxx - 2, ' ').c_str()));
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* window, int n) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{16};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};
  wattron(window, COLOR_PAIR(2));
  CleanLine(window, ++row, pid_column);
  mvwprintw(window, row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "EL. TIME");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));

  int max_procs = std::min(window->_maxy - 3, n);
  for (int j = 0; j < max_procs; j++) {
    int i = j + System::offset;
    // You need to take care of the fact that the cpu utilization has already
    // been multiplied by 100.
    // Clear the line

    // In case there are more processes requested than actual number of processes,
    // avoid trying to print them
    if(i >= static_cast<int>(processes.size())){
      break;
    }

    CleanLine(window, ++row, pid_column);

    mvwprintw(window, row, pid_column, to_string(processes[i].Pid()).c_str());

    mvwprintw(window, row, user_column, processes[i].User().c_str());

    float cpu = processes[i].CpuUtilization() * 100.;

    mvwprintw(window, row, cpu_column, to_string(cpu).substr(0, 4).c_str());

    mvwprintw(window, row, ram_column, processes[i].Ram().c_str());

    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());

    mvwprintw(window, row, command_column,
              processes[i].Command().substr(0, window->_maxx - 46).c_str());
  }
  CleanLine(window, ++row, pid_column);
  wattron(window, COLOR_PAIR(6));
  mvwprintw(window, row, window->_maxx/2 - 13, ("Sort (H) - By: " + System::sort_by_str).c_str());
  std::string Exit = "Exit (Q)";
  mvwprintw(window, row, window->_maxx - 9, Exit.c_str());
  std::string nav = "Navigate: ";
  mvwprintw(window, row, 2, (nav + std::to_string(System::offset)).c_str());
  wattroff(window, COLOR_PAIR(6));
}

void NCursesDisplay::Display(System& system, int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  nodelay(stdscr, TRUE); // no delay mode (so that getch() doesn't wait for an input)

  keypad(stdscr, TRUE); // Let there be a keypad!

  int x_max{getmaxx(stdscr)};
  WINDOW* system_window = newwin(9 + system.n_cpu_, x_max - 1, 0, 0);
  //WINDOW* process_window =
  //    newwin(3 + n + 1, x_max - 1, system_window->_maxy + 1, 0);
  int y_max{getmaxy(stdscr)};

  int max_hei = std::min(y_max - system_window->_maxy - 1, 4 + n);
  WINDOW* process_window =
        newwin(max_hei, x_max - 1, system_window->_maxy + 1, 0);

  while (1) {
    fflush(stdin);
    int c = getch();
    switch (c)
    {
    case 113:
      endwin();
      exit(EXIT_SUCCESS);
      break;

    case 104:
      if (System::sort_by == 5){
        System::sort_by = 1;
        break;
      } else {
        System::sort_by++;
        break;
      }

    case KEY_UP:
      if (System::offset == 0){
        System::offset = 0;
        break;
      } else {
        System::offset--;
        break;
      }

    case KEY_DOWN:
      if (System::offset + max_hei - 4 == system.NProcesses()){
        break;
      } else {
        System::offset++;
        break;
      }
    
    default:
      break;
    }

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);
    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window, n);
    wrefresh(system_window);
    wrefresh(process_window);
    refresh();
    // The delay will be made within LinuxParser::DeltaCpuUtilization
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cerr << "\n";
  }
  endwin();
}
