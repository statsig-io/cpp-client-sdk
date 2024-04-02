#pragma once

#include "statsig/evaluations_data_provider.h"

namespace statsig {
class NetworkService;
}

namespace statsig::evaluations_data_providers {

class NetworkEvaluationsDataProvider : public EvaluationsDataProvider {
 public:
  explicit NetworkEvaluationsDataProvider();

  std::optional<std::string> GetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user
  ) override;

  ValueSource GetSource() override;

  bool IsTerminal() override;

 private:
  NetworkService *network_;
};

}
