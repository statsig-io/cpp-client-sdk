#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace statsig::internal {

struct ErrorBoundaryRequestArgs {
  std::string tag;
  std::string exception;
  std::vector<std::string> info;
  std::optional<std::unordered_map<std::string, std::string>> extra;
};

}