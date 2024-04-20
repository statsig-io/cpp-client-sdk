#pragma once

namespace statsig {

class AsyncHelper {
 public:
  static void RunInBackground(const std::function<void()> &task) {
    std::thread([task]() {
      task();
    }).detach();
  }
};

}
