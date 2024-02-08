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

    ofstream file;
    file.open(path);
    file << content;
    file.close();
  }

  static optional<string> ReadFromCache(const string &key) {
    EnsureCacheDirectoryExists();

    auto path = fs::path(kCacheDirectory) / fs::path(key);

    if (!fs::exists(path)) {
      return nullopt;
    }

    ifstream file;
    file.open(path);
    string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
  }

  static void EnsureCacheDirectoryExists() {
    if (fs::exists(kCacheDirectory)) {
      return;
    }
    fs::create_directory(kCacheDirectory);
  }
};

}