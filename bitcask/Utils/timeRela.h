#pragma once
#include <iomanip>
#include <time.h>
#include <chrono>


// 2019-11-28
std::string getStrToday();
// 2019-11-28 23:23:12
std::string getCurrentOfFormat(const char* format);
// 1970-01-01 00:00:00 --
std::string getCurrentOfSecond();