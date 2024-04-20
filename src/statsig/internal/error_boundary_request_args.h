#pragma once

#include <string>
#include <vector>

namespace statsig::internal {

struct ErrorBoundaryRequestArgs {
  std::string tag;
  std::string exception;
  std::vector<std::string> info;
};

}