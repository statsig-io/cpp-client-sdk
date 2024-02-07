#pragma once

#include <utility>

#include "network_service.hpp"
#include "statsig_logger.hpp"
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
  ) : sdk_key(sdk_key) {
    this->user = user.value_or(StatsigUser());
    this->options = options.value_or(StatsigOptions());

    network = new NetworkService(this->sdk_key, &this->options);
    store = new EvaluationStore();
  }

  string sdk_key;
  StatsigUser user;
  StatsigOptions options;

  NetworkService *network;
  EvaluationStore *store;
};

}