#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <utility>

#include "constants.h"
#include "statsig_user.h"
#include "statsig_options.h"
#include "initialize_response.hpp"
#include "time.hpp"
#include "uuid.hpp"
#include "statsig_event_internal.hpp"
#include "stable_id.hpp"

using namespace httplib;
using namespace std;
using namespace std::chrono;
using namespace statsig::constants;
using namespace nlohmann;

namespace statsig {

template<typename T>
struct NetworkResult {
  T response;
  string raw;

  NetworkResult(T response, string raw)
      : response(std::move(response)), raw(std::move(raw)) {}
};

class NetworkService {
 public:
  explicit NetworkService(
      string &sdk_key, StatsigOptions &options) : sdk_key_(sdk_key), options_(options), session_id_(UUID::v4()) {}

  optional<NetworkResult<InitializeResponse>> FetchValues(StatsigUser &user) {
    auto response = PostWithRetry(
        kEndpointInitialize,
        {
            {"hash", "djb2"},
            {"user", user}
        },
        3
    );

    if (response->status != 200) {
      return nullopt;
    }

    auto actual = json::parse(response->body).template get<data::InitializeResponse>();
    return NetworkResult(actual, response->body);
  }

  void SendEvents(const vector<StatsigEventInternal> &events) {
    Post(
        kEndpointLogEvent,
        {{"events", events}});
  }

 private:
  string &sdk_key_;
  StatsigOptions &options_;
  string session_id_;
  StableID stable_id_;

  Headers GetHeaders() {
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

  httplib::Result PostWithRetry(
      const string &endpoint,
      const unordered_map<string, json> &body,
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
      unordered_map<string, json> body
  ) {
    auto api = options_.api.value_or(kDefaultApi);

    Client client(api);
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
