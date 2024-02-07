#pragma once

#include <string>
#include <unordered_map>
#include <type_traits>
#include <utility>

using namespace std;

namespace statsig {

template<typename T>
struct StatsigEvent {
 public:
  std::string event_name;
  T value;
  std::unordered_map<std::string, std::string> metadata;
};

}