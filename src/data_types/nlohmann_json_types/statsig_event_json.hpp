#pragma once

#include <nlohmann/json.hpp>
#include "../../statsig_event_internal.hpp"

namespace statsig::data_types::log_event_request_args {

using LogEventRequestArgs = internal::LogEventRequestArgs;

std::string Serialize(const LogEventRequestArgs &res) {
  auto j = nlohmann::json{

  };

  return j.dump();
}

}

namespace statsig::data_types::statsig_event {

using StatsigEventInternal = internal::StatsigEventInternal;

std::string Serialize(const StatsigEventInternal &res) {
  auto j = nlohmann::json{

  };

  return j.dump();
}

StatsigEventInternal Deserialize(const std::string &input) {
  auto j = nlohmann::json::parse(input);
  StatsigEventInternal res;
  return res;
}

//void to_json(json &j, const StatsigEventInternal &event) {
//  j = json{
//      {"eventName", event.event_name},
//      {"time", event.time},
////      {"user", event.user},
//  };
//
//  if (event.metadata.has_value()) {
//    j["metadata"] = event.metadata.value();
//  }
//
//  if (event.secondary_exposures.has_value()) {
//    j["secondaryExposures"] = event.secondary_exposures.value();
//  }
//
//  if (event.string_value.has_value()) {
//    j["value"] = event.string_value.value();
//  } else if (event.double_value.has_value()) {
//    j["value"] = event.double_value.value();
//  }
//}
//
//void from_json(const json &j, StatsigEventInternal &event) {
//  j.at("eventName").get_to(event.event_name);
////  j.at("gateValue").get_to(event.gate_value);
////  j.at("ruleID").get_to(event.rule_id);
//}


}