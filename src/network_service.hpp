#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "constants.h"
#include "statsig_user.h"
#include "statsig_options.h"
#include "initialize_response.hpp"

using namespace httplib;
using namespace std::chrono;
using namespace statsig::constants;
using namespace nlohmann;

namespace statsig {

class NetworkService {
 public:
  explicit NetworkService(std::string sdk_key,
                          StatsigOptions *options)
      : sdk_key_(std::move(sdk_key)),
        options_(options) {};

  void FetchValues(StatsigUser *user) {
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    Client client(options_->api);
    Headers headers = {
        {kHeaderApiKey, sdk_key_},
        {kHeaderClientTime, std::to_string(now)},
//      {"STATSIG-SERVER-SESSION-ID", Utils::genUUIDString()},
        {kHeaderSdkType, kSdkType},
        {kHeaderSdkVersion, kSdkVersion},
    };

    auto res = client.Post(kEndpointInitialize, headers, "{}", kContentTypeJson);

    if (!res || res->status != 200) {
      return;
    }

    json body = json::parse(res->body);
//  body.template get<std::string>();
    auto p2 = body.template get<statsig::InitializeResponse>();

    if (res->status != 200) {
//    return nlohmann::json::parse("{}");
    }

  }

 private:
  std::string sdk_key_;
  StatsigOptions *options_;
};

}
