#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

#include "../output_logger/log.hpp"
#include "../primitives/string.hpp"
#include "statsig/internal/shareable.hpp"
#include "statsig/internal/time.hpp"

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

class BackgroundTimerHandle final {
public:
  BackgroundTimerHandle() = default;

  BackgroundTimerHandle(const BackgroundTimerHandle&) = delete;
  BackgroundTimerHandle& operator=(const BackgroundTimerHandle&) = delete;

  BackgroundTimerHandle(BackgroundTimerHandle&& Other) {
    shutdown_condition = std::move(Other.shutdown_condition);
    Other.shutdown_condition = nullptr;
  }

  BackgroundTimerHandle& operator=(BackgroundTimerHandle&& Other) {
    shutdown_condition = std::move(Other.shutdown_condition);
    Other.shutdown_condition = nullptr;
    return *this;
  }

  BackgroundTimerHandle(const std::shared_ptr<std::atomic<bool>>& in_shutdown_condition)
    : shutdown_condition(in_shutdown_condition) {
  }

  ~BackgroundTimerHandle() {
    Reset();
  }

  void Reset() {
    if (shutdown_condition) {
      shutdown_condition->store(true);
      shutdown_condition = nullptr;
    }
  }

private:
  std::shared_ptr<std::atomic<bool>> shutdown_condition;
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

  void RunInBackground(const std::function<void()> &task) {
    thread_pool_.Add(task);
  }

  BackgroundTimerHandle StartBackgroundTimer(const std::function<void()>& task, int timer_interval_ms) {
    std::shared_ptr<std::atomic<bool>> shutdown_condition = std::make_shared<std::atomic<bool>>(false);

    thread_pool_.Add([task, shutdown_condition, timer_interval_ms](){
      time_t last_attempt = statsig::internal::Time::now();

      while (!shutdown_condition->load()) {
        if (statsig::internal::Time::now() - last_attempt < timer_interval_ms) {
          std::this_thread::sleep_for(std::chrono::milliseconds(timer_interval_ms));
          continue;
        }

        if (shutdown_condition->load()) {
          break;
        }
        last_attempt = statsig::internal::Time::now();

        if (task) {
          task();
        }
      }
    });

    return BackgroundTimerHandle(shutdown_condition);
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

}
