#pragma once

#include <utility>

#include "statsig_internal.h"
#include "initialize_request_args.h"
#include "initialize_response.hpp"
#include "data_types/json_parser.hpp"
#include "network_compat.hpp"
#include "unordered_map_util.hpp"

namespace statsig::internal {

template<typename T>
struct NetworkResult {
  T response;
  std::string raw;

  NetworkResult(T response, std::string raw)
      : response(std::move(response)), raw(std::move(raw)) {}
};

typedef std::optional<NetworkResult<data::InitializeResponse>> FetchValuesResult;

namespace {
std::unordered_map<int, bool> retryable_codes_{
    {408, true},
    {500, true},
    {502, true},
    {503, true},
    {504, true},
    {522, true},
    {524, true},
    {599, true},
};
}

class NetworkService {
  using string = std::string;

 public:
  explicit NetworkService(
      string &sdk_key,
      StatsigOptions &options
  ) : sdk_key_(sdk_key),
      options_(options),
      err_boundary_(ErrorBoundary(sdk_key)),
      session_id_(UUID::v4()) {}

  FetchValuesResult FetchValues(const StatsigUser &user) {
    FetchValuesResult result;

    err_boundary_.Capture(__func__, [this, &result, &user]() {
      result = FetchValuesImpl(user);
    });

    return result;
  }

  void SendEvents(const std::vector<StatsigEventInternal> &events) {
    auto args = LogEventRequestArgs{
        events,
        GetStatsigMetadata()
    };

    PostWithRetry(
        constants::kEndpointLogEvent,
        internal::Json::Serialize(args),
        constants::kLogEventRetryCount
    );
  }

 private:
  string &sdk_key_;
  StatsigOptions &options_;
  ErrorBoundary err_boundary_;
  string session_id_;
  StableID stable_id_;

  std::unordered_map<std::string, std::string> GetHeaders() {
    return {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion},
        {"Accept-Encoding", "gzip"}
    };
  }

  std::unordered_map<std::string, std::string> GetStatsigMetadata() {
    return {
        {"sdkType", constants::kSdkType},
        {"sdkVersion", constants::kSdkVersion},
        {"sessionID", session_id_},
        {"stableID", stable_id_.Get()}};
  }

  FetchValuesResult FetchValuesImpl(const StatsigUser &user) {
    auto args = internal::InitializeRequestArgs{
        "djb2",
        user,
        GetStatsigMetadata()
    };

    auto response = PostWithRetry(
        constants::kEndpointInitialize,
        internal::Json::Serialize(args),
        constants::kInitializeRetryCount
    );

    if (!response.has_value()) {
      return std::nullopt;
    }

    if (response->status < 200 || response->status >= 300) {
      return std::nullopt;
    }

    auto init_response = Json::Deserialize<data::InitializeResponse>(response->text);
    if (!init_response.has_value()) {
      return std::nullopt;
    }

    return NetworkResult(init_response.value(), response->text);
  }

  std::optional<HttpResponse> PostWithRetry(
      const string &endpoint,
      const std::string &body,
      const int max_attempts
  ) {
    for (int i = 0; i < max_attempts; i++) {
      auto response = Post(endpoint, body);

      if (response.status >= 200 && response.status < 300) {
        return response;
      }

      if (!MapContains(retryable_codes_, response.status)) {
        break;
      }
    }

    return std::nullopt;
  }

  HttpResponse Post(
      const string &endpoint,
      const std::string &body
  ) {
    auto api = options_.api.value_or(constants::kDefaultApi);

//    body["statsigMetadata"] = GetStatsigMetadata();

    return NetworkCompat::Post(
        api,
        endpoint,
        GetHeaders(),
        body
    );
  }
};

}
