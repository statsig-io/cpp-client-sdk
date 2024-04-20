#pragma once

#include "statsig.h"

namespace statsig::internal {

struct ErrorBoundaryRequestArgs {
  std::string tag;
  std::string exception;
  std::vector<std::string> info;
};

}

#include <set>
#include <unordered_map>

#include "json_parser.hpp"
#include "constants.h"
#include "compat/network/network_client.hpp"
#include "compat/constants/constants.h"
#include "unordered_map_util.hpp"

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

  explicit ErrorBoundary(string &sdk_key)
      : sdk_key_(sdk_key) {}

  inline static string eb_api = constants::kDefaultApi;

  void ReportBadResult(
      const string &tag,
      const StatsigResultCode &code,
      const std::optional<std::unordered_map<std::string, std::string>> &extra
  ) {
    if (code == Ok || code == ClientUninitialized || code == InvalidSdkKey) {
      return;
    }

    if (code == NetworkFailureBadStatusCode && !MapGetOrNull(extra, constants::kBadNetworkErr).has_value()) {
      return;
    }

    LogError(tag, ResultCodeToString(code, extra));
  }

  StatsigResultCode Capture(
      const string &tag,
      const std::function<StatsigResultCode()> &task
  ) {
    StatsigResultCode code;
#ifndef STATSIG_DISABLE_EXCEPTIONS
    try {
      code = task();
    } catch (const std::exception &error) {
      try {
        std::cerr << "[Statsig]: An unexpected exception occurred. " << error.
            what() << std::endl;
        LogError(tag, error.what());
      } catch (std::exception &) {
        // noop
      }
      return UnexpectedError;
    }
#else
    code = task();
#endif
    ReportBadResult(tag, code, std::nullopt);
    return code;
  }

 private:
  string &sdk_key_;
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

  static std::string ResultCodeToString(
      StatsigResultCode code,
      const std::optional<std::unordered_map<std::string, std::string>> &extra
  ) {
    auto custom = MapGetOrNull(extra, constants::kBadNetworkErr);
    if (custom.has_value()) {
      return custom.value();
    }

    switch (code) {
      case JsonFailureInitializeRequestArgs: return "JsonFailureInitializeRequestArgs";
      case JsonFailureInitializeResponse: return "JsonFailureInitializeResponse";
      case JsonFailureLogEventResponse: return "JsonFailureLogEventResponse";
      case JsonFailureNoDeserializerFound: return "JsonFailureNoDeserializerFound";
      case JsonFailureStatsigUser: return "JsonFailureStatsigUser";
      case Ok:return "Ok";
      case UnexpectedError:return "UnexpectedError";
      case InvalidSdkKey: return "InvalidSdkKey";
      case JsonFailureDataAdapterResult: return "JsonFailureDataAdapterResult";
      case JsonFailureRetryableEventPayload: return "JsonFailureRetryableEventPayload";
      case NetworkFailureBadStatusCode: return "NetworkFailureBadStatusCode";
      case ClientUninitialized: return "ClientUninitialized";
    }

    return std::to_string(code);
  }

  void LogError(
      const string &tag,
      const string &error
  ) {
    if (seen_.find(error) != seen_.end()) {
      return;
    }

    seen_.insert(error);

    ErrorBoundaryRequestArgs body{tag, error, GetStackTrace()};

    std::unordered_map<string, string> headers{
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-SDK-TYPE", statsig_compatibility::constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion}
    };

#ifndef STATSIG_DISABLE_EXCEPTIONS
    try {
#endif
      NetworkClient::Post(
          {
              eb_api,
              "/v1/sdk_exception",
              headers,
              Json::Serialize(body)
          },
          [](auto) {}
      );
#ifndef STATSIG_DISABLE_EXCEPTIONS
    } catch (std::exception &) {
      // noop
    }
#endif
  }

};

}
