#pragma once

#include "statsig.h"

using namespace statsig;

class MockEvaluationsDataAdapter : public EvaluationsDataAdapter {
 public:
  std::function<StatsigResult<DataAdapterResult>(
      const StatsigUser &user
  )> on_get_data_sync = [](auto) { return StatsigResult<DataAdapterResult>{Ok}; };

  std::function<void(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(StatsigResult<DataAdapterResult>)> &callback
  )> on_get_data_async = [](auto, auto, auto callback) { callback({Ok}); };

  void Attach(
      std::string &sdk_key,
      StatsigOptions &options
  ) override {
  }

  StatsigResult<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) override {
    return on_get_data_sync(user);
  }

  void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(StatsigResult<DataAdapterResult>)> &callback
  ) override {
    on_get_data_async(user, current, callback);
  }

  void SetData(
      const StatsigUser &user,
      const std::string &data
  ) override {
    // TODO: Support Bootstrap
  }

  void PrefetchData(
      const StatsigUser &user,
      const std::function<void(void)> &callback
  ) override {
    // TODO: Support Prefetch
  }
};