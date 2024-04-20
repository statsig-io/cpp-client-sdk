#pragma once

#include "statsig/internal/statsig_event_internal.hpp"
#include "nlohmann/json.hpp"

namespace statsig::data_types::statsig_event {

using StatsigEventInternal = internal::StatsigEventInternal;

std::string Serialize(const StatsigEventInternal& res) {
  auto j = nlohmann::json{

  };

  return j.dump();
}

StatsigEventInternal Deserialize(const std::string& input) {
  auto j = nlohmann::json::parse(input);
  StatsigEventInternal res;
  return res;
}

} 