#pragma once

#include <nlohmann/json.hpp>

#include "../../error_boundary.hpp"

namespace statsig::data_types::error_boundary_request_args {

std::string Serialize(const internal::ErrorBoundaryRequestArgs &args) {
  auto j = nlohmann::json{
      {"tag", args.tag},
      {"exception", args.exception},
      {"info", args.info},
  };

  return j.dump();
}

}