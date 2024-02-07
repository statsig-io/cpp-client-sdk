#include <utility>

#pragma once

using namespace std;

namespace statsig {

template<typename T>
struct StatsigEvent {
 public:
  static_assert(
      is_same<T, double>::value || is_same<T, string>::value || is_same<T, int>::value,
      "Invalid type for StatsigEvent. Valid types are [string, double, int]");

  string event_name;
  optional<T> value;
  optional<unordered_map<string, string>> metadata;

  explicit StatsigEvent(
      string event_name,
      const optional<T> &value = nullopt,
      const optional<unordered_map<string, string>> &metadata = nullopt
  )
      : event_name(std::move(event_name)), value(value), metadata(metadata) {}
};

StatsigEvent(string,
    optional<double>,
const optional<unordered_map<string, string>> & = nullopt) ->
StatsigEvent<double>;

StatsigEvent(string,
const optional<string> & = nullopt,
const optional<unordered_map<string, string>> & = nullopt) ->
StatsigEvent<string>;

}