#pragma once

#include <chrono>

using namespace std;
using namespace std::chrono;

namespace statsig {
class Time {
 public:
  static long now() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  }
};
}