#pragma once

#include "../../statsig_event_internal.hpp"

namespace statsig::data_types::log_event_request_args {

using LogEventRequestArgs = internal::LogEventRequestArgs;

std::string Serialize(const LogEventRequestArgs &res) {
  return "";
}

}

namespace statsig::data_types::statsig_event {

using StatsigEventInternal = internal::StatsigEventInternal;

std::string Serialize(const StatsigEventInternal &res) {
  return "";
}

StatsigEventInternal Deserialize(const std::string &input) {
  StatsigEventInternal res;
  return res;
}

}