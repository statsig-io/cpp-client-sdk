#pragma once

#include "nlohmann/json.hpp"

#include "statsig_event_json.hpp"

namespace statsig::data_types::log_event_request_args {

inline std::string Serialize(const internal::LogEventRequestArgs &args) {
  auto j = nlohmann::json{
//      {"events", args.events},
      {"statsigMetadata", args.statsig_metadata}
  };

  return j.dump();
}

}