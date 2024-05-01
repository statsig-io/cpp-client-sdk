#pragma once

#include "statsig_compat/filesystem/file_helper.hpp"

namespace statsig::internal {

class File {
 public:
  using string = std::string;
  using FileHelper = statsig_compatibility::FileHelper;

  static void WriteToCache(
      const string &key,
      const string &content,
      const std::function<void(bool)> &callback
  ) {
    FileHelper::EnsureCacheDirectoryExists();

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    FileHelper::WriteStringToFile(content, path, callback);
  }

  static void DeleteFromCache(const string &key) {
    FileHelper::EnsureCacheDirectoryExists();
    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    FileHelper::DeleteFile(path);
  }

  static std::optional<string> ReadFromCache(const string &key) {
    FileHelper::EnsureCacheDirectoryExists();

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(), key);
    return FileHelper::ReadStringToFile(path);
  }

  static void RunCacheEviction(const std::string &prefix) {
    auto paths = FileHelper::GetCachePathsSortedYoungestFirst(prefix);

    if (paths.size() < constants::kMaxCachedEvaluationsCount) {
      return;
    }

    while (paths.size() >= constants::kMaxCachedEvaluationsCount) {
      const auto &eldest = paths.back();
      FileHelper::DeleteFile(eldest);
      paths.pop_back();
    }
  }
};

}
