#pragma once

#include <optional>
#include <string>

#include "nlohmann/json.hpp"

namespace statsig {

class JsonValue : public nlohmann::json {
 public:
  JsonValue() = default;
  JsonValue(nlohmann::json initial) : nlohmann::json(initial) {}

  static std::optional<JsonValue> GetValue(const std::string &key, const JsonValue &json) {
    if (!json.contains(key)) {
      return std::nullopt;
    }

    auto value = json.at(key);
    return JsonValue(value);
  }
};


}