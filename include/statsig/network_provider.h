#pragma once

#include <string>

#include "statsig_user.h"
#include "statsig_options.h"

namespace statsig {

class NetworkProvider {
 public:
  virtual void Attach(
      std::string &sdk_key,
      StatsigOptions &options
  ) = 0;

};

}

