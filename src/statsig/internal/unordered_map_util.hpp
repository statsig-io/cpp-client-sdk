#pragma once

#include <unordered_map>

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

}