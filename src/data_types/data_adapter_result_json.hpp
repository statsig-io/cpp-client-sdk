#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <iostream>

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
  auto j = json::parse(input);
  DataAdapterResult res;
  j.at("source").get_to(res.source);
  j.at("data").get_to(res.data);
  j.at("receivedAt").get_to(res.receivedAt);
  return res;
}

//std::optional<DataAdapterResult> FromJson(const std::unordered_map<std::string, std::any> &json) {
//  try {
//    DataAdapterResult result;
//
//    result.source = std::any_cast<ValueSource>(json.at("source"));
//    result.data = std::any_cast<std::string>(json.at("data"));
//    result.receivedAt = std::any_cast<long long>(json.at("receivedAt"));
//
//    return result;
//  } catch (const std::exception &error) {
//    std::cerr << "[Statsig]: An unexpected exception occurred. " << error.what() << std::endl;
//    return std::nullopt;
//  }
//}

}