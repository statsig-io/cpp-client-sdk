#pragma once

#include "statsig/evaluations_data_adapter.h"

#include "nlohmann/json.hpp"

namespace statsig::data_types::data_adapter_result {

inline StatsigResult<std::string> Serialize(const DataAdapterResult &res) {
  auto j = nlohmann::json{
      {"source", res.source},
      {"data", res.data},
      {"receivedAt", res.received_at},
      {"fullUserHash", res.full_user_hash}
  };

  return {Ok, j.dump()};
}

inline StatsigResult<DataAdapterResult> Deserialize(const std::string &input) {
  try {
    auto j = nlohmann::json::parse(input);
    DataAdapterResult res{
        j.at("fullUserHash").get<std::string>(),
        j.at("receivedAt").get<time_t>()
    };
    j.at("source").get_to(res.source);
    j.at("data").get_to(res.data);
    return {Ok, res};
  }
  catch (std::exception &) {
    return {JsonFailureDataAdapterResult};
  }
}

}