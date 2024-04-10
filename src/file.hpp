#pragma once

#include <filesystem>

#include "statsig_compatibility/filesystem/file_helper.hpp"

namespace statsig::internal {

namespace fs = std::filesystem;

class File {
public:
  using string = std::string;
  using FileHelper = statsig_compatibility::FileHelper;

  static void WriteToCache(const string& key, const string& content) {
    FileHelper::EnsureCacheDirectoryExists();

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    FileHelper::WriteStringToFile(content, path);
  }

  static std::optional<string> ReadFromCache(const string& key) {
    FileHelper::EnsureCacheDirectoryExists();

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    return FileHelper::ReadStringToFile(path);
  }

  static void RunCacheEviction(std::string prefix) {
    auto paths = FileHelper::GetCachePathsSortedYoungestFirst(prefix);

    if (paths.size() < constants::kMaxCacheEntriesCount) {
      return;
    }
    
    while (paths.size() > constants::kMaxCacheEntriesCount) {
      const auto& eldest = paths.back();
      FileHelper::DeleteFile(eldest);
      paths.pop_back();
    }
  }
};

}
