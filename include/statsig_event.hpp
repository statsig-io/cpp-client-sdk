#pragma once

#include <utility>

using namespace std;
using OptStringMap = optional<unordered_map<string, string>>;

namespace statsig {

struct StatsigEvent {
  string event_name;
  optional<long> time;
  optional<string> string_value;
  optional<double> double_value;
  OptStringMap metadata;

  StatsigEvent(const string &event_name) {
    this->event_name = event_name;
  }

  StatsigEvent(
      const string &event_name,
      const optional<double> &value,
      const OptStringMap &metadata = nullopt
  ) {
    this->event_name = event_name;
    this->double_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      const string &event_name,
      const optional<string> &value,
      const OptStringMap &metadata = nullopt
  ) {
    this->event_name = event_name;
    this->string_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      const string &event_name,
      const unordered_map<string, string> &metadata
  ) {
    this->event_name = event_name;
    this->metadata = metadata;
  }
};

}