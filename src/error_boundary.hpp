#pragma once

#include "statsig.h"

namespace statsig::internal {

struct ErrorBoundaryRequestArgs {
  std::string tag;
  std::string exception;
  std::vector<std::string> info;
};

}

#include "json_parser.hpp"
#include "constants.h"
#include "compat/network/network_client.hpp"
#include "compat/constants/constants.h"
#include <set>

#ifndef STATSIG_DISABLE_EXCEPTIONS
#ifdef __unix__
#include <execinfo.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <stdexcept>
#endif
#endif // STATSIG_DISABLE_EXCEPTIONS

namespace statsig::internal {

class ErrorBoundary {
public:
  using string = std::string;

  explicit ErrorBoundary(string& sdk_key)
    : sdk_key_(sdk_key) {}

  inline static string eb_api = constants::kDefaultApi;

  StatsigResultCode Enforce(
      const StatsigResultCode& failure_result,
      const std::function<bool()>& verification
      ) {
    if (verification()) {
      return Ok;
    }

    return failure_result;
  }

  void Capture(
      const string& tag,
      const std::function<void()>& task
      ) {
#ifndef STATSIG_DISABLE_EXCEPTIONS
    try {
      task();
    } catch (const std::exception &error) {
      try {
        std::cerr << "[Statsig]: An unexpected exception occurred. " << error.
            what() << std::endl;
        LogError(tag, error.what());
      } catch (std::exception &) {
        // noop
      }
    }
#else // STATSIG_DISABLE_EXCEPTIONS
    task();
#endif
  }

private:
  string& sdk_key_;
  std::set<string> seen_;

  static std::vector<string> GetStackTrace() {
    std::vector<string> trace;

#ifndef STATSIG_DISABLE_EXCEPTIONS
#ifdef __unix__
    const int maxFrames = 128;
    void* stackTrace[maxFrames];
    int stackSize = backtrace(stackTrace, maxFrames);
    char** symbols = backtrace_symbols(stackTrace, stackSize);

    if (symbols == nullptr) {
      return trace;
    }

    for (int i = 0; i < stackSize; ++i) {
      trace.emplace_back(symbols[i]);
    }

    free(symbols);
#endif // __unix__
#endif // STATSIG_DISABLE_EXCEPTIONS

    return trace;
  }

  void LogError(
      const string& tag,
      const string& error
      ) {

    if (seen_.find(error) != seen_.end()) {
      return;
    }

    seen_.insert(error);

    ErrorBoundaryRequestArgs body{error, tag, GetStackTrace()};

    std::unordered_map<string, string> headers{
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-SDK-TYPE", statsig_compatibility::constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion}
    };

    NetworkClient::Post(
        {
            eb_api,
            "/v1/sdk_exception",
            headers,
            Json::Serialize(body)
        },
        [](std::optional<HttpResponse> response) {});
  }

};

}
