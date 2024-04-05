#pragma once

#include <optional>
#include <nlohmann/json.hpp>

namespace statsig {

typedef nlohmann::json JsonValue;

class ValueMap {
 public:
  ValueMap() = default;
  explicit ValueMap(JsonValue data) : data_(data) {}

  std::optional<std::string> GetStringValue(const std::string &key);

  JsonValue GetValue(const std::string &key);

 private:
  JsonValue data_;
};

}
