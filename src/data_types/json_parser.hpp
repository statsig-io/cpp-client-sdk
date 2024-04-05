#pragma once

#include <string>
#include <unordered_map>
#include <any>

#include "data_adapter_result_json.hpp"
#include "statsig_user_json.hpp"
#include "initialize_request_args_json.hpp"

namespace statsig::internal {

class Json {
 public:
  template<class T>
  static std::optional<T> Deserialize(const std::string &input) {
    if constexpr (std::is_same_v<T, DataAdapterResult>) {
      return data_types::Deserialize(input);
    }

    if constexpr (std::is_same_v<T, StatsigUser>) {
      return data_types::statsig_user::Deserialize(input);
    }

    return std::nullopt;
  }

  template<class T>
  static std::string Serialize(const T &input) {
    if constexpr (std::is_same_v<T, InitializeRequestArgs>) {
      return data_types::initialize_request_args::Serialize(input);
    }

    if constexpr (std::is_same_v<T, StatsigUser>) {
      return data_types::statsig_user::Serialize(input);
    }

    if constexpr (std::is_same_v<T, DataAdapterResult>) {
      return data_types::Serialize(input);
    }

    return "";
  }
};

}