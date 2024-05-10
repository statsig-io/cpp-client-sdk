#pragma once

#include <future>
#include <thread>
#include <memory>
#include <fstream>

typedef std::function<void()> DoneBlocking;

inline bool RunBlocking(long timeout_ms, const std::function<void(DoneBlocking)> &task) {
  std::promise<void> promise;
  auto future = promise.get_future();

  std::thread([&, task]() {
    task([&promise] {
      promise.set_value();
    });
  }).detach();

  auto timeout = std::chrono::milliseconds(timeout_ms);
  if (future.wait_for(timeout) != std::future_status::ready) {
    throw std::runtime_error("RunBlocking timed out");
  }

  return true;
}

inline std::string ReadFile(const std::string &path) {
  const std::string root = ROOT_DIR;
  std::ifstream stream(root + "/" + path, std::ios::binary);

  if (!stream) {
    throw std::runtime_error("Could not open file: " + path);
  }

  std::stringstream buffer;
  buffer << stream.rdbuf();

  stream.close();
  return buffer.str();
}

inline std::string Trim(const std::string &in) {
  std::string out(in);
  out.erase(std::remove_if(out.begin(), out.end(), ::isspace), out.end());
  return out;
}

namespace fs = std::filesystem;
inline void WipeAllCaching() {
  const auto dir = "/tmp/statsig_cpp_client";
  if (fs::exists(dir)) {
    return;
  }

  fs::remove_all(dir);
}

