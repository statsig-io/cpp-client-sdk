#pragma once

#include <utility>

#include "statsig_internal.h"
#include "initialize_request_args.h"
#include "initialize_response.hpp"
#include "json_parser.hpp"
#include "statsig_compatibility/network/network_client.hpp"
#include "unordered_map_util.hpp"

namespace statsig::internal {

template<typename T>
struct NetworkResult {
  T response;
  std::string raw;

  NetworkResult(T response, std::string raw)
      : response(std::move(response)),
        raw(std::move(raw)) {}
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

  void FetchValues(
      const StatsigUser &user,
      const std::function<void(FetchValuesResult)> &callback
  ) {
    err_boundary_.Capture(__func__, [this, &callback, &user]() {
      FetchValuesImpl(user, callback);
    });
  }

  void SendEvents(const std::vector<StatsigEventInternal> &events) {
    auto args = LogEventRequestArgs{
        events,
        GetStatsigMetadata()
    };

    PostWithRetry(
        constants::kEndpointLogEvent,
        internal::Json::Serialize(args),
        constants::kLogEventRetryCount,
        [](std::optional<HttpResponse> response) {
          // todo: save to disk on failure
        }
    );
  }

 private:
  string &sdk_key_;
  StatsigOptions &options_;
  ErrorBoundary err_boundary_;
  string session_id_;
  StableID stable_id_;

  std::unordered_map<string, string> GetHeaders() {
    return {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion},
        {"Accept-Encoding", "gzip"}
    };
  }

  std::unordered_map<string, string> GetStatsigMetadata() {
    return {
        {"sdkType", constants::kSdkType},
        {"sdkVersion", constants::kSdkVersion},
        {"sessionID", session_id_},
        {"stableID", stable_id_.Get()}};
  }

  void FetchValuesImpl(
      const StatsigUser &user,
      const std::function<void(FetchValuesResult)> &callback
  ) {
    auto args = internal::InitializeRequestArgs{
        "djb2",
        user,
        GetStatsigMetadata()
    };

    PostWithRetry(
        constants::kEndpointInitialize,
        internal::Json::Serialize(args),
        constants::kInitializeRetryCount,
        [&callback](std::optional<HttpResponse> response) {
          if (!response.has_value()) {
            callback(std::nullopt);
            return;
          }

          if (response->status < 200 || response->status >= 300) {
            callback(std::nullopt);
            return;
          }

          auto init_response = Json::Deserialize<data::InitializeResponse>(response->text);
          if (!init_response.has_value()) {
            callback(std::nullopt);
            return;
          }

          callback(NetworkResult(init_response.value(), response->text));
        }
    );
  }

  void PostWithRetry(
      const string &endpoint,
      const std::string &body,
      const int max_attempts,
      const std::function<void(std::optional<HttpResponse>)> &callback
  ) {
    int attempt = 0;

    Post(endpoint, body, [&attempt, &callback](HttpResponse response) {
      if (response.status >= 200 && response.status < 300) {
        callback(response);
        return;
      }

      if (!MapContains(retryable_codes_, response.status)) {
        callback(std::nullopt);
        return;
      }
    });
//
//    for (int i = 0; i < max_attempts; i++) {
//
//
//      if (response.status >= 200 && response.status < 300) {
//        return response;
//      }
//
//      if (!MapContains(retryable_codes_, response.status)) {
//        break;
//      }
//    }

  }

  void Post(
      const string &endpoint,
      const std::string &body,
      const std::function<void(HttpResponse)> &callback
  ) {
    auto api = options_.api.value_or(constants::kDefaultApi);

    return NetworkClient::Post(
        api,
        endpoint,
        GetHeaders(),
        body,
        callback
    );
  }
};

}
