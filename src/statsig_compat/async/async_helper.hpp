#pragma once

#include <thread>

namespace statsig_compatibility {

class AsyncHelper {
 public:
  static void RunInBackground(const std::function<void()> &task) {
    std::thread([task]() {
      task();
    }).detach();
  }

  static void Sleep(const long milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }
};

}
