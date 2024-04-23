#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace statsig {

typedef std::unordered_map<std::string, std::string> StringMap;

inline std::unordered_map<std::string, std::string> FromCompat(const StringMap &input) {
  return input;
}

inline StringMap ToCompat(const std::unordered_map<std::string, std::string> &input) {
  return input;
}

inline long GetMapSize(const StringMap &map) {
  return map.size();
}

inline std::vector<std::pair<String, String>> GetKeyValuePairs(const StringMap &map) {
  std::vector<std::pair<String, String>> pairs(map.begin(), map.end());
  return pairs;
}

}
