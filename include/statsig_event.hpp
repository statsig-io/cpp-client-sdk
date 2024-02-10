#pragma once

#include <utility>
#include <unordered_map>

namespace statsig {

typedef std::unordered_map<std::string, std::string> StrMap;

struct StatsigEvent {
  std::string event_name;
  std::optional<long> time;
  std::optional<std::string> string_value;
  std::optional<double> double_value;
  std::optional<StrMap> metadata;

  explicit StatsigEvent(const std::string &event_name) {
    this->event_name = event_name;
  }

  StatsigEvent(
      const std::string &event_name,
      const std::optional<double> &value,
      const std::optional<StrMap> &metadata = std::nullopt
  ) {
    this->event_name = event_name;
    this->double_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      const std::string &event_name,
      const std::optional<std::string> &value,
      const std::optional<StrMap> &metadata = std::nullopt
  ) {
    this->event_name = event_name;
    this->string_value = value;
    this->metadata = metadata;
  }

  StatsigEvent(
      const std::string &event_name,
      const std::unordered_map<std::string, std::string> &metadata
  ) {
    this->event_name = event_name;
    this->metadata = metadata;
  }
};

}