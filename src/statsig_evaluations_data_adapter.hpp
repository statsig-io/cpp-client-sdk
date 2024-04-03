#pragma once

#include "statsig/evaluations_data_adapter.h"
#include "network_service.hpp"
#include "error_boundary.hpp"
#include "statsig_user_internal.hpp"

namespace statsig::internal {

std::optional<DataAdapterResult> ReadFromCacheFile(const std::string &cache_key) {
  auto data = File::ReadFromCache(cache_key);
  if (!data.has_value()) {
    return std::nullopt;
  }

  auto result = json::parse(data.value())
      .template get<DataAdapterResult>();

  result.source = ValueSource::Cache;
  return result;
}

void WriteToCacheFile(const std::string &cache_key, const DataAdapterResult &result) {
  File::WriteToCache(cache_key, json(result).dump());
  File::RunCacheEviction(kCachedEvaluationsPrefix);
}

class StatsigEvaluationsDataAdapter : public statsig::EvaluationsDataAdapter {
 public:
  explicit StatsigEvaluationsDataAdapter() : error_boundary_(ErrorBoundary()) {}

  void Attach(
      std::string &sdk_key,
      StatsigOptions &options
  ) override {
    sdk_key_ = sdk_key;
    network_ = new NetworkService(sdk_key, options);
    error_boundary_.SetSdkKey(sdk_key);
  }

  std::optional<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) override {
    std::optional<DataAdapterResult> result;

    error_boundary_.Capture(__func__, [this, &user, &result]() {
      const auto cache_key = GetCacheKey(user);
      auto in_mem = MapGetOrNull(in_memory_cache_, cache_key);

      if (in_mem.has_value()) {
        result = in_mem;
        return;
      }

      auto cache = ReadFromCacheFile(cache_key);
      if (cache.has_value()) {
        result = cache;
        AddToInMemoryCache(cache_key, cache.value());
        return;
      }
    });

    return result;
  }

  void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(std::optional<DataAdapterResult>)> &callback
  ) override {
    error_boundary_.Capture(__func__, [this, &user, &current, &callback]() {
      const auto cache = current ? current : GetDataSync(user);
      const auto latest = FetchLatest(user, cache);
      const auto cache_key = GetCacheKey(user);

      if (!latest.has_value()) {
        callback(std::nullopt);
        return;
      }

      AddToInMemoryCache(cache_key, latest.value());

      if (latest->source == ValueSource::Network) {
        WriteToCacheFile(cache_key, latest.value());
      }

      callback(latest);
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
  std::unordered_map<std::string, DataAdapterResult> in_memory_cache_ = {};
  NetworkService *network_;
  ErrorBoundary error_boundary_;

  std::string GetCacheKey(const StatsigUser &user) {
    const auto key = MakeCacheKey(GetSdkKey(), user);
    return kCachedEvaluationsPrefix + key;
  }

  std::string GetSdkKey() {
    if (!sdk_key_->empty()) {
      return sdk_key_.value();
    }

    std::cerr << "[Statsig]: StatsigEvaluationsDataAdapter is not attached to a Client. " << std::endl;
    return "";
  }

  void AddToInMemoryCache(const std::string &cache_key, const DataAdapterResult &result) {
    if (in_memory_cache_.size() < kMaxCacheEntriesCount) {
      in_memory_cache_[cache_key] = result;
      return;
    }

    auto oldest = in_memory_cache_.begin();
    long long smallestReceivedAt = std::numeric_limits<long long>::max();
    for (auto it = in_memory_cache_.begin(); it != in_memory_cache_.end(); ++it) {
      if (it->second.receivedAt < smallestReceivedAt) {
        oldest = it;
        smallestReceivedAt = it->second.receivedAt;
      }
    }

    if (oldest != in_memory_cache_.end()) {
      in_memory_cache_.erase(oldest);
    }

    in_memory_cache_[cache_key] = result;
  }

  std::optional<DataAdapterResult> FetchLatest(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> current
  ) {
    const auto response = network_->FetchValues(user);
    if (!response.has_value()) {
      return std::nullopt;
    }

    DataAdapterResult result;
    result.data = response->raw;
    result.receivedAt = Time::now();
    result.source = ValueSource::Network;
    return result;
  }
};

}