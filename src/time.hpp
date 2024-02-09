#pragma once

#include <chrono>

namespace statsig {

using namespace std::chrono;

class Time {
 public:
  static long now() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  }
};

}