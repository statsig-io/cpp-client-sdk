#pragma once

#include "statsig/internal/statsig_event_internal.hpp"
#include "statsig_event_json.hpp"

namespace statsig::data_types::retryable_event_payload {

inline StatsigResult<std::string> Serialize(
    const std::vector<internal::RetryableEventPayload> &failures) {
  return {Ok, ""};
}

inline StatsigResult<std::vector<internal::RetryableEventPayload>> Deserialize(
    const std::string &input) {
  return {Ok};
}

}
