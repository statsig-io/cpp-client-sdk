#pragma once

#include "statsig/evaluations_data_provider.h"
#include "statsig/data_providers/local_file_cache_evaluations_data_provider.h"

namespace statsig::evaluations_data_providers {

std::optional<std::string> LocalFileCacheEvaluationsDataProvider::GetEvaluationsData(
    const std::string &sdk_key,
    const StatsigUser &user
) {
  auto cache_key = MakeCacheKey(sdk_key, user);
  return File::ReadFromCache(cache_key);
}

void LocalFileCacheEvaluationsDataProvider::SetEvaluationsData(
    const std::string &sdk_key,
    const StatsigUser &user,
    const std::string &data
) {
  auto cache_key = MakeCacheKey(sdk_key, user);
  File::WriteToCache(cache_key, data);
  File::RunCacheEviction();
}

ValueSource LocalFileCacheEvaluationsDataProvider::GetSource() {
  return ValueSource::Cache;
}

}
