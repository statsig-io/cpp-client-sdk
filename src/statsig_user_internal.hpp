#pragma once

#include "hashing.hpp"
#include "macros.hpp"

namespace statsig {

using json = nlohmann::json;
using string = std::string;

//void to_json(json &j, const StatsigUser &u) {
//  j = json{
//      {"userID", u.user_id},
//  };
//
//  if (!u.custom_ids.empty()) {
//    j["customIDs"] = u.custom_ids;
//  }
//
//  OPT_TO_JSON(j, "email", u.email);
//  OPT_TO_JSON(j, "ip", u.ip);
//  OPT_TO_JSON(j, "userAgent", u.user_agent);
//  OPT_TO_JSON(j, "country", u.country);
//  OPT_TO_JSON(j, "locale", u.locale);
//  OPT_TO_JSON(j, "appVersion", u.app_version);
//  OPT_TO_JSON(j, "custom", u.custom);
//  OPT_TO_JSON(j, "privateAttributes", u.private_attributes);
//}
//
//void from_json(const json &j, StatsigUser &u) {
//  u.user_id = j.value("userID", "");
//  u.custom_ids = j.value("customIDs", std::unordered_map<string, string>());
//
//  OPT_STR_FROM_JSON(j, "email", u.email);
//  OPT_STR_FROM_JSON(j, "ip", u.ip);
//  OPT_STR_FROM_JSON(j, "userAgent", u.user_agent);
//  OPT_STR_FROM_JSON(j, "country", u.country);
//  OPT_STR_FROM_JSON(j, "locale", u.locale);
//  OPT_STR_FROM_JSON(j, "appVersion", u.app_version);
//  OPT_STR_MAP_FROM_JSON(j, "custom", u.custom);
//  OPT_STR_MAP_FROM_JSON(j, "privateAttributes", u.private_attributes);
//}

string MakeCacheKey(
    const string &sdk_key,
    const StatsigUser &user
) {
  std::vector<std::pair<string, string>> pairs(user.custom_ids.begin(), user.custom_ids.end());

  string custom_ids;

  if (!pairs.empty()) {
    sort(
        pairs.begin(), pairs.end(),
        [](const auto &left, const auto &right) {
          return left.first < right.first;
        });

    auto const [k1, v1] = pairs[0];
    custom_ids = accumulate(
        next(pairs.begin()), pairs.end(), k1 + "-" + v1,
        [](const string &acc, const std::pair<string, string> &p) {
          return acc + "," + p.first + "-" + p.second;
        }
    );
  }

  std::vector<string> parts{
      "uid:" + user.user_id,
      "cids:" + custom_ids,
      "k:" + sdk_key
  };

  auto result = accumulate(
      next(parts.begin()), parts.end(), parts[0],
      [](const string &acc, const string &s) {
        return acc + "|" + s;
      }
  );

  return hashing::DJB2(result);
}

bool AreUsersEqual(const StatsigUser &left, const StatsigUser &right) {
  if (left.user_id != right.user_id) {
    return false;
  }

  return true;
}

}