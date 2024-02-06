#pragma once

#include "statsig_user.h"
#include "statsig_options.h"

#include "../src/statsig_context.h"

namespace statsig {

class StatsigClient {
 public:
  static StatsigClient &Shared();

  void Initialize(
      const std::string &sdk_key,
      StatsigUser *user = nullptr,
      StatsigOptions *options = nullptr
  );

  void Shutdown();

  void UpdateUser(StatsigUser *user);

  bool CheckGate(const std::string &gate_name);

//        void get_config();
//
//        void get_experiment();
//
//        void get_layer();

 private:
  std::optional<StatsigContext> context_;

  void fetch_and_save_values();
};

}
