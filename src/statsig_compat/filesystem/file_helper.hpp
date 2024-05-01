#pragma once

#include <string>
#include <optional>
#include <filesystem>
#include <fstream>

#include "statsig/internal/constants.h"
#include "../async/async_helper.hpp"

namespace statsig_compatibility {

namespace fs = std::filesystem;

using namespace statsig::constants;

class FileHelper {
 public:
  static std::string GetCacheDir() {
    return kCacheDirectory;
  }

  static std::string CombinePath(const std::string &left,
                                 const std::string &right) {
    auto path = fs::path(left) / fs::path(right);
    return path.string();
  }

  static void WriteStringToFile(
      const std::string &content,
      const std::string &path,
      const std::function<void(bool)> &callback
  ) {
    AsyncHelper::RunInBackground([content, path, callback] {
      auto actual_path = fs::path(path);

      std::ofstream file(actual_path);
      file << content;
      file.close();

      callback(true);
    });
  }

  static std::optional<std::string> ReadStringToFile(
      const std::string &path
  ) {
    auto actual_path = fs::path(path);
    if (!fs::exists(path)) {
      return std::nullopt;
    }

    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
  }

  static void EnsureCacheDirectoryExists() {
    if (fs::exists(kCacheDirectory)) {
      return;
    }
    fs::create_directory(kCacheDirectory);
  }

  static void DeleteFile(const std::string &path) {
    auto actual_path = fs::path(path);
    fs::remove(actual_path);
  }

  static std::vector<std::string> GetCachePathsSortedYoungestFirst(const std::string &prefix) {
    std::vector<fs::path> paths;

    for (const auto &entry : fs::directory_iterator(kCacheDirectory)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      if (entry.path().filename().string().find(prefix) == std::string::npos) {
        continue;
      }

      paths.push_back(entry.path());
    }

    sort(paths.begin(), paths.end(),
         [](const fs::path &left, const fs::path &right) {
           return fs::last_write_time(left) > fs::last_write_time(right);
         });

    std::vector<std::string> result;
    result.reserve(paths.size());

    std::transform(
        paths.begin(),
        paths.end(),
        std::back_inserter(result),
        [](const fs::path &p) -> std::string {
          return p.string();
        });

    return result;
  }
};

}
