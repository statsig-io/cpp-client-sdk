#pragma once

#include <string>
#include <unordered_map>

#include "../statsig.h"
#include "statsig_compat/json_serialization/data_adapter_result_json.hpp"
#include "statsig_compat/json_serialization/statsig_user_json.hpp"
#include "statsig_compat/json_serialization/initialize_request_args_json.hpp"
#include "statsig_compat/json_serialization/initialize_response_json.hpp"
#include "statsig_compat/json_serialization/statsig_event_json.hpp"
#include "statsig_compat/json_serialization/error_boundary_request_args_json.hpp"
#include "statsig_compat/json_serialization/log_event_response_json.hpp"
#include "statsig_compat/json_serialization/log_event_request_args_json.hpp"
#include "statsig_compat/json_serialization/retryable_event_payload_json.hpp"

namespace statsig::internal {

class Json {
 public:
  template<class T>
  static StatsigResult<T> Deserialize(const std::string &input) {
    static_assert(
        std::disjunction<
            std::is_same<T, StatsigUser>,
            std::is_same<T, DataAdapterResult>,
            std::is_same<T, data::InitializeResponse>,
            std::is_same<T, LogEventResponse>,
            std::is_same<T, std::vector<RetryableEventPayload>>>::value,
        "type T is invalid");

    if constexpr (std::is_same_v<T, DataAdapterResult>) {
      return data_types::data_adapter_result::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, StatsigUser>) {
      return data_types::statsig_user::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, data::InitializeResponse>) {
      return data_types::initialize_response::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, LogEventResponse>) {
      return data_types::log_event_response::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, std::vector<RetryableEventPayload>>) {
      return data_types::retryable_event_payload::Deserialize(input);
    }

    return {JsonFailureNoDeserializerFound};
  }

  template<class T>
  static StatsigResult<std::string> Serialize(const T &input) {
    static_assert(
        std::disjunction<
            // std::is_same<T, data::InitializeResponse>, Do Not Serialize InitRes, just use the raw network string
            std::is_same<T, InitializeRequestArgs>,
            std::is_same<T, StatsigUser>,
            std::is_same<T, DataAdapterResult>,
            std::is_same<T, LogEventRequestArgs>,
            std::is_same<T, ErrorBoundaryRequestArgs>,
            std::is_same<T, std::vector<RetryableEventPayload>>>::value,
        "type T is invalid");

    if constexpr (std::is_same_v<T, InitializeRequestArgs>) {
      return data_types::initialize_request_args::Serialize(input);
    }

    if constexpr (std::is_same_v<T, StatsigUser>) {
      return data_types::statsig_user::Serialize(input);
    }

    if constexpr (std::is_same_v<T, DataAdapterResult>) {
      return data_types::data_adapter_result::Serialize(input);
    }

    if constexpr (std::is_same_v<T, LogEventRequestArgs>) {
      return data_types::log_event_request_args::Serialize(input);
    }

    if constexpr (std::is_same_v<T, ErrorBoundaryRequestArgs>) {
      return data_types::error_boundary_request_args::Serialize(input);
    }

    if constexpr (std::is_same_v<T, std::vector<RetryableEventPayload>>) {
      return data_types::retryable_event_payload::Serialize(input);
    }

    return {JsonFailureNoSerializerFound};
  }
};

}
