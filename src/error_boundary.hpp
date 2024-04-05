#pragma once

#include <utility>
#include <execinfo.h>

namespace statsig::internal {

struct ErrorBoundaryRequestArgs {
  std::string tag;
  std::string exception;
  std::vector<std::string> info;
};

}

#include "data_types/json_parser.hpp"
#include "constants.h"
#include "network_compat.hpp"
#include <set>

namespace statsig::internal {

class ErrorBoundary {
 public:
  using string = std::string;

  explicit ErrorBoundary(string &sdk_key) : sdk_key_(sdk_key) {}

  inline static string eb_api = constants::kDefaultApi;

  void Capture(
      const string &tag,
      const std::function<void()> &task
  ) {
    try {
      task();
    } catch (const std::exception &error) {
      std::cerr << "[Statsig]: An unexpected exception occurred. " << error.what() << std::endl;
      LogError(tag, error.what());
    }
  }

 private:
  string &sdk_key_;
  std::set<string> seen_;

  void LogError(
      const string &tag,
      const string &error
  ) {
    try {
      if (seen_.find(error) != seen_.end()) {
        return;
      }

      seen_.insert(error);

      ErrorBoundaryRequestArgs body{error, tag, GetStackTrace()};

      std::unordered_map<string, string> headers{
          {"STATSIG-API-KEY", sdk_key_},
          {"STATSIG-SDK-TYPE", constants::kSdkType},
          {"STATSIG-SDK-VERSION", constants::kSdkVersion}
      };

      NetworkCompat::Post(
          eb_api,
          "/v1/sdk_exception",
          headers,
          Json::Serialize(body)
      );
    } catch (std::exception &_) {
      // noop
    }
  }

  static std::vector<string> GetStackTrace() {
    const int maxFrames = 128;
    void *stackTrace[maxFrames];
    int stackSize = backtrace(stackTrace, maxFrames);
    char **symbols = backtrace_symbols(stackTrace, stackSize);

    std::vector<string> trace;

    if (symbols == nullptr) {
      return trace;
    }

    for (int i = 0; i < stackSize; ++i) {
      trace.emplace_back(symbols[i]);
    }

    free(symbols);
    return trace;
  }

};

}