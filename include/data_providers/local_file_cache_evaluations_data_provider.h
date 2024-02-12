#pragma once

#include "evaluations_data_provider.h"

namespace statsig::evaluations_data_providers {

class LocalFileCacheEvaluationsDataProvider : public EvaluationsDataProvider {
 public:
  std::optional<std::string> GetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user
  ) override;

  void SetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user,
      const std::string &data
  ) override;

  ValueSource GetSource() override;
};

}
