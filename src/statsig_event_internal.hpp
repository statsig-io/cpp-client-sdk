#pragma once

#include <string>
#include "statsig_user_internal.hpp"

namespace statsig {

struct StatsigEventInternal {
  string event_name;
  long time;
  StatsigUser user;
  optional<string> string_value;
  optional<double> double_value;
  optional<unordered_map<string, string>> metadata;
  optional<unordered_map<string, string>> secondary_exposures;
};

static StatsigEventInternal InternalizeEvent(StatsigEvent event, StatsigUser user) {
  StatsigEventInternal result;
  result.event_name = event.event_name;
  result.time = event.time.has_value() ? event.time.value() : Time::now();
  result.user = user;
  result.metadata = event.metadata;
  result.double_value = event.double_value;
  result.string_value = event.string_value;

  return result;
}

void to_json(json &j, const StatsigEventInternal &event) {
  j = json{
      {"eventName", event.event_name},
      {"time", event.time},
      {"user", event.user},
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
}

void from_json(const json &j, StatsigEventInternal &event) {
//  j.at("gate").get_to(event.gate);
//  j.at("gateValue").get_to(event.gate_value);
//  j.at("ruleID").get_to(event.rule_id);
}

}