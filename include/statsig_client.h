#pragma once

#include "statsig_user.h"
#include "statsig_options.h"
#include "statsig_types.h"

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

  FeatureGate GetFeatureGate(const string &gate_name);

  DynamicConfig GetDynamicConfig(const string &config_name);

  Experiment GetExperiment(const string &experiment_name);

  Layer GetLayer(const string &layer_name);

 private:
  optional<StatsigContext> context_;

  void SetValuesFromNetwork();
  bool EnsureInitialized(const char *caller);
};

}
