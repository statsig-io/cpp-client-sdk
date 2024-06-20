#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "statsig_internal.h"
#include "initialize_request_args.h"
#include "initialize_response.hpp"
#include "json_parser.hpp"
#include "statsig_compat/network/network_client.hpp"
#include "statsig_compat/output_logger/log.hpp"
#include "statsig_compat/constants/constants.h"
#include "unordered_map_util.hpp"
#include "diagnostics.hpp"

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

class NetworkService: public std::enable_shared_from_this<NetworkService> {
  using string = std::string;
  using InitializeResponse = data::InitializeResponse;
  using Log = statsig_compatibility::Log;

 public:
  static std::shared_ptr<NetworkService> Create(
      std::string sdk_key,
      StatsigOptions &options
  ) {
    return std::shared_ptr<NetworkService>(new NetworkService(std::move(sdk_key), options));
  }

  void FetchValues(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(NetworkResult<InitializeResponse>)> &callback
  ) {
    const auto weak_self = weak_from_this();

    err_boundary_.Capture(__func__, [weak_self, callback, user, current]() {
      USE_REF_WITH_RETURN(weak_self, shared_self, SharedPointerLost);

      auto args = internal::InitializeRequestArgs{
          "djb2",
          user,
          shared_self->GetStatsigMetadata(),
      };

      const auto cache = DeserializeCacheValueIfValidFor204(user, current);
      if (cache.has_value() && cache->has_updates) {
        args.since_time = cache->time;
      }

      auto serialized = internal::Json::Serialize(args);
      if (serialized.code == Ok && serialized.value.has_value()) {
        shared_self->PostWithRetry(
            constants::kEndpointInitialize,
            serialized.value.value(),
            constants::kInitializeRetryCount,
            HandleFetchValuesResponse(shared_self->diagnostics_, cache, callback)
        );
      }

      return serialized.code;
    });
  }

  void SendEvents(
      const std::vector<StatsigEventInternal> &events,
      const std::function<void(NetworkResult<bool>)> &callback) {
    auto args = LogEventRequestArgs{
        events,
        GetStatsigMetadata()};

    auto serialized = Json::Serialize(args);
    if (serialized.code != Ok || !serialized.value.has_value()) {
      return;
    }

    PostWithRetry(
        constants::kEndpointLogEvent,
        serialized.value.value(),
        constants::kLogEventRetryCount,
        [callback](std::optional<HttpResponse> response) {
          if (!HasSuccessCode(response)) {
            callback({{NetworkFailureBadStatusCode, std::nullopt, GetStatusCodeErr(response)}});
          } else {
            auto res = Json::Deserialize<LogEventResponse>(response->text);
            callback({{res.code, res.value.has_value() && res.value->success}});
          }
        });
  }

 private:
  string sdk_key_;
  StatsigOptions options_;
  std::shared_ptr<Diagnostics> diagnostics_;
  ErrorBoundary err_boundary_;
  string session_id_;
  StableID stable_id_;

  static bool HasSuccessCode(const std::optional<HttpResponse> &response) {
    return response.has_value() && (response->status >= 200 && response->status < 300);
  }

  static std::optional<std::unordered_map<string, string>>
  GetStatusCodeErr(const std::optional<HttpResponse> &response) {
    if (!response.has_value() || response->status < 500) {
      return std::nullopt;
    }

    return std::unordered_map<string, string>{
        {constants::kBadNetworkErr, constants::kBadNetworkErr + std::to_string(response->status)}};
  }

  explicit NetworkService(
      string sdk_key,
      StatsigOptions &options
  )
      : sdk_key_(sdk_key),
        options_(options),
        diagnostics_(Diagnostics::Get(sdk_key)),
        err_boundary_(ErrorBoundary(sdk_key)),
        session_id_(UUID::v4()),
        stable_id_(sdk_key, options) {}

  std::unordered_map<string, string> GetHeaders() {
    std::unordered_map<string, string> headers(GetDefaultPlatformHeaders());
    headers.merge(std::unordered_map<string, string>{
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", statsig_compatibility::constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion}});
    return headers;
  }

  std::unordered_map<string, string> GetStatsigMetadata() {
    return {
        {"sdkType", statsig_compatibility::constants::kSdkType},
        {"sdkVersion", constants::kSdkVersion},
        {"sessionID", session_id_},
        {"stableID", stable_id_.Get()}};
  }

  static std::optional<InitializeResponse> DeserializeCacheValueIfValidFor204(
      const StatsigUser &current_user,
      const std::optional<DataAdapterResult> &cached_result
  ) {
    if (!cached_result.has_value()) {
      return std::nullopt;
    }

    const auto current_user_hash = GetFullUserHash(current_user);
    const bool is_match = current_user_hash == cached_result->full_user_hash;
    if (!is_match) {
      return std::nullopt;
    }

    return Json::Deserialize<InitializeResponse>(cached_result->data).value;
  }

  static std::function<void(std::optional<HttpResponse> response)>
  HandleFetchValuesResponse(
      const std::shared_ptr<Diagnostics> &diagnostics,
      const std::optional<InitializeResponse> &cache,
      const std::function<void(NetworkResult<InitializeResponse>)> &callback
  ) {
    return [diagnostics, callback, cache](auto response) {
      if (!HasSuccessCode(response)) {
        callback({{NetworkFailureBadStatusCode, std::nullopt, GetStatusCodeErr(response)}});
        return;
      }

      diagnostics->Mark(markers::ProcessStart()); // ended in StatsigEvaluationsDataAdapter

      if (response->status == 204) {
        callback({{Ok, InitializeResponse{}}});
        return;
      }

      auto data = Json::Deserialize<InitializeResponse>(response->text);
      if (data.code != Ok) {
        diagnostics->Mark(markers::ProcessEnd(false));
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
    const auto is_initialize = endpoint == constants::kEndpointInitialize;
    if (is_initialize) { diagnostics_->Mark(markers::NetworkStart(attempt)); }

    const auto start = Time::now();
    const auto weak_self = weak_from_this();
    Post(endpoint, body, [weak_self, is_initialize, start, endpoint, body, max_attempts, attempt, callback](HttpResponse response) {
      USE_REF(weak_self, shared_self);

      const auto end = Time::now();
      Log::Debug("Request to " + endpoint + " completed. Status " + std::to_string(response.status) + ". Time "
                     + std::to_string(end - start) + "ms");

      if (is_initialize) {
        shared_self->diagnostics_->Mark(markers::NetworkEnd(
            attempt,
            response.status,
            response.text,
            response.sdk_region,
            response.error
        ));
      }

      if (HasSuccessCode(response)) {
        callback(response);
        return;
      }

      if (!MapContains(retryable_codes_, response.status) || attempt >=
          max_attempts) {
        callback(response);
        return;
      }

      shared_self->PostWithRetry(endpoint, body, max_attempts, callback, attempt + 1);
    });
  }

  void Post(
      const string &endpoint,
      const std::string &body,
      const std::function<void(HttpResponse)> &callback) {
    std::string api = FromCompat(options_.api.value_or(constants::kDefaultApi));
    Log::Debug("Making post request to " + api + endpoint);

    NetworkClient::Post(
        {
            api,
            endpoint,
            GetHeaders(),
            body,
        },
        callback);
  }
};

}
