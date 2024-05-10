#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "macros.hpp"

namespace statsig::internal {

class ThreadPool {
  typedef std::function<void()> Task;

 public:
  ThreadPool(int num_threads) {
    is_running_.store(true);

    for (int i = 0; i < num_threads; i++) {
      CreateThread();
    }
  }

  void Add(const Task &t){
    LOCK(mutex_);

    tasks.push(t);
    cv.notify_one();
  }

  void Shutdown(const time_t timeout_ms) {
    is_running_.store(false);
    cv.notify_all();
  }

 private:
  std::vector<std::thread> threads;
  std::queue<Task> tasks;
  std::mutex mutex_;
  std::condition_variable cv;
  std::atomic<bool> is_running_{true};

  void CreateThread() {
    threads.emplace_back([this]{
      while (true) {
        std::optional<Task> task;

        {
          std::unique_lock<std::mutex> lock(mutex_);
          cv.wait(lock, [this] { return !is_running_ || !tasks.empty(); });

          if (!is_running_ && tasks.empty()) break;

          task = std::move(tasks.front());
          tasks.pop();
        }

        if (task.has_value()) {
          task.value()();
        }
      }
    });
  }
};

}