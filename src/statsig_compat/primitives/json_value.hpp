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

inline std::unordered_map<std::string, JsonValue> StringMapToJsonValueMap(
    const std::unordered_map<std::string, std::string> &map
) {
  std::unordered_map<std::string, JsonValue> result;
  for (const auto &pair : map) {
    result[pair.first] = pair.second;
  }
  return result;
}

inline void AddToJsonObject(
    JsonObject &obj,
    const std::string &key,
    const JsonValue &value
) {
  obj[key] = value;
}

inline JsonValue JsonValueFromNumber(const long &input) {
  return input;
}

inline JsonValue TimeToJsonValue(const time_t &input) {
  return input;
}

inline JsonValue IntToJsonValue(const int &input) {
  return input;
}

inline JsonValue BoolToJsonValue(const bool input) {
  return input;
}

inline JsonValue CompatStringToJsonValue(const String &input) {
  return input;
}

inline JsonValue StringToJsonValue(const std::string &input) {
  return input;
}

inline JsonValue JsonObjectToJsonValue(const JsonObject &input) {
  return input;
}

inline JsonValue JsonValueMapToJsonValue(
    const std::unordered_map<std::string, JsonValue> &input) {
  return input;
}

inline JsonValue JsonArrayToJsonValue(const std::vector<JsonValue> &input) {
  return input;
}

}