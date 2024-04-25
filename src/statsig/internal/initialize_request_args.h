#pragma once

#include <string>
#include "../statsig_user.h"

namespace statsig::internal {

struct InitializeRequestArgs {
  std::string hash;
  StatsigUser user;
  std::unordered_map<std::string, std::string> statsig_metadata;
  time_t since_time;
};

}
