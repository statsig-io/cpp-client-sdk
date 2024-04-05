#pragma once

#include <nlohmann/json.hpp>

#include "statsig/evaluations_data_adapter.h"

namespace statsig::data_types {

std::string Serialize( const DataAdapterResult &res) {
  auto j = nlohmann::json{
      {"source", res.source},
      {"data", res.data},
      {"receivedAt", res.receivedAt},
  };

  return j.dump();
}

DataAdapterResult Deserialize(const std::string &input) {
  auto j = nlohmann::json::parse(input);
  DataAdapterResult res;
  j.at("source").get_to(res.source);
  j.at("data").get_to(res.data);
  j.at("receivedAt").get_to(res.receivedAt);
  return res;
}

}