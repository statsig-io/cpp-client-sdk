#pragma once

#include <set>
#include <unordered_map>
#include <utility>

#include "constants.h"
#include "error_boundary_request_args.h"
#include "json_parser.hpp"
#include "shareable.hpp"
#include "statsig_compat/async/async_helper.hpp"
#include "statsig_compat/constants/constants.h"
#include "statsig_compat/defines/module_definitions.h"
#include "statsig_compat/network/network_client.hpp"
#include "statsig_compat/output_logger/log.hpp"
#include "unordered_map_util.hpp"

#ifndef STATSIG_UNREAL_PLUGIN
#ifdef __unix__
#include <execinfo.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <stdexcept>
#endif
#endif // STATSIG_UNREAL_PLUGIN

namespace statsig::internal {

class ErrorBoundary {
 public:
  using string = std::string;

  explicit ErrorBoundary(const string &sdk_key)
      : sdk_key_(sdk_key) {}

  inline static string eb_api = constants::kDefaultApi;

  static std::shared_ptr<ErrorBoundary> Get(const string &sdk_key) {
    auto instance = shareable_.Get(sdk_key);
    if (instance != nullptr) {
      return instance;
    }

    std::shared_ptr<ErrorBoundary> new_instance(new ErrorBoundary(sdk_key));
    shareable_.Add(sdk_key, new_instance);
    return new_instance;
  }

  static void Shutdown(const string &sdk_key) {
    shareable_.Remove(sdk_key);
  }

  bool HandleBadResult(
      const string &tag,
      const StatsigResultCode &code,
      const std::optional<std::unordered_map<std::string, std::string>> &extra
  ) {
    if (code == Ok || code == ClientUninitialized || code == InvalidSdkKey || code == SharedPointerLost) {
      return false;
    }

    if (code == ShutdownFailureDanglingThreads && !MapGetOrNull(
        extra, constants::kShutdownTimeoutExtra).has_value()) {
      return false;
    }

    if (code == NetworkFailureBadStatusCode && !MapGetOrNull(
        extra, constants::kBadNetworkErr).has_value()) {
      return false;
    }

    LogError(tag, ErrorFromResultCode(code, extra), extra);

    return true;
  }

  StatsigResultCode Capture(
      const string &tag,
      const std::function<StatsigResultCode()> &task,
      const std::optional<std::function<void(StatsigResultCode)>> &recover =
      std::nullopt
  ) {
    StatsigResultCode code;
#ifndef STATSIG_UNREAL_PLUGIN
    try {
      code = task();
    }
    catch (const std::exception &error) {
      try {
        LogError(tag, error.what(), std::nullopt);
      }
      catch (std::exception &) {
        // noop
      }
      if (recover.has_value()) {
        recover.value()(UnexpectedError);
      }
      return UnexpectedError;
    }
#else
    code = task();
#endif
    if (HandleBadResult(tag, code, std::nullopt) && recover.has_value()) {
      recover.value()(code);
    }
    return code;
  }

 private:
  STATSIG_EXPORT static Shareable<ErrorBoundary> shareable_;

  string sdk_key_;
  std::set<string> seen_;

  static std::vector<string> GetStackTrace() {
    std::vector<string> trace;

#ifndef STATSIG_UNREAL_PLUGIN
#ifdef __unix__
    const int maxFrames = 128;
    void *stackTrace[maxFrames];
    int stackSize = backtrace(stackTrace, maxFrames);
    char **symbols = backtrace_symbols(stackTrace, stackSize);

    if (symbols == nullptr)
    {
      return trace;
    }

    for (int i = 0; i < stackSize; ++i)
    {
      trace.emplace_back(symbols[i]);
    }

    free(symbols);
#endif // __unix__
#endif // STATSIG_UNREAL_PLUGIN
    return trace;
  }

  static std::string ErrorFromResultCode(
      StatsigResultCode code,
      const std::optional<std::unordered_map<std::string, std::string>> &
      extra) {
    auto custom = MapGetOrNull(extra, constants::kBadNetworkErr);
    if (custom.has_value()) {
      return custom.value();
    }

    return ResultCodeToString(code);
  }

  void LogError(
      const string &tag,
      const string &error,
      const std::optional<std::unordered_map<std::string, std::string>> &extra
  ) {
    if (seen_.find(error) != seen_.end()) {
      return;
    }

    statsig_compatibility::Log::Error(
        "An unexpected exception occurred. " + error);

    seen_.insert(error);

    ErrorBoundaryRequestArgs body{tag, error, GetStackTrace(), extra};

    std::unordered_map<string, string> headers{
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-SDK-TYPE", statsig_compatibility::constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion}};

#ifndef STATSIG_UNREAL_PLUGIN
    try {
#endif
      auto serialized = Json::Serialize(body);
      if (serialized.code != Ok || !serialized.value.has_value()) {
        return;
      }

      statsig_compatibility::AsyncHelper::Get(sdk_key_)->RunInBackground(
          [headers, serialized]() {
            NetworkClient::Post(
                {eb_api,
                 "/v1/sdk_exception",
                 headers,
                 serialized.value.value()
                },
                [](auto) {});
          });

#ifndef STATSIG_UNREAL_PLUGIN
    }
    catch (std::exception &) {
      // noop
    }
#endif
  }
};

}
