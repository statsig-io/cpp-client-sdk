#pragma once

#include "evaluations_data_provider.h"

namespace statsig::evaluations_data_providers {

class LocalFileCacheEvaluationsDataProvider : public EvaluationsDataProvider {
 public:
  std::optional<std::string> GetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user
  ) override {
    auto cache_key = MakeCacheKey(sdk_key, user);
    return File::ReadFromCache(cache_key);
  }

  void SetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user,
      const std::string &data
  ) override {
    auto cache_key = MakeCacheKey(sdk_key, user);
    File::WriteToCache(cache_key, data);
    File::RunCacheEviction();
  }

  ValueSource GetSource() override {
    return ValueSource::Cache;
  }
};

}
