#pragma once

#include "statsig.h"

namespace statsig {

class StatsigContext;

class StatsigClient {
 public:
  static StatsigClient &Shared();

  void Initialize(
      const std::string &sdk_key,
      const std::optional<StatsigUser> &user = std::nullopt,
      const std::optional<StatsigOptions> &options = std::nullopt
  );

  void Shutdown();

  void UpdateUser(const StatsigUser &user);

  void LogEvent(const StatsigEvent &event);

  bool CheckGate(const std::string &gate_name);

  FeatureGate GetFeatureGate(const std::string &gate_name);

  DynamicConfig GetDynamicConfig(const std::string &config_name);

  Experiment GetExperiment(const std::string &experiment_name);

  Layer GetLayer(const std::string &layer_name);

 private:
  std::unique_ptr<StatsigContext> context_;

  bool EnsureInitialized(const char *caller);
  void SwitchUser(const StatsigUser &user);
};

}
