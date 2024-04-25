#pragma once

#include <utility>
#include <unordered_map>

#include "statsig_compat/primitives/string.hpp"

namespace statsig {

struct StatsigEvent {
  String event_name;
  std::optional<time_t> time;
  std::optional<String> string_value;
  std::optional<double> double_value;
  std::optional<StringMap> metadata;

  explicit StatsigEvent(const String &event_name) {
    this->event_name = event_name;
  }

  StatsigEvent(
      const String &event_name,
      const std::optional<double> &value,
      const std::optional<StringMap> &metadata = std::nullopt) {
    this->event_name = event_name;
    this->double_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      const String &event_name,
      const std::optional<String> &value,
      const std::optional<StringMap> &metadata = std::nullopt) {
    this->event_name = event_name;
    this->string_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      const String &event_name,
      const StringMap &metadata) {
    this->event_name = event_name;
    this->metadata = metadata;
  }
};

}