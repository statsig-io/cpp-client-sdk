#pragma once

#include "nlohmann/json.hpp"

#include "statsig_event_json.hpp"

namespace statsig::data_types::log_event_request_args {

inline StatsigResult<std::string> Serialize(const internal::LogEventRequestArgs &args) {
  if (args.statsig_metadata.empty()) {
    return {JsonFailureLogEventRequestArgs};
  }

  std::vector<nlohmann::json> events_json_arr;
  events_json_arr.reserve(args.events.size());

  for (const auto &event : args.events) {
    events_json_arr.push_back(statsig_event::ToJson(event));
  }

  auto j = nlohmann::json{
      {"events", events_json_arr},
      {"statsigMetadata", args.statsig_metadata}
  };

  return {Ok, j.dump()};
}

}