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

}