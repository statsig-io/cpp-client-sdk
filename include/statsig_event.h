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

  StatsigEvent(string event_name) {
    this->event_name = std::move(event_name);
  }

  StatsigEvent(
      string event_name,
      optional<double> value,
      OptStringMap metadata = nullopt
  ) {
    this->event_name = std::move(event_name);
    this->double_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      string event_name,
      optional<string> value,
      OptStringMap metadata = nullopt
  ) {
    this->event_name = std::move(event_name);
    this->string_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      string event_name,
      unordered_map<string, string> metadata
  ) {
    this->event_name = std::move(event_name);
    this->metadata = metadata;
  }
};

}