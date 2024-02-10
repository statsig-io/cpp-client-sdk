#pragma once

#include <future>

#include "statsig.h"

namespace statsig {

class StatsigContext;

class StatsigClient {
 public:
  static StatsigClient &Shared();
  StatsigClient();
  ~StatsigClient();

  std::future<void> Initialize(
      const std::string &sdk_key,
      const std::optional<StatsigUser> &user = std::nullopt,
      const std::optional<StatsigOptions> &options = std::nullopt
  );

  std::future<void> UpdateUser(const StatsigUser &user);

  void Shutdown();

  void LogEvent(const StatsigEvent &event);

  bool CheckGate(const std::string &gate_name);

  FeatureGate GetFeatureGate(const std::string &gate_name);

  DynamicConfig GetDynamicConfig(const std::string &config_name);

  Experiment GetExperiment(const std::string &experiment_name);

  Layer GetLayer(const std::string &layer_name);

 private:
  StatsigContext *context_;

  bool EnsureInitialized(const char *caller);
  void SwitchUser(const StatsigUser &user);
};

}
