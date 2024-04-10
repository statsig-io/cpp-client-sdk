#pragma once


namespace statsig::internal {

struct ErrorBoundaryRequestArgs {
  std::string tag;
  std::string exception;
  std::vector<std::string> info;
};

}


#include "json_parser.hpp"
#include "constants.h"
#include "statsig_compatibility/network/network_client.hpp"
#include <set>

namespace statsig::internal {

class ErrorBoundary {
public:
  using string = std::string;

  explicit ErrorBoundary(string& sdk_key)
    : sdk_key_(sdk_key) {}

  inline static string eb_api = constants::kDefaultApi;

  void Capture(
      const string& tag,
      const std::function<void()>& task
      ) {
#if PLATFORM_EXCEPTIONS_DISABLED
    task();
#else
    try {
      task();
    } catch (const std::exception& error) {
      std::cerr << "[Statsig]: An unexpected exception occurred. " << error.
          what() << std::endl;
      LogError(tag, error.what());
    }
#endif
  }

  void LogError(
      const string& tag,
      const string& error
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

      NetworkClient::Post(
          eb_api,
          "/v1/sdk_exception",
          headers,
          Json::Serialize(body),
          [](std::optional<HttpResponse> response) {}
          );
    } catch (std::exception& _) {
      // noop
    }
  }

private:
  string& sdk_key_;
  std::set<string> seen_;

  static std::vector<string> GetStackTrace() {
    const int maxFrames = 128;
    void* stackTrace[maxFrames];
    int stackSize = backtrace(stackTrace, maxFrames);
    char** symbols = backtrace_symbols(stackTrace, stackSize);

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
