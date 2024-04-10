#pragma once

#include <optional>
#include <string>

#include "nlohmann/json.hpp"

namespace statsig {

typedef nlohmann::json JsonValue;
typedef nlohmann::ordered_json JsonObject;

inline JsonValue GetJsonValueFromJsonObject(
    const std::string key,
    const JsonObject& object) {

  return nlohmann::json();
}

inline JsonObject GetSafeJsonObject(const JsonObject& object) {
  return nlohmann::json();
}

inline JsonObject EmptyJsonObject() {
  return nlohmann::json::parse("{}");
}

}