#pragma once

#include <utility>
#include <execinfo.h>

#include "constants.h"

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

      nlohmann::json body = {
          {"exception", error},
          {"info", GetStackTrace()},
          {"tag", tag}
      };

      httplib::Headers headers = {
          {"STATSIG-API-KEY", sdk_key_},
          {"STATSIG-SDK-TYPE", constants::kSdkType},
          {"STATSIG-SDK-VERSION", constants::kSdkVersion}
      };

      httplib::Client(eb_api)
          .Post(
              "/v1/sdk_exception",
              headers,
              nlohmann::json(body).dump(),
              constants::kContentTypeJson
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