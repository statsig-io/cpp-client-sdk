#pragma once

#include <string>

#include "statsig_user.h"
#include "statsig_options.h"

namespace statsig {

enum class ValueSource {
  Uninitialized,
  Loading,
  NoValues,
  Cache,
  Network,
  Bootstrap
};

struct DataAdapterResult {
  ValueSource source;
  std::string data;
  long long receivedAt;
};

class EvaluationsDataAdapter {
 public:
  virtual void Attach(
      std::string &sdk_key,
      StatsigOptions &options
  ) = 0;

  virtual std::optional<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) = 0;

  virtual void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(std::optional<DataAdapterResult>)> &callback
  ) = 0;

  virtual void SetData(
      const StatsigUser &user,
      const std::string &data
  ) = 0;

  virtual void PrefetchData(
      const StatsigUser &user,
      const std::function<void(void)> &callback
  ) = 0;
};

}

