#pragma once

#include "network_service.h"
#include "statsig_logger.h"
#include "evaluation_store.h"
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
    network = nullptr;
    store = nullptr;
  }
};
}