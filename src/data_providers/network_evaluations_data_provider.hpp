#pragma once

#include "evaluations_data_provider.h"

namespace statsig::evaluations_data_providers {

class NetworkEvaluationsDataProvider : public EvaluationsDataProvider {
 public:
  explicit NetworkEvaluationsDataProvider(NetworkService &network) : network_(network) {}

  std::optional<std::string> GetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user
  ) override {
    auto result = network_.FetchValues(user);
    return result.has_value() ? std::optional(result->raw) : std::nullopt;
  }

  ValueSource GetSource() override {
    return ValueSource::Network;
  }

  bool IsTerminal() override {
    return true;
  }

 private:
  NetworkService &network_;
};

}
