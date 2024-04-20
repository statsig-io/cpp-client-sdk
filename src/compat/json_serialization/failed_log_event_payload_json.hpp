#pragma once

#include "statsig/internal/statsig_event_internal.hpp"
#include "statsig_event_json.hpp"

namespace statsig::data_types::failed_log_event_payload {

inline std::string Serialize(
    const std::vector<internal::RetryableEventPayload> &failures) {
  return "";
}

inline StatsigResult<std::vector<internal::RetryableEventPayload>> Deserialize(
    const std::string &input) {
  return {Ok};
}

}
