#pragma once

#include <utility>

#include "network_service.hpp"
#include "event_logger.hpp"
#include "evaluation_store.hpp"
#include "statsig_user.h"
#include "error_boundary.hpp"
#include "data_providers/local_file_cache_evaluations_data_provider.cpp"
#include "data_providers/network_evaluations_data_provider.cpp"

namespace statsig {

using namespace statsig::evaluations_data_providers;

std::vector<EvaluationsDataProvider *> GetDefaultDataProviders() {
  return {
      new LocalFileCacheEvaluationsDataProvider(),
      new NetworkEvaluationsDataProvider()
  };
}

class StatsigContext {
 public:
  explicit StatsigContext(
      string sdk_key,
      const std::optional<StatsigUser> &user,
      const std::optional<StatsigOptions> &options
  ) : sdk_key(std::move(sdk_key)),
      user(user.value_or(StatsigUser())),
      options(options.value_or(StatsigOptions())),
      err_boundary(this->sdk_key),
      network(NetworkService(this->sdk_key)),
      store(EvaluationStore()),
      data_providers(
          this->options.providers
              .value_or(GetDefaultDataProviders())
      ),
      logger(EventLogger(this->options, this->network)) {}

  string sdk_key;
  StatsigUser user;
  StatsigOptions options;

  ErrorBoundary err_boundary;
  NetworkService network;
  EvaluationStore store;
  EventLogger logger;
  std::vector<EvaluationsDataProvider *> data_providers;
};

}