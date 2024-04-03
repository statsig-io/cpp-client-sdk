#pragma once

#include <iostream>
#include <filesystem>

namespace statsig {

namespace fs = std::filesystem;
using namespace statsig::constants;

using string = std::string;

class File {
 public:
  static void WriteToCache(const string &key, const string &content) {
    EnsureCacheDirectoryExists();

    auto path = fs::path(kCacheDirectory) / fs::path(key);

    std::ofstream file(path);
    file << content;
    file.close();
  }

  static std::optional<string> ReadFromCache(const string &key) {
    EnsureCacheDirectoryExists();

    auto path = fs::path(kCacheDirectory) / fs::path(key);

    if (!fs::exists(path)) {
      return std::nullopt;
    }

    std::ifstream file(path);
    string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
  }

  static void RunCacheEviction(std::string prefix) {
    std::vector<fs::path> paths;

    for (const auto &entry : fs::directory_iterator(kCacheDirectory)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      const auto filename = entry.path().filename().string();
      if (filename.find_first_of(prefix) == std::string::npos) {
        continue;
      }

      paths.push_back(entry.path());
    }

    sort(paths.begin(), paths.end(),
         [](const fs::path &left, const fs::path &right) {
           return fs::last_write_time(left) > fs::last_write_time(right);
         });

    while (paths.size() > kMaxCacheEntriesCount) {
      const auto &eldest = paths.back();
      fs::remove(eldest);
      paths.pop_back();
    }
  }

  static void EnsureCacheDirectoryExists() {
    if (fs::exists(kCacheDirectory)) {
      return;
    }
    fs::create_directory(kCacheDirectory);
  }
};

}