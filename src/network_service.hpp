#pragma once

#include <utility>

#include "statsig_internal.h"

namespace statsig {

template<typename T>
struct NetworkResult {
  T response;
  string raw;

  NetworkResult(T response, string raw)
      : response(std::move(response)), raw(std::move(raw)) {}
};

typedef std::optional<NetworkResult<InitializeResponse>> FetchValuesResult;

class NetworkService {
 public:
  explicit NetworkService(
      string &sdk_key
  ) : sdk_key_(sdk_key),
      err_boundary_(ErrorBoundary(sdk_key)),
      session_id_(UUID::v4()
      ) {}

  FetchValuesResult FetchValues(const StatsigUser &user) {
    FetchValuesResult result;

    err_boundary_.Capture(__func__, [this, &result, &user]() {
      result = FetchValuesImpl(user);
    });

    return result;
  }

  void SendEvents(const std::vector<StatsigEventInternal> &events) {
    PostWithRetry(
        kEndpointLogEvent,
        {{"events", events}},
        kLogEventRetryCount
    );

  }

 private:
  string &sdk_key_;
  ErrorBoundary err_boundary_;
  string session_id_;
  StableID stable_id_;

  httplib::Headers GetHeaders() {
    return {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", kSdkType},
        {"STATSIG-SDK-VERSION", kSdkVersion},
        {"Accept-Encoding", "gzip"}};
  }

  json GetStatsigMetadata() {
    return {
        {"sdkType", kSdkType},
        {"sdkVersion", kSdkVersion},
        {"sessionID", session_id_},
        {"stableID", stable_id_.Get()}};
  }

  FetchValuesResult FetchValuesImpl(const StatsigUser &user) {
    auto response = PostWithRetry(
        kEndpointInitialize,
        {
            {"hash", "djb2"},
            {"user", user}
        },
        kInitializeRetryCount
    );

    if (response->status != 200) {
      return std::nullopt;
    }

    auto initialize_res = json::parse(response->body)
        .template get<data::InitializeResponse>();

    return NetworkResult(initialize_res, response->body);
  }

  httplib::Result PostWithRetry(
      const string &endpoint,
      const std::unordered_map<string, json> &body,
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
      std::unordered_map<string, json> body
  ) {
    auto api = kDefaultApi;

    httplib::Client client(api);
    client.set_compress(endpoint == kEndpointLogEvent);

    body["statsigMetadata"] = GetStatsigMetadata();

    return client.Post(
        endpoint,
        GetHeaders(),
        json(body).dump(),
        kContentTypeJson);
  }
};

}
