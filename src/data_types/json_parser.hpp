#pragma once

#include <string>
#include <unordered_map>
#include <any>

#include "data_adapter_result_json.hpp"

namespace statsig::internal {

class Json {
 public:
  static std::optional<DataAdapterResult> Deserialize(const std::string &input) {
    return data_types::Deserialize(input);
  }

  static std::string Stringify(const DataAdapterResult &input) {
    return data_types::Serialize(input);
  }
};

}