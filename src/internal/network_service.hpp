#pragma once

#include <utility>

#include "statsig_internal.h"
#include "initialize_request_args.h"
#include "initialize_response.hpp"
#include "json_parser.hpp"
#include "compat/network/network_client.hpp"
#include "compat/constants/constants.h"
#include "unordered_map_util.hpp"

namespace statsig::internal {

template<typename T>
struct NetworkResult : public StatsigResult<T> {
  std::string raw;
};

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
  using InitializeResponse = data::InitializeResponse;

 public:
  explicit NetworkService(
      string sdk_key,
      StatsigOptions &options
  )
      : sdk_key_(sdk_key),
        options_(options),
        err_boundary_(ErrorBoundary(sdk_key)),
        session_id_(UUID::v4()) {}

  void FetchValues(
      const StatsigUser &user,
      const std::optional<std::string> &current,
      const std::function<void(NetworkResult<InitializeResponse>)> &callback
  ) {
    err_boundary_.Capture(__func__, [this, callback, &user, &current]() {
      auto cache = current.has_value()
                   ? Json::Deserialize<InitializeResponse>(current.value()).value
                   : std::nullopt;

      auto args = internal::InitializeRequestArgs{
          "djb2",
          user,
          GetStatsigMetadata(),
      };

      if (cache.has_value() && cache->has_updates) {
        args.since_time = cache->time;
      }

      PostWithRetry(
          constants::kEndpointInitialize,
          internal::Json::Serialize(args),
          constants::kInitializeRetryCount,
          HandleFetchValuesResponse(cache, callback)
      );

      return Ok;
    });
  }

  void SendEvents(
      const std::vector<StatsigEventInternal> &events,
      const std::function<void(NetworkResult<bool>)> &callback
  ) {
    auto args = LogEventRequestArgs{
        events,
        GetStatsigMetadata()
    };

    PostWithRetry(
        constants::kEndpointLogEvent,
        Json::Serialize(args),
        constants::kLogEventRetryCount,
        [callback](std::optional<HttpResponse> response) {
          if (!HasSuccessCode(response)) {
            callback({{NetworkFailureBadStatusCode, std::nullopt, GetStatusCodeErr(response)}});
          } else {
            auto res = Json::Deserialize<LogEventResponse>(response->text);
            callback({{res.code, res.value.has_value() && res.value->success}});
          }
        }
    );
  }

 private:
  string sdk_key_;
  StatsigOptions &options_;
  ErrorBoundary err_boundary_;
  string session_id_;
  StableID stable_id_;

  static bool HasSuccessCode(const std::optional<HttpResponse> &response) {
    return response.has_value() && (
        response->status >= 200 && response->status < 300);
  }

  static std::optional<std::unordered_map<string, string>>
  GetStatusCodeErr(const std::optional<HttpResponse> &response) {
    if (!response.has_value() || response->status < 500) {
      return std::nullopt;
    }

    return std::unordered_map<string, string>{
        {constants::kBadNetworkErr, constants::kBadNetworkErr + std::to_string(response->status)}
    };
  }

  std::unordered_map<string, string> GetHeaders() {
    return {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", statsig_compatibility::constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion},
        {"Accept-Encoding", "gzip"}
    };
  }

  std::unordered_map<string, string> GetStatsigMetadata() {
    return {
        {"sdkType", statsig_compatibility::constants::kSdkType},
        {"sdkVersion", constants::kSdkVersion},
        {"sessionID", session_id_},
        {"stableID", stable_id_.Get()}};
  }

  static std::function<void(std::optional<HttpResponse> response)>
  HandleFetchValuesResponse(
      const std::optional<InitializeResponse> &cache,
      const std::function<void(NetworkResult<InitializeResponse>)> &callback
  ) {
    return [callback, cache](auto response) {
      if (!HasSuccessCode(response)) {
        callback({{NetworkFailureBadStatusCode, std::nullopt, GetStatusCodeErr(response)}});
        return;
      }

      if (response->status == 204) {
        callback({{Ok, {}}});
        return;
      }

      auto data = Json::Deserialize<InitializeResponse>(response->text);
      if (data.code != Ok) {
        callback({{data.code}});
        return;
      }

      NetworkResult<InitializeResponse> result = {{Ok}};
      result.raw = response->text;
      result.value = data.value;
      callback(result);
    };
  }

  void PostWithRetry(
      const string &endpoint,
      const std::string &body,
      const int max_attempts,
      const std::function<void(std::optional<HttpResponse>)> &callback,
      const int attempt = 1
  ) {
    Post(endpoint, body, [&, endpoint, body, max_attempts, attempt, callback]
        (HttpResponse response) {
      if (HasSuccessCode(response)) {
        callback(response);
        return;
      }

      if (!MapContains(retryable_codes_, response.status) || attempt >=
          max_attempts) {
        callback(response);
        return;
      }

      PostWithRetry(endpoint, body, max_attempts, callback, attempt + 1);
    });
  }

  void Post(
      const string &endpoint,
      const std::string &body,
      const std::function<void(HttpResponse)> &callback
  ) {
    auto api = options_.api.value_or(constants::kDefaultApi);

    NetworkClient::Post(
        {
            api,
            endpoint,
            GetHeaders(),
            body,
        },
        callback
    );
  }
};

}
