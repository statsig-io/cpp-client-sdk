#pragma once

#include "statsig_compat/primitives/string.hpp"
#include "./statsig.h"

namespace statsig {

class StatsigContext;

class StatsigClient {
 public:
  static StatsigClient &Shared();
  StatsigClient();
  ~StatsigClient();

  StatsigResultCode InitializeSync(
      const String &sdk_key,
      const std::optional<StatsigUser> &user = std::nullopt,
      const std::optional<StatsigOptions> &options = std::nullopt);

  void InitializeAsync(
      const String &sdk_key,
      const std::function<void(StatsigResultCode)> &callback,
      const std::optional<StatsigUser> &user = std::nullopt,
      const std::optional<StatsigOptions> &options = std::nullopt);

  StatsigResultCode UpdateUserSync(const StatsigUser &user);

  void UpdateUserAsync(
      const StatsigUser &user,
      const std::function<void(StatsigResultCode)> &callback
  );

  void Shutdown();

  void Flush();

  void LogEvent(const StatsigEvent &event);

  bool CheckGate(const String &gate_name);

  FeatureGate GetFeatureGate(const String &gate_name);

  DynamicConfig GetDynamicConfig(const String &config_name);

  Experiment GetExperiment(const String &experiment_name);

  Layer GetLayer(const String &layer_name);

 private:
  std::shared_ptr<StatsigContext> context_;

  bool EnsureInitialized(const char *caller);
};

}
