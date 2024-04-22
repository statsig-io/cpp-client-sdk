#pragma once

#include "statsig/internal/statsig_event_internal.hpp"

namespace statsig::data_types::log_event_response {

using LogEventResponse = internal::LogEventResponse;

inline StatsigResult<LogEventResponse> Deserialize(const std::string &input) {
  return {Ok};
}

}
