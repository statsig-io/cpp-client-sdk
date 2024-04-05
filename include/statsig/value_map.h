#pragma once

#include <optional>
#include <string>

#ifdef STATSIG_WITH_NLOHMANN_JSON
#include <nlohmann/json.hpp>
typedef nlohmann::json JsonValue;
#else
#include <any>
typedef std::any JsonValue;
#endif

namespace statsig {

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
