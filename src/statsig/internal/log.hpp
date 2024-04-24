#pragma once

#include <string>
#include <iostream>
#include "../log_level.h"

namespace statsig::internal {

class Log {
 public:
  static LogLevel level;

  static void Error(const std::string& value) {
    if (Log::level >= LogLevel::Error) {
      std::cerr << "[Statsig]: " << value << std::endl;
    }
  }

  static void Warn(const std::string& value) {
    if (Log::level >= LogLevel::Warn) {
      std::cout << "[Statsig]: " << value << std::endl;
    }
  }

  static void Debug(const std::string& value) {
    if (Log::level >= LogLevel::Debug) {
      std::cout << "[Statsig]: " << value << std::endl;
    }
  }
};

LogLevel Log::level = LogLevel::Error;

}