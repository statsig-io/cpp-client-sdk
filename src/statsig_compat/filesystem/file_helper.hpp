#pragma once

#include <string>
#include <optional>

namespace statsig_compatibility {

class FileHelper {
 public:
  static std::string GetCacheDir() {
    return "";
  }

  static void WriteStringToFile(
      const std::string &content,
      const std::string &path
  ) {
  }

  static std::string CombinePath(const std::string &left,
                                 const std::string &right) {
    return "";
  }

  static std::optional<std::string> ReadStringToFile(const std::string &path) {
    return std::nullopt;
  }

  static void EnsureCacheDirectoryExists() {
  }

  static void DeleteFile(const std::string &path) {

  }

  static std::vector<std::string> GetCachePathsSortedYoungestFirst(
      std::string prefix) {
    std::vector<std::string> result;
    return result;
  }
};

}
