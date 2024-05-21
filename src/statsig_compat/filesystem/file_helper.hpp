#pragma once

#include <string>
#include <optional>
#include <filesystem>
#include <fstream>

#include "statsig/internal/constants.h"
#include "statsig/statsig_options.h"
#include "../async/async_helper.hpp"
#include "../primitives/file_path.hpp"

namespace statsig_compatibility {

namespace fs = std::filesystem;

using namespace statsig::constants;

class FileHelper {
 public:
  static statsig::FilePath GetCacheDir(const statsig::StatsigOptions &options) {
    return options.cache_directory.value_or(kCacheDirectory);
  }

  static statsig::FilePath CombinePath(
      const statsig::FilePath &left,
      const statsig::FilePath &right
  ) {
    auto path = left / right;
    return path;
  }

  static void WriteStringToFile(
      const std::string &sdk_key,
      const std::string &content,
      const statsig::FilePath &path,
      const std::function<void(bool)> &callback
  ) {
    AsyncHelper::Get(sdk_key)->RunInBackground([content, path, callback]() {
      std::ofstream file(path);
      file << content;
      file.close();

      callback(true);
    });
  }

  static std::optional<std::string> ReadStringToFile(
      const statsig::FilePath &path
  ) {
    if (!fs::exists(path)) {
      return std::nullopt;
    }

    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
  }

  static void EnsureCacheDirectoryExists(
      const statsig::StatsigOptions &options
  ) {
    if (fs::exists(GetCacheDir(options))) {
      return;
    }
    fs::create_directories(GetCacheDir(options));
  }

  static void DeleteFile(const statsig::FilePath &path) {
    fs::remove(path);
  }

  static std::vector<statsig::FilePath> GetCachePathsSortedYoungestFirst(
      const statsig::StatsigOptions &options,
      const std::string &prefix
  ) {
    std::vector<fs::path> paths;

    for (const auto &entry : fs::directory_iterator(GetCacheDir(options))) {
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

    return paths;
  }
};

}
