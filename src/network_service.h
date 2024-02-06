#pragma once

#include <json.hpp>
#include <utility>
#include "statsig_user.h"
#include "statsig_options.h"

namespace statsig {

class NetworkService {
 public:
  explicit NetworkService(std::string sdk_key,
                          StatsigOptions *options)
      : sdk_key_(std::move(sdk_key)),
        options_(options) {};

  nlohmann::json fetch_for_user(StatsigUser *user);

 private:
  std::string sdk_key_;
  StatsigOptions *options_;
};

}
