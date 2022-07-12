#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>
#include <unordered_map>
#include <string>

#include "process.h"
#include "system.h"

namespace NCursesDisplay {
void Display(System& system, int n = 10);
void DisplaySystem(System& system, WINDOW* window);
void DisplayProcesses(std::vector<Process>& processes, WINDOW* window, int n);
void CPUProgressBar(float percent, WINDOW* window, int size);
void MemoryProgressBar(std::unordered_map<std::string, float> data_map,
                       WINDOW* window, int row);
void SwapProgressBar(std::unordered_map<std::string, float> data_map,
                     WINDOW* window, int row);
void DisplayCPUs(System& system, WINDOW* window, int& row);
std::string PopulateProgressBar(float percent, std::string icon, int size);
void BarHeader(float percent, int color_pair, WINDOW* window);
void BarTail(float usedGB, float totalGB, WINDOW* window);
void CleanLine(WINDOW* window, int& row, int column);

};  // namespace NCursesDisplay

#endif