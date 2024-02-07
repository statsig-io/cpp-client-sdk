#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <utility>

#include "constants.h"
#include "statsig_user.h"
#include "statsig_options.h"
#include "initialize_response.hpp"

using namespace httplib;
using namespace std;
using namespace std::chrono;
using namespace statsig::constants;
using namespace nlohmann;

namespace statsig {

class NetworkService {
 public:
  explicit NetworkService(string sdk_key, StatsigOptions *options) : sdk_key_(sdk_key) {
    this->options_ = options;
  }

  optional<data::InitializeResponse> FetchValues(StatsigUser *user) {
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto api = options_->api.value_or(kDefaultApi);

    Client client(api);
    Headers headers = {
        {"STATSIG-API-KEY", sdk_key_},
        {"STATSIG-CLIENT-TIME", std::to_string(now)},
//      {"STATSIG-SERVER-SESSION-ID", Utils::genUUIDString()},
        {"STATSIG-SDK-TYPE", kSdkType},
        {"STATSIG-SDK-VERSION", kSdkVersion},
    };

    json body;
    body["hash"] = "djb2";

    auto res = client.Post(
        kEndpointInitialize,
        headers,
        to_string(body),
        kContentTypeJson
    );

    if (!res || res->status != 200) {
      return std::nullopt;
    }

    return json::parse(res->body)
        .template get<data::InitializeResponse>();
  }

 private:
  std::string sdk_key_;
  StatsigOptions *options_;
};

}
