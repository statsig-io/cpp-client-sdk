#pragma once

#ifndef STATSIG_DISABLE_FILESYSTEM
#include <filesystem>
#endif // STATSIG_DISABLE_FILESYSTEM

#include "statsig_compatibility/filesystem/file_helper.hpp"

namespace statsig::internal {

#ifndef STATSIG_DISABLE_FILESYSTEM
namespace fs = std::filesystem;
#endif // STATSIG_DISABLE_FILESYSTEM

class File {
public:
  using string = std::string;
  using FileHelper = statsig_compatibility::FileHelper;

  static void WriteToCache(const string& key, const string& content) {
    FileHelper::EnsureCacheDirectoryExists();

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    FileHelper::WriteStringToFile(content, path);
  }

  static void DeleteFromCache(const string& key) {
    FileHelper::EnsureCacheDirectoryExists();
    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    FileHelper::DeleteFile(path);
  }

  static std::optional<string> ReadFromCache(const string& key) {
    FileHelper::EnsureCacheDirectoryExists();

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    return FileHelper::ReadStringToFile(path);
  }

  static void RunCacheEviction(std::string prefix) {
    auto paths = FileHelper::GetCachePathsSortedYoungestFirst(prefix);

    if (paths.size() < constants::kMaxCachedEvaluationsCount) {
      return;
    }
    
    while (paths.size() > constants::kMaxCachedEvaluationsCount) {
      const auto& eldest = paths.back();
      FileHelper::DeleteFile(eldest);
      paths.pop_back();
    }
  }
};

}
