#pragma once

#include "constants.h"
#include "statsig_compat/filesystem/file_helper.hpp"
#include "../statsig_options.h"

namespace statsig::internal {

class File {
 public:
  using string = std::string;
  using FileHelper = statsig_compatibility::FileHelper;

  static void WriteToCache(
      const string &sdk_key,
      const StatsigOptions &options,
      const string &cache_key,
      const string &content,
      const std::function<void(bool)> &callback
  ) {
    FileHelper::EnsureCacheDirectoryExists(options);

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(options),  ToCompat(cache_key));
    FileHelper::WriteStringToFile(sdk_key, content, path, callback);
  }

  static void DeleteFromCache(
      const StatsigOptions &options,
      const string &cache_key
  ) {
    FileHelper::EnsureCacheDirectoryExists(options);

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(options), ToCompat(cache_key));
    FileHelper::DeleteFile(path);
  }

  static std::optional<string> ReadFromCache(
      const StatsigOptions &options,
      const string &cache_key
  ) {
    FileHelper::EnsureCacheDirectoryExists(options);

    auto path = FileHelper::CombinePath(FileHelper::GetCacheDir(options), ToCompat(cache_key));
    return FileHelper::ReadStringToFile(path);
  }

  static void RunCacheEviction(
      const StatsigOptions &options,
      const std::string &prefix
  ) {
    auto paths = FileHelper::GetCachePathsSortedYoungestFirst(options, prefix);

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
