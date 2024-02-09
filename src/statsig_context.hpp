#pragma once

#include <utility>

#include "network_service.hpp"
#include "event_logger.hpp"
#include "evaluation_store.hpp"
#include "statsig_user.h"
#include "error_boundary.hpp"

namespace statsig {

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
      network(NetworkService(this->sdk_key, this->options, this->err_boundary)),
      store(EvaluationStore()),
      logger(EventLogger(this->options, this->network)) {}

  string sdk_key;
  StatsigUser user;
  StatsigOptions options;

  ErrorBoundary err_boundary;
  NetworkService network;
  EvaluationStore store;
  EventLogger logger;
};

}