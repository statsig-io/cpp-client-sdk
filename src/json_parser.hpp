#pragma once

#include <string>
#include <unordered_map>
#include <any>

#ifdef STATSIG_WITH_NLOHMANN_JSON
#include "nlohmann_json_types/data_adapter_result_json.hpp"
#include "nlohmann_json_types/statsig_user_json.hpp"
#include "nlohmann_json_types/initialize_request_args_json.hpp"
#include "nlohmann_json_types/initialize_response_json.hpp"
#include "nlohmann_json_types/statsig_event_json.hpp"
#include "nlohmann_json_types/error_boundary_request_args_json.hpp"
#else
#include "statsig_compatibility/json/data_adapter_result_json.hpp"
#include "statsig_compatibility/json/statsig_user_json.hpp"
#include "statsig_compatibility/json/initialize_request_args_json.hpp"
#include "statsig_compatibility/json/initialize_response_json.hpp"
#include "statsig_compatibility/json/statsig_event_json.hpp"
#include "statsig_compatibility/json/error_boundary_request_args_json.hpp"
#endif

namespace statsig::internal {

class Json {
 public:
  template<class T>
  static std::optional<T> Deserialize(const std::string &input) {
    static_assert(
        std::disjunction<
            std::is_same<T, StatsigUser>,
            std::is_same<T, DataAdapterResult>,
            std::is_same<T, data::InitializeResponse>
        >::value,
        "type T is invalid"
    );

    if constexpr (std::is_same_v<T, DataAdapterResult>) {
      return data_types::data_adapter_result::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, StatsigUser>) {
      return data_types::statsig_user::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, data::InitializeResponse>) {
      return data_types::initialize_response::Deserialize(input);
    }

    return std::nullopt;
  }

  template<class T>
  static std::string Serialize(const T &input) {
    static_assert(
        std::disjunction<
            std::is_same<T, InitializeRequestArgs>,
            std::is_same<T, StatsigUser>,
            std::is_same<T, DataAdapterResult>,
            std::is_same<T, LogEventRequestArgs>,
            std::is_same<T, ErrorBoundaryRequestArgs>
        >::value,
        "type T is invalid"
    );

    if constexpr (std::is_same_v<T, InitializeRequestArgs>) {
      return data_types::initialize_request_args::Serialize(input);
    }

    if constexpr (std::is_same_v<T, LogEventRequestArgs>) {
      return data_types::log_event_request_args::Serialize(input);
    }

    if constexpr (std::is_same_v<T, StatsigUser>) {
      return data_types::statsig_user::Serialize(input);
    }

    if constexpr (std::is_same_v<T, DataAdapterResult>) {
      return data_types::data_adapter_result::Serialize(input);
    }

    return "";
  }
};

}
