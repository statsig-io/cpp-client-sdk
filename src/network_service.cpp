#include "network_service.h"
#include <httplib.h>

namespace statsig {
    nlohmann::json NetworkService::fetch_for_user(StatsigUser *user) {
      httplib::Client client(options_->api);
      httplib::Headers headers = {
          {"STATSIG-API-KEY", sdk_key_},
//          {"STATSIG-CLIENT-TIME", std::to_string(currentTime)},
//          {"STATSIG-SERVER-SESSION-ID", Utils::genUUIDString()},
//          {"STATSIG-SDK-TYPE", StatsigMetadata.sdkType},
//          {"STATSIG-SDK-VERSION", StatsigMetadata.sdkVersion},
      };

      httplib::Result res = client.Post("/initialize", headers, "{}", "application/json");

      if (res->status != 200) {
        return nlohmann::json::parse("{}");
      }

      return nlohmann::json::parse(res->body);
    }
}

