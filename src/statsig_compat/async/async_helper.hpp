#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "../output_logger/log.hpp"
#include "../primitives/string.hpp"
#include "statsig/internal/shareable.hpp"

namespace statsig_compatibility {

class ThreadPool {
  typedef std::function<void()> Task;

 public:
  explicit ThreadPool() = default;

  ~ThreadPool() {
    is_running_.store(false);
    cv.notify_all();

    for (auto &thread : threads) {
      thread.detach();
    }

    threads.clear();
  }

  void Reset(int num_threads) {
    if (is_running_) {
      Log::Warn("ThreadPool must be shutdown before resetting");
      return;
    }

    threads.clear();
    completed_count_.store(0);
    is_running_.store(true);

    for (int i = 0; i < num_threads; i++) {
      CreateThread();
    }
  }

  void Add(const Task &task) {
    std::unique_lock<std::mutex> lock(mutex_);

    tasks.push(task);
    cv.notify_one();
  }

  bool Shutdown(const time_t timeout_ms) {
    is_running_.store(false);
    cv.notify_all();

    const auto t_ms = std::chrono::milliseconds(timeout_ms);
    auto start_time = std::chrono::steady_clock::now();

    while (completed_count_.load() != threads.size()) {
      auto end_time = std::chrono::steady_clock::now();
      const auto duration = end_time - start_time;
      if (duration >= t_ms) {
        break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (completed_count_.load() == threads.size()) {
      for (auto &thread : threads) {
        thread.join();
      }

      threads.clear();
      return true;
    }

    Log::Warn("Failed to gracefully shutdown threads");
    for (auto &thread : threads) {
      thread.detach();
    }

    threads.clear();
    return false;
  }

 private:
  std::vector<std::thread> threads{};
  std::queue<Task> tasks{};
  std::mutex mutex_{};
  std::condition_variable cv{};
  std::atomic<bool> is_running_{false};
  std::atomic<int> completed_count_{0};

  void CreateThread() {
    threads.emplace_back([this] {
      while (true) {
        std::optional<Task> task;
        {
          std::unique_lock<std::mutex> lock(mutex_);
          cv.wait(lock, [this] { return !is_running_ || !tasks.empty(); });

          if (!is_running_ && tasks.empty()) {
            completed_count_.store(completed_count_.load() + 1);
            break;
          }

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

class AsyncHelper {
 public:
  static std::shared_ptr<AsyncHelper> Get(const std::string &sdk_key) {
    auto instance = shareable_.Get(sdk_key);
    if (instance != nullptr) {
      return instance;
    }

    std::shared_ptr<AsyncHelper> new_instance(new AsyncHelper());
    shareable_.Add(sdk_key, new_instance);
    return new_instance;
  }

  static void Sleep(const time_t duration_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
  }

  void RunInBackground(const std::function<void()> &task) {
    thread_pool_.Add(task);
  }

  void Start() {
    thread_pool_.Reset(5);
  }

  bool Shutdown(const time_t timeout_ms) {
    return thread_pool_.Shutdown(timeout_ms);
  }

#ifdef STATSIG_TESTS
  static void ShutdownAll() {
    for (const auto &pair : shareable_.GetInstances()) {
      pair.second->Shutdown(1);
    }
  }
#endif

 private:
  static statsig::internal::Shareable<AsyncHelper> shareable_;
  ThreadPool thread_pool_{};
};

statsig::internal::Shareable<AsyncHelper> AsyncHelper::shareable_ = statsig::internal::Shareable<AsyncHelper>();

}
