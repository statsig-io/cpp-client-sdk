#pragma once

#include "statsig_user.h"
#include "statsig_options.h"

#include "../src/statsig_context.hpp"

namespace statsig {

class StatsigClient {
 public:
  static StatsigClient &Shared();

  void Initialize(
      string sdk_key,
      const optional<StatsigUser> &user = std::nullopt,
      const optional<StatsigOptions> &options = std::nullopt
  );

  void Shutdown();

  void UpdateUser(StatsigUser *user);

  bool CheckGate(const string &gate_name);

//        void get_config();
//
//        void get_experiment();
//
//        void get_layer();

 private:
  optional<StatsigContext> context_;

  void set_values_from_network();
};

}
