#pragma once

#include <string>
#include <iostream>
#include "../log_level.h"

namespace statsig::internal {

class Log {
  inline static const char* RED = "\033[31m";
  inline static const char* YELLOW = "\033[33m";
  inline static const char* BLUE = "\033[34m";
  inline static const char* RESET = "\033[0m";

public:
  static LogLevel level;

  static void Error(const std::string& value) {
    if (level >= LogLevel::Error) {
      std::cerr << RED << "[Statsig]: " << value << RESET << std::endl;
    }
  }

  static void Warn(const std::string& value) {
    if (level >= LogLevel::Warn) {
      std::cout << YELLOW << "[Statsig]: " << value << RESET << std::endl;
    }
  }

  static void Debug(const std::string& value) {
    if (level >= LogLevel::Debug) {
      std::cout << BLUE << "[Statsig]: " << value << RESET << std::endl;
    }
  }
};

LogLevel Log::level = LogLevel::Error;

}
