#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "statsig_user.h"
#include "statsig_options.h"
#include "statsig_result.h"

namespace statsig {

enum class ValueSource {
  Uninitialized,
  Loading,
  NoValues,
  Cache,
  Network,
  NetworkNotModified,
  Bootstrap
};

struct DataAdapterResult {
  ValueSource source;
  std::string data;
  time_t received_at{};
  std::string full_user_hash;

  explicit DataAdapterResult(
      std::string user_hash,
      time_t received
  )
      : source(ValueSource::Uninitialized),
        received_at(received),
        full_user_hash(std::move(user_hash)) {}

  DataAdapterResult(
      ValueSource src,
      std::string values_str,
      time_t received,
      std::string user_hash)
      : source(src),
        data(std::move(values_str)),
        received_at(received),
        full_user_hash(std::move(user_hash)) {}

};

class EvaluationsDataAdapter {
 public:
  virtual ~EvaluationsDataAdapter() = default;

  virtual void Attach(
      std::string &sdk_key,
      StatsigOptions &options
  ) = 0;

  virtual StatsigResult<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) = 0;

  virtual void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(StatsigResult<DataAdapterResult>)> &callback
  ) = 0;

  virtual void SetData(
      const StatsigUser &user,
      const std::string &data
  ) = 0;

  virtual void PrefetchData(
      const StatsigUser &user,
      const std::function<void()> &callback
  ) = 0;
};

}
