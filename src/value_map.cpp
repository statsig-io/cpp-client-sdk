#include "statsig/value_map.h"

#include <optional>

namespace statsig {

#ifdef STATSIG_WITH_NLOHMANN_JSON
std::optional<std::string> ValueMap::GetStringValue(const std::string &key) {
  auto str = GetValue(key);
  if (str.is_string()) {
    return str.get<std::string>();
  }

  return std::nullopt;
}

JsonValue ValueMap::GetValue(const std::string &key) {
  if (!data_.contains(key)) {
    return nullptr;
  }

  auto value = data_.at(key);
  if (value != nullptr) {
    return value;
  }

  return nullptr;
}
#else
std::optional<std::string> ValueMap::GetStringValue(const std::string &key) {
  throw "";
}

JsonValue ValueMap::GetValue(const std::string &key) {
  return nullptr;
}
#endif
}