#pragma once

#include <future>
#include <thread>
#include <memory>

typedef std::function<void()> DoneBlocking;

inline bool RunBlocking(long timeout_ms, const std::function<void(DoneBlocking)> &task) {
  std::promise<void> promise;
  auto future = promise.get_future();

  std::thread([&, task]() {
    task([&promise]{
      promise.set_value();
    });
  }).detach();

  auto timeout = std::chrono::milliseconds(timeout_ms);
  if (future.wait_for(timeout) != std::future_status::ready) {
    throw std::runtime_error("RunBlocking timed out");
  }

  return true;
}
