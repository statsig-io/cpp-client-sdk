#pragma once

#include <optional>
#include <string>

#include "nlohmann/json.hpp"

namespace statsig {

typedef nlohmann::json JsonValue;
typedef nlohmann::ordered_json JsonObject;

inline JsonValue GetJsonValueFromJsonObject(
    const std::string &key,
    const JsonObject &object) {
  if (object.is_object()) {
    return object[key];
  }

  return {};
}

inline JsonObject EmptyJsonObject() {
  return nlohmann::json::parse("{}");
}

inline JsonObject GetSafeJsonObject(const JsonObject &object) {
  if (object.is_null()) {
    return EmptyJsonObject();
  }

  return object;
}


}