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

inline std::optional<std::string> ReadFileAbsolute(
    const std::string &path,
    const bool throw_on_not_found = false
) {
  std::ifstream stream(path, std::ios::binary);

  if (!stream && throw_on_not_found) {
    throw std::runtime_error("Could not open file: " + path);
  }

  if (!stream) {
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << stream.rdbuf();

  stream.close();
  return buffer.str();
}

inline std::string ReadFile(const std::string &path) {
  const std::string root = ROOT_DIR;
  return ReadFileAbsolute(root + "/" + path, true).value_or("");
}

inline std::string Trim(const std::string &in) {
  std::string out(in);
  out.erase(std::remove_if(out.begin(), out.end(), ::isspace), out.end());
  return out;
}

namespace fs = std::filesystem;
inline void DeleteDirectory(std::string dir) {
  if (!fs::exists(dir)) {
    return;
  }

  fs::remove_all(dir);
}

inline void WipeAllCaching() {
  const auto dir = "/tmp/statsig_cpp_client";
  DeleteDirectory(dir);
}

