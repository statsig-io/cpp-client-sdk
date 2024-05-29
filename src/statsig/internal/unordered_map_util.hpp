#pragma once

#include <optional>
#include <unordered_map>
#include <map>

namespace statsig::internal {

template<typename Key, typename Value>
bool MapContains(const std::unordered_map<Key, Value> &map, const Key &key) {
  return map.find(key) != map.end();
}

template<typename Key, typename Value>
std::optional<Value> MapGetOrNull(const std::unordered_map<Key, Value> &map, const Key &key) {
  return MapContains(map, key) ? std::optional(map.at(key)) : std::nullopt;
}

template<typename Key, typename Value>
std::optional<Value> MapGetOrNull(const std::optional<std::unordered_map<Key, Value>> &map, const Key &key) {
  return map.has_value() ? MapGetOrNull(map.value(), key) : std::nullopt;
}

inline std::string CreateSortedMapString(const std::optional<StringMap> &unordered) {
  if (!unordered.has_value()) {
    return "";
  }

  const auto &unwrapped = unordered.value();
  
  std::map<std::string, std::string> sorted;
  for (const auto &[fst, snd] : unwrapped) {
    sorted.emplace(FromCompat(fst), FromCompat(snd));
  }
  
  std::ostringstream oss;
  for (const auto &pair : sorted) {
    oss << pair.first << ":" << pair.second << "|";
  }

  return oss.str();
}

}