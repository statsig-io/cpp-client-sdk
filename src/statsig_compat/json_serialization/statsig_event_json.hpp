#pragma once

#include "statsig/internal/statsig_event_internal.hpp"
#include "nlohmann/json.hpp"

namespace statsig::data_types::statsig_event {

using StatsigEventInternal = internal::StatsigEventInternal;

nlohmann::json ToJson(const StatsigEventInternal &event) {
  auto j = nlohmann::json{
      {"eventName", event.event_name},
      {"time", event.time},
      {"user", statsig_user::ToJson(event.user)},
  };

  if (event.metadata.has_value()) {
    j["metadata"] = event.metadata.value();
  }

  if (event.secondary_exposures.has_value()) {
    j["secondaryExposures"] = event.secondary_exposures.value();
  }

  if (event.string_value.has_value()) {
    j["value"] = event.string_value.value();
  } else if (event.double_value.has_value()) {
    j["value"] = event.double_value.value();
  }

  return j;
}

StatsigEventInternal FromJson(const nlohmann::json &json) {
  StatsigEventInternal result;

  json.at("eventName").get_to(result.event_name);
  json.at("time").get_to(result.time);

  const auto null_json = nlohmann::json();

  const auto &user_json = json.value("user", null_json);
  if (user_json.is_object()) {
    result.user = statsig_user::FromJson(user_json);
  }

  const auto &metadata = json.value("metadata", null_json);
  if (metadata.is_object()) {
    result.metadata = metadata.get<std::unordered_map<std::string, JsonValue>>();
  }

  const auto &secondary_exposures = json.value("secondary_exposures", null_json);
  if (secondary_exposures.is_array()) {
    result.secondary_exposures = secondary_exposures.get<std::vector<std::unordered_map<std::string, std::string>>>();
  }

  const auto &value = json.value("value", null_json);
  if (value.is_string()) {
    result.string_value = value.get<std::string>();
  } else if (value.is_number()) {
    result.double_value = value.get<double>();
  }

  return result;
}

}