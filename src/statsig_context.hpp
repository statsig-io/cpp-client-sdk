#pragma once

#include "network_service.hpp"
#include "statsig_logger.hpp"
#include "evaluation_store.hpp"
#include "statsig_user.h"

namespace statsig {

class StatsigContext {
 public:
  NetworkService *network;
  EvaluationStore *store;

  std::string sdk_key;
  StatsigOptions *options;
  StatsigUser *user;

  StatsigContext(
      std::string sdk_key,
      StatsigOptions *options,
      StatsigUser *user
  ) : sdk_key(std::move(sdk_key)), options(options), user(user) {

    auto n = NetworkService("sdk_key", options);
    network = &n;
    store = nullptr;
  }
};
}