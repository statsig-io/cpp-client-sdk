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

using namespace httplib;
using namespace std;
using namespace std::chrono;
using namespace statsig::constants;
using namespace nlohmann;

namespace statsig {

class NetworkService {
 public:
  explicit NetworkService(
      string &sdk_key, StatsigOptions &options
  ) : sdk_key_(sdk_key), options_(options), session_id_(UUID::v4()) {}

  optional<data::InitializeResponse> FetchValues(StatsigUser &user) {
    auto response = Post(
        kEndpointInitialize,
        {
            {"hash", "djb2"},
            {"user", user}
        }
    );

    return response.is_null()
           ? nullopt
           : optional(response.template get<data::InitializeResponse>());
  }

  void SendEvents(const vector<StatsigEventInternal> &events) {
    Post(
        kEndpointLogEvent,
        {{"events", events}}
    );
  }

 private:
  string &sdk_key_;
  StatsigOptions &options_;
  string session_id_;

  Headers GetHeaders() {
    return {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(Time::now())},
        {"STATSIG-SERVER-SESSION-ID", session_id_},
        {"STATSIG-SDK-TYPE", kSdkType},
        {"STATSIG-SDK-VERSION", kSdkVersion},
        {"Accept-Encoding", "gzip"}
    };
  }

  json Post(const string &endpoint, const unordered_map<string, json> &body) {
    auto api = options_.api.value_or(kDefaultApi);

    Client client(api);
    client.set_compress(true);

    auto res = client.Post(
        endpoint,
        GetHeaders(),
        json(body).dump(),
        kContentTypeJson
    );

    if (!res || res->status != 200) {
      return {};
    }

    return json::parse(res->body);
  }
};

}
