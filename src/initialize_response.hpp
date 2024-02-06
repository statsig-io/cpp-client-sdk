#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace statsig {

struct InitializeResponse {
  std::string generator;
  long time;

 public:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(InitializeResponse, generator, time)
};

}