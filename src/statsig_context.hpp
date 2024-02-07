#pragma once

#include <utility>

#include "network_service.hpp"
#include "event_logger.hpp"
#include "evaluation_store.hpp"
#include "statsig_user.h"

using namespace std;

namespace statsig {

class StatsigContext {
 public:
  explicit StatsigContext(
      string sdk_key,
      const optional<StatsigUser> &user,
      const optional<StatsigOptions> &options
  ) : sdk_key(std::move(sdk_key)),
      user(user.value_or(StatsigUser())),
      options(options.value_or(StatsigOptions())),
      network(NetworkService(this->sdk_key, this->options)),
      store(EvaluationStore()),
      logger(EventLogger(this->options, this->network)) {
  }

  string sdk_key;
  StatsigUser user;
  StatsigOptions options;

  NetworkService network;
  EvaluationStore store;
  EventLogger logger;
};

}