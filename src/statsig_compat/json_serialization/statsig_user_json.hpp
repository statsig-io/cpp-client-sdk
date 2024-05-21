#pragma once

#include "nlohmann/json.hpp"

#include "statsig/evaluations_data_adapter.h"

namespace statsig::data_types::statsig_user {

// Opt + Json
#define OPT_TO_JSON(jsonObj, fieldName, fieldValue) do { if (fieldValue) { jsonObj[#fieldName] = fieldValue.value(); } } while(0)
#define OPT_STR_FROM_JSON(jsonObj, fieldName, target) do { if (jsonObj.contains(fieldName)) { target = jsonObj[fieldName].get<std::string>(); } } while(0)
#define OPT_STR_MAP_FROM_JSON(jsonObj, fieldName, target) do { if (jsonObj.contains(fieldName)) { target = jsonObj[fieldName].get<std::unordered_map<std::string, std::string>>(); } } while(0)

inline nlohmann::json EnvToJson(const StatsigEnvironment &e) {
  auto j = nlohmann::json{};
  OPT_TO_JSON(j, tier, e.tier);
  return j;
}

inline StatsigEnvironment EnvFromJson(const nlohmann::json &j) {
  StatsigEnvironment u;
  OPT_STR_FROM_JSON(j, "tier", u.tier);
  return u;
}

inline nlohmann::json ToJson(const StatsigUser &u) {
  auto j = nlohmann::json{};

  if (!u.custom_ids.empty()) {
    j["customIDs"] = u.custom_ids;
  }

  OPT_TO_JSON(j, userID, u.user_id);
  OPT_TO_JSON(j, email, u.email);
  OPT_TO_JSON(j, ip, u.ip);
  OPT_TO_JSON(j, userAgent, u.user_agent);
  OPT_TO_JSON(j, country, u.country);
  OPT_TO_JSON(j, locale, u.locale);
  OPT_TO_JSON(j, appVersion, u.app_version);
  OPT_TO_JSON(j, custom, u.custom);
  OPT_TO_JSON(j, privateAttributes, u.private_attributes);

  if (u.statsig_environment.has_value()) {
    j["statsigEnvironment"] = EnvToJson(u.statsig_environment.value());
  }

  return j;
}

inline StatsigUser FromJson(const nlohmann::json &j) {
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

  if (j.contains("statsigEnvironment")) {
    u.statsig_environment = EnvFromJson(j.at("statsigEnvironment"));
  }

  return u;
}

inline StatsigResult<std::string> Serialize(const StatsigUser &user) {
  return {Ok, ToJson(user).dump()};
}

inline StatsigResult<StatsigUser> Deserialize(const std::string &input) {
  try {
    auto j = nlohmann::json::parse(input);
    auto user = FromJson(j);
    return {Ok, user};
  } catch (std::exception &) {
    return {JsonFailureStatsigUser};
  }
}

}