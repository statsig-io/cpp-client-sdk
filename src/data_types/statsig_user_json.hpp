#pragma once

#include <nlohmann/json.hpp>

#include "statsig/evaluations_data_adapter.h"

namespace statsig::data_types::statsig_user {

// Opt + Json
#define OPT_TO_JSON(jsonObj, fieldName, fieldValue) do { if (fieldValue) { jsonObj[#fieldName] = fieldValue.value(); } } while(0)
#define OPT_STR_FROM_JSON(jsonObj, fieldName, target) do { if (jsonObj.contains(fieldName)) { target = jsonObj[fieldName].get<std::string>(); } } while(0)
#define OPT_STR_MAP_FROM_JSON(jsonObj, fieldName, target) do { if (jsonObj.contains(fieldName)) { target = jsonObj[fieldName].get<std::unordered_map<std::string, std::string>>(); } } while(0)

nlohmann::json ToJson(const StatsigUser &u) {
  auto j = nlohmann::json{
      {"userID", u.user_id},
  };

  if (!u.custom_ids.empty()) {
    j["customIDs"] = u.custom_ids;
  }

  OPT_TO_JSON(j, "email", u.email);
  OPT_TO_JSON(j, "ip", u.ip);
  OPT_TO_JSON(j, "userAgent", u.user_agent);
  OPT_TO_JSON(j, "country", u.country);
  OPT_TO_JSON(j, "locale", u.locale);
  OPT_TO_JSON(j, "appVersion", u.app_version);
  OPT_TO_JSON(j, "custom", u.custom);
  OPT_TO_JSON(j, "privateAttributes", u.private_attributes);

  return j;
}

std::string Serialize(const StatsigUser &user) {
  return ToJson(user).dump();
}

StatsigUser Deserialize(const std::string &input) {
  auto j = nlohmann::json::parse(input);

  StatsigUser u;
  u.user_id = j.value("userID", "");
  u.custom_ids = j.value("customIDs", std::unordered_map<std::string, std::string>());

  OPT_STR_FROM_JSON(j, "email", u.email);
  OPT_STR_FROM_JSON(j, "ip", u.ip);
  OPT_STR_FROM_JSON(j, "userAgent", u.user_agent);
  OPT_STR_FROM_JSON(j, "country", u.country);
  OPT_STR_FROM_JSON(j, "locale", u.locale);
  OPT_STR_FROM_JSON(j, "appVersion", u.app_version);
  OPT_STR_MAP_FROM_JSON(j, "custom", u.custom);
  OPT_STR_MAP_FROM_JSON(j, "privateAttributes", u.private_attributes);

  return u;
}

}