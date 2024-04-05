#pragma once

#include <utility>

#include "statsig_internal.h"
#include "initialize_request_args.h"
#include "initialize_response.hpp"
#include "data_types/json_parser.hpp"

namespace statsig::internal {

template<typename T>
struct NetworkResult {
  T response;
  std::string raw;

  NetworkResult(T response, std::string raw)
      : response(std::move(response)), raw(std::move(raw)) {}
};

typedef std::optional<NetworkResult<data::InitializeResponse>> FetchValuesResult;

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
    auto args = LogEventRequestArgs{events};

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

  httplib::Headers GetHeaders() {
    return {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", constants::kSdkType},
        {"STATSIG-SDK-VERSION", constants::kSdkVersion},
        {"Accept-Encoding", "gzip"}};
  }

  nlohmann::json GetStatsigMetadata() {
    return {
        {"sdkType", constants::kSdkType},
        {"sdkVersion", constants::kSdkVersion},
        {"sessionID", session_id_},
        {"stableID", stable_id_.Get()}};
  }

  FetchValuesResult FetchValuesImpl(const StatsigUser &user) {
    auto args = internal::InitializeRequestArgs{
        "djb2",
        user
    };

    auto response = PostWithRetry(
        constants::kEndpointInitialize,
        internal::Json::Serialize(args),
        constants::kInitializeRetryCount
    );

    if (response->status != 200) {
      return std::nullopt;
    }

    auto initialize_res = nlohmann::json::parse(response->body)
        .template get<data::InitializeResponse>();

    return NetworkResult(initialize_res, response->body);
  }

  httplib::Result PostWithRetry(
      const string &endpoint,
      const std::string &body,
      const int max_attempts
  ) {
    httplib::Result result;
    for (int i = 0; i < max_attempts; i++) {
      result = Post(endpoint, body);

      if (result->status == 200) {
        break;
      }
    }

    return result;
  }

  httplib::Result Post(
      const string &endpoint,
      const std::string &body
  ) {
    auto api = options_.api.value_or(constants::kDefaultApi);

    httplib::Client client(api);
    client.set_compress(endpoint == constants::kEndpointLogEvent);

//    body["statsigMetadata"] = GetStatsigMetadata();

    return client.Post(
        endpoint,
        GetHeaders(),
        body,
        constants::kContentTypeJson);
  }
};

}
