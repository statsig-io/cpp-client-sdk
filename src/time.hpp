#pragma once

#include <chrono>

namespace statsig::internal {

class Time {
  using milliseconds = std::chrono::milliseconds;
  using system_clock = std::chrono::system_clock;

 public:
  static long now() {
    return std::chrono::duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
  }
};

}