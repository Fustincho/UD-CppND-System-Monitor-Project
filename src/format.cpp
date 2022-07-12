#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  long sec = seconds % 60;
  long min = (seconds / 60) % 60;
  long hrs = seconds / 3600;

  string str_sec = std::to_string(sec);
  string str_min = std::to_string(min);
  string str_hrs = std::to_string(hrs);

  if (sec < 10) {
    str_sec = "0" + str_sec;
  }
  if (min < 10) {
    str_min = "0" + str_min;
  }

  return (str_hrs + ":" + str_min + ":" + str_sec);
}