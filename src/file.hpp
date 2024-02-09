#pragma once

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

using namespace statsig::constants;

namespace statsig {

class File {
 public:
  static void WriteToCache(const string &key, const string &content) {
    EnsureCacheDirectoryExists();

    auto path = fs::path(kCacheDirectory) / fs::path(key);

    ofstream file(path);
    file << content;
    file.close();
  }

  static optional<string> ReadFromCache(const string &key) {
    EnsureCacheDirectoryExists();

    auto path = fs::path(kCacheDirectory) / fs::path(key);

    if (!fs::exists(path)) {
      return nullopt;
    }

    ifstream file(path);
    string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
  }

  static void RunCacheEviction() {
    vector<fs::path> paths;

    for (const auto &entry : fs::directory_iterator(kCacheDirectory)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      if (entry.path().filename() == kStableIdKey) {
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