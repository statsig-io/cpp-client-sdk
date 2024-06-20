#pragma once

#include "../evaluations_data_adapter.h"
#include "network_service.hpp"
#include "json_parser.hpp"

#ifdef _WIN32
#define MAX_LONG_LONG LLONG_MAX
#else
#define MAX_LONG_LONG std::numeric_limits<long long>::max()
#endif

namespace statsig::internal {

namespace /* private */ {

StatsigResult<DataAdapterResult>
ReadFromCacheFile(
    const StatsigOptions &options,
    const std::string &cache_key
) {
  auto data = File::ReadFromCache(options, cache_key);
  if (!data.has_value()) {
    return {Ok, std::nullopt};
  }

  auto result = Json::Deserialize<DataAdapterResult>(data.value());
  if (result.value.has_value()) {
    result.value->source = ValueSource::Cache;
  }

  return result;
}

void WriteToCacheFile(
    const std::string &sdk_key,
    const StatsigOptions &options,
    const std::string &cache_key,
    const DataAdapterResult &result,
    const std::function<void(StatsigResultCode)> &callback
) {
  File::RunCacheEviction(options, constants::kCachedEvaluationsPrefix);

  auto serialized = Json::Serialize(result);
  if (serialized.code == Ok && serialized.value.has_value()) {
    File::WriteToCache(sdk_key, options, cache_key, serialized.value.value(), [callback](bool success) {
      callback(success ? Ok : FileFailureDataAdapterResult);
    });
  }
}

}

class StatsigEvaluationsDataAdapter : public EvaluationsDataAdapter, public std::enable_shared_from_this<StatsigEvaluationsDataAdapter> {
 public:
  static std::shared_ptr<StatsigEvaluationsDataAdapter> Create() {
    return std::shared_ptr<StatsigEvaluationsDataAdapter>(new StatsigEvaluationsDataAdapter());
  }

  void Attach(
      std::string &sdk_key,
      StatsigOptions &options
  ) override {
    sdk_key_ = sdk_key;
    options_ = options;
    network_ = NetworkService::Create(sdk_key, options);
  }

  StatsigResult<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) override {
    const auto cache_key = GetCacheKey(user);
    auto result = MapGetOrNull(in_memory_cache_, cache_key);

    if (result.has_value()) {
      return {Ok, result};
    }

    auto cache = ReadFromCacheFile(GetStatsigOptions(), cache_key);
    if (cache.code == Ok && cache.value.has_value()) {
      AddToInMemoryCache(cache_key, cache.value.value());
      return {Ok, cache.value};
    }

    return {cache.code, std::nullopt};
  }

  void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(StatsigResult<DataAdapterResult>)> &callback
  ) override {
    const auto cache = current.has_value() ? current : GetDataSync(user).value;
    const auto weak_self = weak_from_this();

    FetchLatest(user, cache, [weak_self, callback, user]
        (StatsigResult<DataAdapterResult> latest) {
      const auto shared_self = weak_self.lock();

      if (!shared_self) {
        callback({SharedPointerLost, std::nullopt});
        return;
      }

      const auto cache_key = shared_self->GetCacheKey(user);
      const auto sdk_key = shared_self->GetSdkKey();

      if (!latest.value.has_value()) {
        callback({latest.code, std::nullopt, latest.extra});
        return;
      }

      shared_self->AddToInMemoryCache(cache_key, latest.value.value());

      if (latest.value->source == ValueSource::Network) {
        WriteToCacheFile(
            sdk_key,
            shared_self->GetStatsigOptions(),
            cache_key,
            latest.value.value(),
            [latest, callback, sdk_key](StatsigResultCode result) {
              Diagnostics::Get(sdk_key)->Mark(markers::ProcessEnd(result == Ok));
              callback(latest);
            });
      } else {
        Diagnostics::Get(sdk_key)->Mark(markers::ProcessEnd(true));
        callback(latest);
      }
    });

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

 private:
  std::optional<std::string> sdk_key_;
  std::optional<StatsigOptions> options_;

  std::unordered_map<std::string, DataAdapterResult> in_memory_cache_ = {};
  std::shared_ptr<NetworkService> network_;

  explicit StatsigEvaluationsDataAdapter() {}

  std::string GetCacheKey(const StatsigUser &user) {
    const auto key = MakeCacheKey(GetSdkKey(), user);
    return constants::kCachedEvaluationsPrefix + key;
  }

  std::string GetSdkKey() {
    if (!sdk_key_->empty()) {
      return sdk_key_.value();
    }

    std::cerr << "[Statsig]: StatsigEvaluationsDataAdapter is not attached to a Client. " << std::endl;
    return "";
  }

  StatsigOptions GetStatsigOptions() {
    return options_.value_or(StatsigOptions());
  }

  void AddToInMemoryCache(
      const std::string &cache_key,
      const DataAdapterResult &result
  ) {
    if (in_memory_cache_.size() < constants::kMaxCachedEvaluationsCount) {
      in_memory_cache_.emplace(
          cache_key,
          result
      );
      return;
    }

    auto oldest = in_memory_cache_.begin();
    long long oldest_rec_at = MAX_LONG_LONG;
    for (auto it = in_memory_cache_.begin(); it != in_memory_cache_.end(); ++
        it) {
      if (it->second.received_at < oldest_rec_at) {
        oldest = it;
        oldest_rec_at = it->second.received_at;
      }
    }

    if (oldest != in_memory_cache_.end()) {
      in_memory_cache_.erase(oldest);
    }

    in_memory_cache_.emplace(cache_key, result);
  }

  void FetchLatest(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(StatsigResult<DataAdapterResult>)> &callback
  ) {
    network_->FetchValues(
        user,
        current,
        [callback, current, user](
            NetworkResult<data::InitializeResponse> net_result
        ) {
          if (net_result.code != Ok || !net_result.value.has_value()) {
            callback({net_result.code, std::nullopt, net_result.extra});
            return;
          }

          DataAdapterResult result{GetFullUserHash(user), Time::now()};
          const data::InitializeResponse data = net_result.value.value();

          if (data.has_updates) {
            result.data = net_result.raw;
            result.source = ValueSource::Network;
          } else {
            result.data = current.has_value() ? current->data : "";
            result.source = ValueSource::NetworkNotModified;
          }

          callback({Ok, result});
        });

  }
};

}
