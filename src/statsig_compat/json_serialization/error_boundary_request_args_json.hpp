#pragma once

#include "nlohmann/json.hpp"

#include "statsig/internal/error_boundary_request_args.h"

namespace statsig::data_types::error_boundary_request_args {

StatsigResult<std::string> Serialize(const internal::ErrorBoundaryRequestArgs &args) {
  auto j = nlohmann::json{
      {"tag", args.tag},
      {"exception", args.exception},
      {"info", args.info},
  };

  if (args.extra.has_value()) {
    j["extra"] = args.extra.value();
  }

  return {Ok, j.dump()};
}

}